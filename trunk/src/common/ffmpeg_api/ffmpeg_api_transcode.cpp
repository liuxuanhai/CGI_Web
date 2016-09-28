#include "ffmpeg_api_transcode.h"
#include "ffmpeg_api_util.h"

namespace common
{
	namespace ffmpegapi
	{				
		/**
		 * Libswresample (lswr) is a library that handles audio resampling, sample format conversion and mixing
		 * Initialize the audio resampler based on the input and output codec settings.
		 * If the input and output sample formats differ, a conversion is required
		 * libswresample takes care of this, but requires initialization.
		 */
		int FFMPEGAPI_TRANSCODE::init_resampler(SwrContext **resample_context,
								AVCodecContext *input_codec_context,
		                       	AVCodecContext *output_codec_context)
		{
	        int error;
	        /**
	         * Create a resampler context for the conversion.
	         * Set the conversion parameters.
	         * Default channel layouts based on the number of channels
	         * are assumed for simplicity (they are sometimes not detected
	         * properly by the demuxer and/or decoder).
	         */
	        *resample_context = swr_alloc_set_opts(NULL,
	                                              av_get_default_channel_layout(output_codec_context->channels),
	                                              output_codec_context->sample_fmt,
	                                              output_codec_context->sample_rate,
	                                              av_get_default_channel_layout(input_codec_context->channels),
	                                              input_codec_context->sample_fmt,
	                                              input_codec_context->sample_rate,
	                                              0, NULL);
	        if (!*resample_context) 
	        {
				LOG4CPLUS_ERROR(logger, "Error Could not allocate resample context");
	            return AVERROR(ENOMEM);
	        }
	        /**
	        * Perform a sanity check so that the number of converted samples is
	        * not greater than the number of samples to be converted.
	        * If the sample rates differ, this case has to be handled differently
	        */
	        //av_assert0(output_codec_context->sample_rate == input_codec_context->sample_rate);
	        /** Open the resampler with the specified parameters. */
	        if ((error = swr_init(*resample_context)) < 0) 
	        {
				LOG4CPLUS_ERROR(logger, "Error Could not open resample context");
	            swr_free(resample_context);
	            return error;
	        }
		    return 0;
		}		

		/** Initialize a FIFO buffer for the audio samples to be encoded. */
		int FFMPEGAPI_TRANSCODE::init_fifo(AVAudioFifo **fifo, AVCodecContext *output_codec_context)
		{
		    /** Create the FIFO buffer based on the specified output sample format. */
		    if (!(*fifo = av_audio_fifo_alloc(output_codec_context->sample_fmt,
		                                      output_codec_context->channels, 1))) 
		   	{
		        LOG4CPLUS_ERROR(logger, "Error Could not allocate FIFO");
		        return AVERROR(ENOMEM);
		    }
		    return 0;
		}		

		/**
		 * Initialize one output frame for writing to the output file.
		 * The frame will be exactly frame_size samples large.
		 */
		int FFMPEGAPI_TRANSCODE::init_output_frame(AVFrame **frame,
		                             AVCodecContext *output_codec_context,
		                             int frame_size)
		{
		    int error;
		    /** Create a new frame to store the audio samples. */
		    if (!(*frame = av_frame_alloc())) {
				LOG4CPLUS_ERROR(logger, "allocate output frame");
		        return AVERROR_EXIT;
		    }
		    /**
		     * Set the frame's parameters, especially its size and format.
		     * av_frame_get_buffer needs this to allocate memory for the
		     * audio samples of the frame.
		     * Default channel layouts based on the number of channels
		     * are assumed for simplicity.
		     */
		    (*frame)->nb_samples     = frame_size;
		    (*frame)->channel_layout = output_codec_context->channel_layout;
		    (*frame)->format         = output_codec_context->sample_fmt;
		    (*frame)->sample_rate    = output_codec_context->sample_rate;
		    /**
		     * Allocate the samples of the created frame. This call will make
		     * sure that the audio frame can hold as many samples as specified.
		     */
		    if ((error = av_frame_get_buffer(*frame, 0)) < 0) {
				LOG4CPLUS_ERROR(logger, "allocate output frame samples failed, error: " << ErrToString(error));
		        av_frame_free(frame);
		        return error;
		    }
		    return 0;
		}

		/**
		 * Initialize a temporary storage for the specified number of audio samples.
		 * The conversion requires temporary storage due to the different format.
		 * The number of audio samples to be allocated is specified in frame_size.
		 */
		int FFMPEGAPI_TRANSCODE::init_converted_samples(uint8_t ***converted_samples,
		                                  AVCodecContext *output_codec_context,
		                                  int converted_nb_samples)
		{
		    int error;
		    /**
		     * Allocate as many pointers as there are audio channels.
		     * Each pointer will later point to the audio samples of the corresponding
		     * channels (although it may be NULL for interleaved formats).
		     */
		    if (!(*converted_samples = (uint8_t **)calloc(output_codec_context->channels,
		                                            sizeof(**converted_samples)))) 
		   	{
				LOG4CPLUS_ERROR(logger, "allocate converted sample pointers failed");
		        return AVERROR(ENOMEM);
		    }
		    /**
		     * Allocate memory for the samples of all channels in one consecutive
		     * block for convenience.
		     */
		    if ((error = av_samples_alloc(*converted_samples, NULL,
		                                  output_codec_context->channels,
		                                  converted_nb_samples,
		                                  output_codec_context->sample_fmt, 0)) < 0) 
		    {        
				LOG4CPLUS_ERROR(logger, "allocate converted samples failed, error: " << ErrToString(error));
		        av_freep(&(*converted_samples)[0]);
		        free(*converted_samples);
		        return error;
		    }
		    return 0;
		}

		/**
		 * Convert the input audio samples into the output sample format.
		 * The conversion happens on a per-frame basis, the size of which is specified
		 * by frame_size.
		 */
		int FFMPEGAPI_TRANSCODE::convert_samples(SwrContext *resample_context,
									uint8_t **converted_data, const int converted_size,
									const uint8_t **input_data, const int input_size)
		{
		    int error;
		    /** Convert the samples using the resampler. */
		    if ((error = swr_convert(resample_context,
		                             converted_data, converted_size,
		                             input_data    , input_size)) < 0) 
		    {        
				LOG4CPLUS_ERROR(logger, "convert input samples failed, error: " << ErrToString(error));
		        return error;
		    }
		    return 0;
		}

		/** Add converted input audio samples to the FIFO buffer for later processing. */
		int FFMPEGAPI_TRANSCODE::add_samples_to_fifo(AVAudioFifo *fifo,
		                               uint8_t **audio_data,
		                               const int data_size)
		{
		    int error;
		    /**
		     * Make the FIFO as large as it needs to be to hold both,
		     * the old and the new samples.
		     */
		    if ((error = av_audio_fifo_realloc(fifo, av_audio_fifo_size(fifo) + data_size)) < 0) 
		    {
				LOG4CPLUS_ERROR(logger, "reallocate FIFO failed");
		        return error;
		    }
		    
		    /** Store the new samples in the FIFO buffer. */
		    if (av_audio_fifo_write(fifo, (void **)audio_data, data_size) < data_size) 
		    {
				LOG4CPLUS_ERROR(logger, "write data to FIFO failed");
		        return AVERROR_EXIT;
		    }
		    //LOG4CPLUS_DEBUG(logger, "fifo total size: " << av_audio_fifo_size(fifo));
		    return 0;
		}		

		int FFMPEGAPI_TRANSCODE::convert_and_store(AVAudioFifo *fifo, 
						SwrContext *resampler_context, 
						AVFrame *input_frame, 
						AVCodecContext *output_codec_context)
		{
			int ret = -1;
			/** Temporary storage for the converted samples. */
		    uint8_t **converted_samples = NULL;
		    int converted_nb_samples = 0;
		    
		    converted_nb_samples = av_rescale_rnd(input_frame->nb_samples, output_codec_context->sample_rate, input_frame->sample_rate, AV_ROUND_UP);
		    #if 0
		    LOG4CPLUS_DEBUG(logger, "input_frame->nb_samples: " << input_frame->nb_samples);
		    LOG4CPLUS_DEBUG(logger, "input_frame->sample_rate: " << input_frame->sample_rate);
		    LOG4CPLUS_DEBUG(logger, "output_codec_context->sample_rate: " << output_codec_context->sample_rate);
		    LOG4CPLUS_DEBUG(logger, "converted_nb_samples: " << converted_nb_samples);		    
		    #endif

			/** If there is decoded data, convert and store it */
			if (input_frame) 
			{
		        /** Initialize the temporary storage for the converted samples. */
		        if (init_converted_samples(&converted_samples, output_codec_context,
		                                   converted_nb_samples))
		            goto cleanup;
		        /**
		         * Convert the input samples to the desired output sample format.
		         * This requires a temporary storage provided by converted_samples.
		         * int AVFrame::nb_samples：number of audio samples (per channel) described by this frame
		         * input_frame->extended_data sometime is same as input_frame->data 
		         */
		        if (convert_samples(resampler_context,
		        					converted_samples, converted_nb_samples,
		        					(const uint8_t**)input_frame->extended_data, input_frame->nb_samples))
		            goto cleanup;
		        /** Add the converted input samples to the FIFO buffer for later processing. */
		        if (add_samples_to_fifo(fifo, converted_samples,
		                                converted_nb_samples))
		            goto cleanup;
				ret = 0;
		    }
		    
		cleanup:
		    if (converted_samples) 
		    {
		        av_freep(&converted_samples[0]);
		        free(converted_samples);
		    }
		    
		    return ret;		
		}

		int FFMPEGAPI_TRANSCODE::load_encode_and_write(AVAudioFifo *fifo, AVCodecContext *output_codec_context, int stream_index)
		{
			/** Temporary storage of the output samples of the frame written to the file. */
		    AVFrame *output_frame;
		    /**
		     * Use the maximum number of possible samples per frame.
		     * If there is less than the maximum possible frame size in the FIFO
		     * buffer use this number. Otherwise, use the maximum possible frame size
		     */
		    const int frame_size = FFMIN(av_audio_fifo_size(fifo),
		                                 output_codec_context->frame_size);
		    if(frame_size == 0)
		    	return 0;
		    	
			/** Initialize temporary storage for one output frame. */
		    if (init_output_frame(&output_frame, output_codec_context, frame_size))
		        return AVERROR_EXIT;
			/**
		     * Read as many samples from the FIFO buffer as required to fill the frame.
		     * The samples are stored in the frame temporarily.
		     */
		    if (av_audio_fifo_read(fifo, (void **)output_frame->data, frame_size) < frame_size) 
		    {
				LOG4CPLUS_ERROR(logger, "read data from FIFO failed");
		        av_frame_free(&output_frame);
		        return AVERROR_EXIT;
		    }
			
			/** Encode one frame */
			int data_written;
		    if (encode_write_frame(output_frame, output_codec_context, stream_index, &data_written) < 0) {
		        av_frame_free(&output_frame);
		        return AVERROR_EXIT;
		    }
		    av_frame_free(&output_frame);
			
		    return 0;
		}

		int FFMPEGAPI_TRANSCODE::TranscodeAudio(AVAudioFifo *fifo, 
						SwrContext *resampler_context, 
						AVFrame *input_frame, 
						unsigned int stream_index)
		{			
			AVCodecContext *output_codec_context = ofmt_ctx->streams[stream_index]->codec;
			int output_frame_size = output_codec_context->frame_size;

			if(input_frame) {		
				if(convert_and_store(fifo, resampler_context, input_frame, output_codec_context) < 0)
					return -1;			
			}	

			while(av_audio_fifo_size(fifo) >= output_frame_size){
				if(load_encode_and_write(fifo, output_codec_context, stream_index) < 0)
					return -1;
			}	

			return 0;	
		}

		int FFMPEGAPI_TRANSCODE::TranscodeVideo()
		{
			return 0;
		}
		
		int FFMPEGAPI_TRANSCODE::TranscodeFmtCtx(AVFormatContext *ifmt_ctx, AVFormatContext *ofmt_ctx)
		{
			int ret;
		    AVPacket packet;
		    AVFrame *frame = NULL;
		    enum AVMediaType mediaType;
		    int stream_index;
		    int pre_index = -1;
		    unsigned int i;
		    int got_frame;
		    int (*dec_func)(AVCodecContext *, AVFrame *, int *, const AVPacket *);			    

			//AVFrame实例通常分配一次，重复使用，配合av_frame_unref使用
		    frame = av_frame_alloc();
    		if(!frame)
    		{
				ret = AVERROR(ENOMEM);
				LOG4CPLUS_ERROR(logger, "av_frame_alloc failed, error: " << ErrToString(ret));
				goto end;
    		}
    		init_packet(&packet);

		    LOG4CPLUS_INFO(logger, "################ All init success!!! ################\n\n");

		    /** read all packets */		    
			while(1)
			{
				init_packet(&packet);
				if ((ret = av_read_frame(ifmt_ctx, &packet)) < 0)
				{
			#if 0
					//无法获得AVERROR_EOF标志，所以将此部分注释，直接break
					if(ret == AVERROR_EOF)
					{
						LOG4CPLUS_INFO(logger, "av_read_frame END ^_^");
						break;
					}
					else
					{
						/** error */
						LOG4CPLUS_ERROR(logger, "av_read_frame failed, error: " << ErrToString(ret));
						goto end;
					}
			#endif
					break;
				}				

				//print packet info
				//LOG4CPLUS_DEBUG(logger, "[input_packet] " << get_packet_info(&packet));

				stream_index = packet.stream_index;
        		mediaType = ifmt_ctx->streams[packet.stream_index]->codec->codec_type;  

				//如果stream索引发生改变，且下一个stream类型为音频，则重新初始化resample_ctx 及 fifo
        		if(pre_index != stream_index)
        		{        	
        			if(!(pre_index < 0))
        			{
        				LOG4CPLUS_INFO(logger, ">>>stream_index: " << pre_index << " reencodc success ^_^");
        				if(ofmt_ctx->streams[pre_index]->codec->codec_type == AVMEDIA_TYPE_AUDIO)
        				{
							//flush fifo
							ret = load_encode_and_write(fifo, ofmt_ctx->streams[pre_index]->codec, pre_index);
							if( ret < 0)
							{
								LOG4CPLUS_ERROR(logger, "Flushing fifo failed, error: " << ErrToString(ret));
								goto end;
							}
							LOG4CPLUS_INFO(logger, "Flushing fifo succuss");
        				}        				

						/** flush encoder */
				        ret = flush_encoder(ofmt_ctx->streams[pre_index]->codec, pre_index);
				        if (ret < 0) 
				        {
				            LOG4CPLUS_ERROR(logger, "Flushing encoder failed");
				            //goto end;
				        }
				        LOG4CPLUS_INFO(logger, "Flushing encoder succuss");
        			}    
        			
					LOG4CPLUS_INFO(logger, ">>>stream_index: " << stream_index << " ready for reencoding");        			
        			
        			if(ofmt_ctx->streams[stream_index]->codec->codec_type == AVMEDIA_TYPE_AUDIO)
        			{			
        				//先释放，在重新分配
        				if (fifo)
			        		av_audio_fifo_free(fifo);
			        	swr_free(&resample_ctx);
			        	
						if(init_resampler(&resample_ctx, ifmt_ctx->streams[stream_index]->codec, ofmt_ctx->streams[stream_index]->codec) < 0)
							goto end;
						if(init_fifo(&fifo,ofmt_ctx->streams[stream_index]->codec) < 0)
							goto end;

						LOG4CPLUS_INFO(logger, ">>>stream_index: " << stream_index << " init resampler && fifo successs");
        			}        			

					pre_index = stream_index;
        		}

				//只有音频和视频类型的packet才需要重新编码，否则直接将该packet写入到输出文件里
        		if(AVMEDIA_TYPE_AUDIO == mediaType
        			|| AVMEDIA_TYPE_VIDEO == mediaType)
        		{
					/** decode packet -> frame */	        		
	        		av_packet_rescale_ts(&packet,
	                                 ifmt_ctx->streams[stream_index]->time_base,
	                                 ifmt_ctx->streams[stream_index]->codec->time_base);
	            	dec_func = (mediaType == AVMEDIA_TYPE_VIDEO) ? avcodec_decode_video2 : avcodec_decode_audio4;
	            	ret = dec_func(ifmt_ctx->streams[stream_index]->codec, frame, &got_frame, &packet);
		            if (ret < 0) 
		            {		               
		                LOG4CPLUS_ERROR(logger, "Decoding failed, error: " << ErrToString(ret));
		                goto end;
		            }
		            
		            /** got_frame=Zero if no frame could be decoded, otherwise it is non-zero*/
					if (got_frame)
					{
						frame->pts = av_frame_get_best_effort_timestamp(frame);

						if(AVMEDIA_TYPE_AUDIO == mediaType)
						{
							//transcode audio, 重采样->将重采样数据存入FIFO->从FIFO中读出数据->编码并写入输出文件
							if(TranscodeAudio(fifo, resample_ctx, frame, stream_index) < 0)
								LOG4CPLUS_ERROR(logger, "TranscodeAudio stream_index: " << stream_index << " failed");
						}
						else if(AVMEDIA_TYPE_VIDEO == mediaType)
						{
							//transcode video, 直接重新编码并写入输出文件，不进行重采样
							ret = encode_write_frame(frame, ofmt_ctx->streams[stream_index]->codec, stream_index,NULL);
							if(ret < 0)
							{
								LOG4CPLUS_ERROR(logger, "video encode_write_frame failed");
								goto end;
							}													
						}						
					}					
					
        		}
        		else	
        		{       
        			//非音视频packet直接写入输出文件
					/* remux this frame without reencoding */
		            av_packet_rescale_ts(&packet,
		                                 ifmt_ctx->streams[stream_index]->time_base,
		                                 ofmt_ctx->streams[stream_index]->time_base);
		            ret = av_interleaved_write_frame(ofmt_ctx, &packet);
		            if (ret < 0)
		            {
		            	LOG4CPLUS_ERROR(logger, "other avmedia_type av_interleaved_write_frame failed, error: " << ErrToString(ret));
		                goto end;
		            }
		            LOG4CPLUS_DEBUG(logger, "other avmedia_type av_interleaved_write_frame stream_index: " << stream_index << " success");
        		}				

				/** Unreference all the buffers referenced by frame and reset the frame fields. */
				av_frame_unref(frame);
				av_packet_unref(&packet);	            
			}

			/** flush fifo && encoders */
			if(ifmt_ctx->streams[stream_index]->codec->codec_type == AVMEDIA_TYPE_AUDIO)
			{
				//如果最后一个stream类型是音频，则刷新fifo
				ret = load_encode_and_write(fifo, ofmt_ctx->streams[stream_index]->codec, stream_index);
				if( ret < 0)
				{
					LOG4CPLUS_ERROR(logger, "Flushing fifo failed, error: " << ErrToString(ret));
					goto end;
				}
				LOG4CPLUS_INFO(logger, "Flushing fifo succuss");
			}
			
			/** flush encoder */
	        ret = flush_encoder(ofmt_ctx->streams[stream_index]->codec, stream_index);
	        if (ret < 0) 
	        {
	            LOG4CPLUS_ERROR(logger, "Flushing encoder failed");
	            goto end;
	        }
	        LOG4CPLUS_INFO(logger, "Flushing encoder succuss");
	        
			av_write_trailer(ofmt_ctx);
		    

		end:	
			av_frame_unref(frame);
			if(!packet.data)
		    	av_packet_unref(&packet);
		    if(frame)
		    	av_frame_free(&frame);
		    for (i = 0; i < ifmt_ctx->nb_streams; i++) 
		    {
		    	if (ifmt_ctx->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO)
				{
			    	if (fifo)
			    	{
			        	av_audio_fifo_free(fifo);
			        	fifo = NULL;
			        }
			        swr_free(&resample_ctx);
		    	}
		    	
		        avcodec_close(ifmt_ctx->streams[i]->codec);
		        if (ofmt_ctx && ofmt_ctx->nb_streams > i && ofmt_ctx->streams[i] && ofmt_ctx->streams[i]->codec)
		            avcodec_close(ofmt_ctx->streams[i]->codec);		        
		    }
		    
		    avformat_close_input(&ifmt_ctx);
		    //if (ofmt_ctx && !(ofmt_ctx->oformat->flags & AVFMT_NOFILE))
		    //    avio_closep(&ofmt_ctx->pb);
		    avformat_free_context(ofmt_ctx);
		    
		    if (ret < 0 && ret != AVERROR_EOF)
		        LOG4CPLUS_ERROR(logger, "Error occurred: " << ErrToString(ret));
		    return ret;
		    
		}		
		
		//从磁盘文件in_filename里面读数据作为输入，转换完成后直接存储到磁盘文件out_filename中
		int FFMPEGAPI_TRANSCODE::TranscodeFile(const char *in_filename, const char *out_filename, 
													int audio_encodec_id,
													int vedio_encodec_id)
		{			
			int ret;
			if ((ret = open_input_file(in_filename, &ifmt_ctx)) < 0)
			{
				LOG4CPLUS_ERROR(logger, "open_input_file failed");
				avformat_close_input(&ifmt_ctx);
				return ret;
			}						
			if ((ret = open_output_file(out_filename, ifmt_ctx, &ofmt_ctx, audio_encodec_id, vedio_encodec_id)) < 0)
			{
				LOG4CPLUS_ERROR(logger, "open_output_file failed");
				avformat_close_input(&ifmt_ctx);
				if (ofmt_ctx && !(ofmt_ctx->oformat->flags & AVFMT_NOFILE))
					avio_closep(&ofmt_ctx->pb);
				avformat_free_context(ofmt_ctx);
				return ret;
			}		

			return TranscodeFmtCtx(ifmt_ctx, ofmt_ctx);
		}

		//从内存in_buffer中读数据作为输入，转换后直接存储到磁盘文件out_filename中
		int FFMPEGAPI_TRANSCODE::TranscodeFile(const char *in_buffer, unsigned int in_buffer_size, const char *out_filename,
													int audio_encodec_id,
													int vedio_encodec_id)
		{
			int ret;
			if ((ret = init_input_fmt(in_buffer, in_buffer_size, &ifmt_ctx)) != 0)
			{
				LOG4CPLUS_ERROR(logger, "init_input_fmt failed, error: " <<ErrToString(ret));
				return ret;
			}

			if ((ret = open_output_file(out_filename, ifmt_ctx, &ofmt_ctx, audio_encodec_id, vedio_encodec_id)) < 0)
			{
				LOG4CPLUS_ERROR(logger, "open_output_file failed");
				avformat_close_input(&ifmt_ctx);
				if (ofmt_ctx && !(ofmt_ctx->oformat->flags & AVFMT_NOFILE))
					avio_closep(&ofmt_ctx->pb);
				avformat_free_context(ofmt_ctx);
				return ret;
			}		

			return TranscodeFmtCtx(ifmt_ctx, ofmt_ctx);
		}

		//从内存in_buffer中读取要转换的码流数据，转换完成后依然存储在内存out_buffer中
		int FFMPEGAPI_TRANSCODE::TranscodeFile(const char *in_buffer, unsigned int in_buffer_size, 
													char *out_buffer, unsigned int out_buffer_size,
													int audio_encodec_id,
													int vedio_encodec_id)
		{
			int ret;
			if ((ret = init_input_fmt(in_buffer, in_buffer_size, &ifmt_ctx)) != 0)
			{
				LOG4CPLUS_ERROR(logger, "init_input_fmt failed, error: " <<ErrToString(ret));
				return ret;
			}			

		    char *ret_ptr = NULL;
			if ((ret = init_output_fmt(ifmt_ctx, &ofmt_ctx, 
										audio_encodec_id, vedio_encodec_id, 
										out_buffer, out_buffer_size,
										&ret_ptr)) < 0)
			{
				LOG4CPLUS_ERROR(logger, "open_output_file failed");
				avformat_close_input(&ifmt_ctx);
				//if (ofmt_ctx && !(ofmt_ctx->oformat->flags & AVFMT_NOFILE))
				//	avio_closep(&ofmt_ctx->pb);
				avformat_free_context(ofmt_ctx);
				av_free(ret_ptr);
				return ret;
			}		

			if((ret = TranscodeFmtCtx(ifmt_ctx, ofmt_ctx)) < 0)
			{		
				av_free(ret_ptr);
				return ret;
			}

			av_free(ret_ptr);
			return out_buf_pos;
		}
		
	}
}



