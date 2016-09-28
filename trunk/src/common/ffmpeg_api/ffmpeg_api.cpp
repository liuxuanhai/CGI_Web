#include "ffmpeg_api.h"
#include "ffmpeg_api_util.h"

namespace common
{
	namespace ffmpegapi
	{
		IMPL_LOGGER(FFMPEGAPI, logger);		

		static int write_buf(void *opaque, uint8_t *buf, int buf_size)
		{
			//printf("write_buf!!!! \n");
			FFMPEGAPI *ffmpegapi = (FFMPEGAPI *)opaque;
			/*
			printf("out_buf:%p, out_buf_size:%d, out_buf_pos:%d, buf:%p, buf_size:%d \n", 
													ffmpegapi->out_buf,
													ffmpegapi->out_buf_size,
													ffmpegapi->out_buf_pos,
													buf,
													buf_size);
			*/
			if(ffmpegapi->out_buf_pos + buf_size > ffmpegapi->out_buf_size)
			{
				fprintf(stderr, "out_buf_size out of memory! \n");
				abort();
			}
			char *dest = ffmpegapi->out_buf + ffmpegapi->out_buf_pos;
			memcpy(dest, buf, buf_size);
			ffmpegapi->out_buf_pos += buf_size;
			return 0;
		}
		

		/** Initialize one data packet for reading or writing. */
		void FFMPEGAPI::init_packet(AVPacket *packet)
		{
		    av_init_packet(packet);
		    /** Set the packet data and size so that it is recognized as being empty. */
		    packet->data = NULL;
		    packet->size = 0;
		}

		std::string FFMPEGAPI::get_packet_info(AVPacket *packet)
		{
			/*
			 * duration: 时间长度，实际长度 = duration * AVStream->time_base.num / AVStream->time_base.den (秒)
			 * pts: Presentation Time Stamp，这帧图像什么时候显示给用户，结合 AVStream->time_base 计算
			 * dts: Decoding Time Stamp, 表示这个压缩包应该什么时候被解压，结合 AVStream->time_base 计算
			 * size: packet字节长度 (byte)
			 * pos: 位置或者便宜量，一般当前 current_pos = pre_pos + size;
			 */
			char info[128];
			sprintf(info, "stream_index: %d, duration: %lu, pts: %lu, dts: %lu, size: %d, pos: %lu", 
						packet->stream_index,
						packet->duration,
						packet->pts,
						packet->dts,
						packet->size,
						packet->pos);

			return info;
		}

		void FFMPEGAPI::av_print_stream(AVStream *stream)
		{			
			AVCodecContext *codec_ctx = stream->codec;
			const AVCodec *codec = codec_ctx->codec;

			/*
			 * bit_rate: 平均比特率
			 * ==> audio only:  1) sample_rate: 每秒的采样数
			 *				   	2) channel: number of audio channels
			 *					3) sample_fmt: audio sample format
			 *					4) frame_size: number of samples per channel in an audio frame
			 *
			 */
			
			LOG4CPLUS_INFO(logger, "frequency: " << (stream->time_base.den / stream->time_base.num) << " Hz");	
			LOG4CPLUS_INFO(logger, "bit_rate: " << codec_ctx->bit_rate << " b/s");
			LOG4CPLUS_INFO(logger, "sample_rate: " << codec_ctx->sample_rate);
			LOG4CPLUS_INFO(logger, "sample_fmt: " << AVSampleFmtToString(codec_ctx->sample_fmt));
			LOG4CPLUS_INFO(logger, "frame_size: " << codec_ctx->frame_size);
			LOG4CPLUS_INFO(logger, "codec_type: " << MediaTypeToString(codec->type));
			LOG4CPLUS_INFO(logger, "codec_id: " << codec->id);
			LOG4CPLUS_INFO(logger, "codec_name: " << codec->name << "\n");				
			
		}

		void FFMPEGAPI::av_print_format(AVFormatContext *fmt_ctx, unsigned int stream_index, const char *filename, int is_output)
		{
			AVStream *stream;		   

			std::string stream_name = "Input_stream"; 
		    if(is_output == 1)
		    {
				stream_name = "Output_stream";				
		    }

		    LOG4CPLUS_INFO(logger, "###### [" << stream_name << "_info], filename: " << filename);	

		    if(stream_index == 0)
		    {
				unsigned int i = 0;
				for (i = 0; i < fmt_ctx->nb_streams; i++) 
			    {   
					stream = fmt_ctx->streams[i];

					LOG4CPLUS_INFO(logger, ">>>stream_index: " << i);
					av_print_stream(stream);
				}							
		    }
		    else
		    {
				if(stream_index >= fmt_ctx->nb_streams)
				{
					LOG4CPLUS_ERROR(logger, "Invild param[stream_index]");
					return;
				}

				stream = fmt_ctx->streams[stream_index];
				LOG4CPLUS_INFO(logger, ">>>stream_index: " << stream_index);
				av_print_stream(stream);
		    }		    
			
		}	

		int FFMPEGAPI::read_packet(void *opaque, uint8_t *buf, int buf_size)
		{			
		    BufferData *bd = (BufferData *)opaque;
		    //LOG4CPLUS_DEBUG(logger, "dst_buf_size: " << buf_size << ", BufferData.size: " << bd->size);
		    
		    buf_size = FFMIN(buf_size, bd->size);		    
		    /* copy internal buffer data to buf */
		    memcpy(buf, bd->ptr, buf_size);
		    bd->ptr  += buf_size;
		    bd->size -= buf_size;
		    return buf_size;
		}

		int FFMPEGAPI::init_input_fmt(const char *in_buffer, size_t in_buffer_size, AVFormatContext **fmt_ctx)
		{			
			AVIOContext *avio_ctx = NULL;
			uint8_t *avio_ctx_buffer = NULL;
			size_t avio_ctx_buffer_size = 4096;			
			int ret = 0;
			BufferData stBufferData;				
			
			/* fill opaque structure used by the AVIOContext read callback */
			stBufferData.ptr  = in_buffer;
			stBufferData.size = in_buffer_size;
			if (!(*fmt_ctx = avformat_alloc_context())) {				
			    ret = AVERROR(ENOMEM);			    
			    LOG4CPLUS_ERROR(logger, "avformat_alloc_context failed, error: " << ret);
			    return ret;
			}
			avio_ctx_buffer = (uint8_t *)av_malloc(avio_ctx_buffer_size);
			if (!avio_ctx_buffer) {
				avformat_close_input(fmt_ctx);
			    ret = AVERROR(ENOMEM);		
			    LOG4CPLUS_ERROR(logger, "av_malloc failed, error: " << ret);
			    return ret;
			}
			//将stBufferData里的数据，通过read_packet函数写入到avio_ctx_buffer指向的以avio_ctx_buffer_size为单位分配的地址空间
			//如果空间不足，系统会以avio_ctx_buffer_size为单位自动分配足够存储的内存空间。
			avio_ctx = avio_alloc_context(avio_ctx_buffer, avio_ctx_buffer_size,
			                              0, &stBufferData, &read_packet, NULL, NULL);
			if (!avio_ctx) {
				avformat_close_input(fmt_ctx);
				av_free(avio_ctx_buffer);
			    ret = AVERROR(ENOMEM);
			    LOG4CPLUS_ERROR(logger, "avio_alloc_context failed, error: " << ret);
			    return ret;
			}
			(*fmt_ctx)->pb = avio_ctx;

			ret = avformat_open_input(fmt_ctx, NULL, NULL, NULL);
		    if (ret < 0) {
		        LOG4CPLUS_ERROR(logger, "Could not open input");
		        return ret;
		    }
		    
		    ret = avformat_find_stream_info(*fmt_ctx, NULL);
		    if (ret < 0) {
		        LOG4CPLUS_ERROR(logger, "Could not find stream information");
		        return ret;
		    }

		    unsigned int i = 0;
		    for (i = 0; i < (*fmt_ctx)->nb_streams; i++) 
		    {		    
		        AVStream *stream;
		        AVCodecContext *codec_ctx;
		        stream = (*fmt_ctx)->streams[i];
		        codec_ctx = stream->codec;
		        /* Reencode video & audio and remux subtitles etc. */
		        if (AVMEDIA_TYPE_VIDEO == codec_ctx->codec_type 
		        	|| AVMEDIA_TYPE_AUDIO == codec_ctx->codec_type) 
		        {
		            /* Open decoder */
		            ret = avcodec_open2(codec_ctx, avcodec_find_decoder(codec_ctx->codec_id), NULL);
		            if (ret < 0) 
		            {
		            	LOG4CPLUS_ERROR(logger, "Failed to open decoder for stream, codec_id: " << 
		            							codec_ctx->codec->id << ", codec_name: " <<
		            							codec_ctx->codec->name);		                
		                return ret;
		            }
		        }
		    }

			/** Print detailed information about the input format */
		    av_dump_format(*fmt_ctx, 0, "data_buffer", 0);	//print => stdin
		    av_print_format(*fmt_ctx, 0, "data_buffer", 0);	//print => logfile

			return 0;
			
		}

		int FFMPEGAPI::init_AVOutputFormat(AVOutputFormat *ofmt,
												int audio_encodec_id,
												int vedio_encodec_id)
		{
			ofmt->flags = AVFMT_FLAG_CUSTOM_IO;
			ofmt->video_codec = (enum AVCodecID)vedio_encodec_id;
			ofmt->audio_codec = (enum AVCodecID)audio_encodec_id;

			return 0;
		}		

		//这个地方要如果要设置输出参数的话，实在是太多了,如果有全面的ffmpeg知识，可以考虑传一个指定AVCodecContext过来!!!
		int FFMPEGAPI::init_output_fmt(AVFormatContext *ifmt_ctx, 												
												AVFormatContext **ofmt_ctx, 
												int audio_encodec_id,
												int vedio_encodec_id,												
												char *_out_buf,
												int _out_buf_size,
												char **ret_ptr)
		{
		    AVStream *out_stream;
		    AVStream *in_stream;
		    AVCodecContext *dec_ctx, *enc_ctx;
		    AVCodec *encoder;
		    int ret;
		    unsigned int i;
		    *ofmt_ctx = NULL;		    
		    out_buf = _out_buf;
		    out_buf_size = _out_buf_size;
		
			// 自定义输出AVIOContext，ofmt_ctx输出到this->out_ptr指定的内存
		    AVIOContext *avio_out = NULL;	
		    char *avio_outbuf = (char *)av_malloc(1024 * 8);
		    *ret_ptr = avio_outbuf; 
		    avio_out = avio_alloc_context((unsigned char *)avio_outbuf, 1024 * 8, AVIO_FLAG_WRITE,
		    							this, NULL, write_buf, NULL);

		    if(!avio_out)
		    {
				LOG4CPLUS_ERROR(logger, "avio_alloc_context failed");
				return AVERROR_UNKNOWN;
		    }		    

		    // 定义ofmt_ctx
		    /*
			AVOutputFormat ofmt;
			init_AVOutputFormat(&ofmt, vedio_encodec_id, audio_encodec_id);
			
			avformat_alloc_output_context2(ofmt_ctx, &ofmt, NULL, NULL);
			*/
			avformat_alloc_output_context2(ofmt_ctx, NULL, "mp3", NULL);
			if (!(*ofmt_ctx)) 
			{
				LOG4CPLUS_ERROR(logger, "Could not create output context");
				return AVERROR_UNKNOWN;
			}
			
			(*ofmt_ctx)->pb = avio_out;							//赋值自定义的IO结构体
			(*ofmt_ctx)->flags = AVFMT_FLAG_CUSTOM_IO;			//指定为自定义
		    
		    for (i = 0; i < ifmt_ctx->nb_streams; i++) 
		    {   
				in_stream = ifmt_ctx->streams[i];
				dec_ctx = in_stream->codec;				
				
				if(AVMEDIA_TYPE_VIDEO == dec_ctx->codec_type 
					|| AVMEDIA_TYPE_AUDIO == dec_ctx->codec_type)
				{
					/** Create a new audio stream in the output file container. */
					out_stream = avformat_new_stream(*ofmt_ctx, NULL);			
				    if (!out_stream) 
				    {
				        LOG4CPLUS_ERROR(logger, "Failed allocating output stream");
				        return AVERROR_UNKNOWN;
				    }

				    /** Save the encoder context for easier access later. */
					enc_ctx = out_stream->codec;
					
					/**
				     * Set the basic encoder parameters.
				     * The input file's sample rate is used to avoid a sample rate conversion.
				     */			
				    if(AVMEDIA_TYPE_VIDEO == dec_ctx->codec_type)
				    {				    	
				    	//enc_ctx->codec_id = AV_CODEC_ID_H264; 						//指定编码器
				    	//enc_ctx->codec_id = dec_ctx->codec_id; 							//指定原来的编码器
						enc_ctx->codec_id = (*ofmt_ctx)->oformat->video_codec;		//不指定，使用根据输出文件后缀名提取的默认编码器
						if(vedio_encodec_id != AV_CODEC_ID_NONE)
						{
							enc_ctx->codec_id = (enum AVCodecID)vedio_encodec_id;
						}
						
						encoder = avcodec_find_encoder(enc_ctx->codec_id);				
				        if (!encoder) {
				            LOG4CPLUS_FATAL(logger, "Necessary video encoder not found");
				            return AVERROR_INVALIDDATA;
				        }
						enc_ctx->codec_type = AVMEDIA_TYPE_VIDEO;						//编码器的类型(视频)
						//enc_ctx->pix_fmt = AV_PIX_FMT_YUV420P;							//图像像素格式
						enc_ctx->pix_fmt = dec_ctx->pix_fmt;							//图像像素格式
			            enc_ctx->height = dec_ctx->height;								//视频高
			            enc_ctx->width = dec_ctx->width;           						//视频宽
						enc_ctx->time_base = dec_ctx->time_base;						//根据该参数，可以把PTS转化成为实际的时间(单位为秒S)
			            enc_ctx->sample_aspect_ratio = dec_ctx->sample_aspect_ratio;	//宽高比，带分子:分母的一个结构体 如 9:16

						out_stream->time_base = in_stream->time_base;

						if(enc_ctx->codec_id == AV_CODEC_ID_H264)
						{
							av_opt_set(enc_ctx->priv_data, "preset", "slow", 0); 	

							//H264
							{				
								enc_ctx->me_range = 16;  
				    			enc_ctx->max_qdiff = 4;  
				    			enc_ctx->qcompress = 0.6;  
				    			enc_ctx->qmin = 10;  
				    			enc_ctx->qmax = 51;  		
							}
						}						
				    }
				    else if(AVMEDIA_TYPE_AUDIO == dec_ctx->codec_type)
				    {				    	
				    	//enc_ctx->codec_id = AV_CODEC_ID_MP3; 						//指定编码器 [AV_CODEC_ID_AAC 等]
					    enc_ctx->codec_id = (*ofmt_ctx)->oformat->audio_codec;		//不指定，使用根据输出文件后缀名提取的默认编码器
					    if(audio_encodec_id != AV_CODEC_ID_NONE)
					    {
							enc_ctx->codec_id = (enum AVCodecID)audio_encodec_id;
					    }
					    
					    encoder = avcodec_find_encoder(enc_ctx->codec_id);		    	
					    if (!encoder) {
							LOG4CPLUS_ERROR(logger, "Necessary audio encoder not found");
					        return -1;
					    }		    

					    enc_ctx->codec_type 	= AVMEDIA_TYPE_AUDIO;					//编码器的类型(音频)
					    enc_ctx->channel_layout = dec_ctx->channel_layout;				//音频声道布局
					    enc_ctx->channels       = av_get_channel_layout_nb_channels(enc_ctx->channel_layout);	//音频声道数
					    enc_ctx->sample_rate    = dec_ctx->sample_rate;					//音频采样率
					    //enc_ctx->sample_fmt     = encoder->sample_fmts[0];				//音频采样格式
					    enc_ctx->sample_fmt     = dec_ctx->sample_fmt;				//音频采样格式
					    //enc_ctx->bit_rate       = OUTPUT_BIT_RATE;						//音频比特率
					    enc_ctx->bit_rate       = dec_ctx->bit_rate;						//音频比特率
					    if(enc_ctx->codec_id == AV_CODEC_ID_AAC)
					    {
							/** Allow the use of the experimental AAC encoder */
					    	enc_ctx->strict_std_compliance = FF_COMPLIANCE_EXPERIMENTAL;	//#define FF_COMPLIANCE_EXPERIMENTAL -2 ///< Allow nonstandardized experimental things.
					    }					    
					    /** Set the sample rate for the container. */
						enc_ctx->time_base 	  = dec_ctx->time_base;						//根据该参数，可以把PTS转化成为实际的时间(单位为秒S)
					    out_stream->time_base	  = in_stream->time_base;
				    }			
					
					/** Initialize the AVCodecContext to use the given AVCodec */
					ret =avcodec_open2(enc_ctx, encoder, NULL);
					if (ret < 0) {  
		               LOG4CPLUS_ERROR(logger, "open encoder for output_stream failed");  
		               return ret;  
		            } 					
					
				}		
				else if(dec_ctx->codec_type == AVMEDIA_TYPE_UNKNOWN)
				{
					LOG4CPLUS_FATAL(logger, "Elementary stream #" << i << " is of unknown type, cannot proceed");
		            return AVERROR_INVALIDDATA;
				}
				else	//subtitle
				{
					/* if this stream must be remuxed */
					out_stream = avformat_new_stream(*ofmt_ctx, NULL);			
				    if (!out_stream) 
				    {
				        LOG4CPLUS_ERROR(logger, "Failed allocating output stream");
				        return AVERROR_UNKNOWN;
				    }
					
					ret = avcodec_copy_context((*ofmt_ctx)->streams[i]->codec,
		                    ifmt_ctx->streams[i]->codec);
		            if (ret < 0) 
		            {
		                LOG4CPLUS_ERROR(logger, "Copying stream context failed");
		                return ret;
		            }
				}         

				/**
			     * Some container formats (like MP4) require global headers to be present
			     * Mark the encoder so that it behaves accordingly.
			     */
			    if ((*ofmt_ctx)->oformat->flags & AVFMT_GLOBALHEADER)
		        	enc_ctx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
		    }
		    
		    /** Print detailed information about the output format */
		    av_dump_format(*ofmt_ctx, 0, NULL, 1);	//print => stdin
		    av_print_format(*ofmt_ctx, 0, NULL, 1);	//print => logfile
		    
		    /* init muxer, write output file header */
		    ret = avformat_write_header(*ofmt_ctx, NULL);
		    if (ret < 0) 
		    {
		        LOG4CPLUS_ERROR(logger, "avformat_write_header failed, error: " << ErrToString(ret));
		        return ret;
		    }
		    return 0;
		}

		int FFMPEGAPI::open_input_file(const char *filename, AVFormatContext **ifmt_ctx)
		{
		    int ret;		    
		    *ifmt_ctx = NULL;
		    if ((ret = avformat_open_input(ifmt_ctx, filename, NULL, NULL)) < 0) 
		    {
		    	LOG4CPLUS_ERROR(logger, "Cannot open input file, error: " << ErrToString(ret));			    	
		        return ret;
		    }
		    if ((ret = avformat_find_stream_info(*ifmt_ctx, NULL)) < 0) 
		    {
		    	LOG4CPLUS_ERROR(logger, "Cannot find stream information, error: " << ErrToString(ret));		    	
		        return ret;
		    }					

		    unsigned int i = 0;
		    for (i = 0; i < (*ifmt_ctx)->nb_streams; i++) 
		    {		    
		        AVStream *stream;
		        AVCodecContext *codec_ctx;
		        stream = (*ifmt_ctx)->streams[i];
		        codec_ctx = stream->codec;
		        /* Reencode video & audio and remux subtitles etc. */
		        if (AVMEDIA_TYPE_VIDEO == codec_ctx->codec_type 
		        	|| AVMEDIA_TYPE_AUDIO == codec_ctx->codec_type) 
		        {
		            /* Open decoder */
		            ret = avcodec_open2(codec_ctx, avcodec_find_decoder(codec_ctx->codec_id), NULL);
		            if (ret < 0) 
		            {
		            	LOG4CPLUS_ERROR(logger, "Failed to open decoder for stream, codec_id: " << 
		            							codec_ctx->codec->id << ", codec_name: " <<
		            							codec_ctx->codec->name);		                
		                return ret;
		            }
		        }
		    }
		    
		    /** Print detailed information about the input format */
		    av_dump_format(*ifmt_ctx, 0, filename, 0);	//print => stdin
		    av_print_format(*ifmt_ctx, 0, filename, 0);	//print => logfile
		    
		    return 0;
		}

		//这个地方要如果要设置输出参数的话，实在是太多了,如果有全面的ffmpeg知识，可以考虑传一个指定AVCodecContext过来!!!
		int FFMPEGAPI::open_output_file(const char *filename, AVFormatContext *ifmt_ctx, AVFormatContext **ofmt_ctx, 
												int audio_encodec_id,
												int vedio_encodec_id)
		{
		    AVStream *out_stream;
		    AVStream *in_stream;
		    AVCodecContext *dec_ctx, *enc_ctx;
		    AVCodec *encoder;
		    int ret;
		    unsigned int i;
		    *ofmt_ctx = NULL;
		    avformat_alloc_output_context2(ofmt_ctx, NULL, NULL, filename);
		    if (!(*ofmt_ctx)) 
		    {
		        LOG4CPLUS_ERROR(logger, "Could not create output context");
		        return AVERROR_UNKNOWN;
		    }
		    for (i = 0; i < ifmt_ctx->nb_streams; i++) 
		    {   
				in_stream = ifmt_ctx->streams[i];
				dec_ctx = in_stream->codec;				
				
				if(AVMEDIA_TYPE_VIDEO == dec_ctx->codec_type 
					|| AVMEDIA_TYPE_AUDIO == dec_ctx->codec_type)
				{
					/** Create a new audio stream in the output file container. */
					out_stream = avformat_new_stream(*ofmt_ctx, NULL);			
				    if (!out_stream) 
				    {
				        LOG4CPLUS_ERROR(logger, "Failed allocating output stream");
				        return AVERROR_UNKNOWN;
				    }

				    /** Save the encoder context for easier access later. */
					enc_ctx = out_stream->codec;
					
					/**
				     * Set the basic encoder parameters.
				     * The input file's sample rate is used to avoid a sample rate conversion.
				     */			
				    if(AVMEDIA_TYPE_VIDEO == dec_ctx->codec_type)
				    {				    	
				    	//enc_ctx->codec_id = AV_CODEC_ID_H264; 						//指定编码器
				    	//enc_ctx->codec_id = dec_ctx->codec_id; 							//指定原来的编码器
						enc_ctx->codec_id = (*ofmt_ctx)->oformat->video_codec;		//不指定，使用根据输出文件后缀名提取的默认编码器
						if(vedio_encodec_id != AV_CODEC_ID_NONE)
						{
							enc_ctx->codec_id = (enum AVCodecID)vedio_encodec_id;
						}
						
						encoder = avcodec_find_encoder(enc_ctx->codec_id);				
				        if (!encoder) {
				            LOG4CPLUS_FATAL(logger, "Necessary video encoder not found");
				            return AVERROR_INVALIDDATA;
				        }
						enc_ctx->codec_type = AVMEDIA_TYPE_VIDEO;						//编码器的类型(视频)
						//enc_ctx->pix_fmt = AV_PIX_FMT_YUV420P;							//图像像素格式
						enc_ctx->pix_fmt = dec_ctx->pix_fmt;							//图像像素格式
			            enc_ctx->height = dec_ctx->height;								//视频高
			            enc_ctx->width = dec_ctx->width;           						//视频宽
						enc_ctx->time_base = dec_ctx->time_base;						//根据该参数，可以把PTS转化成为实际的时间(单位为秒S)
			            enc_ctx->sample_aspect_ratio = dec_ctx->sample_aspect_ratio;	//宽高比，带分子:分母的一个结构体 如 9:16

						out_stream->time_base = in_stream->time_base;

						if(enc_ctx->codec_id == AV_CODEC_ID_H264)
						{
							av_opt_set(enc_ctx->priv_data, "preset", "slow", 0); 	

							//H264
							{				
								enc_ctx->me_range = 16;  
				    			enc_ctx->max_qdiff = 4;  
				    			enc_ctx->qcompress = 0.6;  
				    			enc_ctx->qmin = 10;  
				    			enc_ctx->qmax = 51;  		
							}
						}						
				    }
				    else if(AVMEDIA_TYPE_AUDIO == dec_ctx->codec_type)
				    {				    	
				    	//enc_ctx->codec_id = AV_CODEC_ID_MP3; 						//指定编码器 [AV_CODEC_ID_AAC 等]
					    enc_ctx->codec_id = (*ofmt_ctx)->oformat->audio_codec;		//不指定，使用根据输出文件后缀名提取的默认编码器
					    if(audio_encodec_id != AV_CODEC_ID_NONE)
					    {
							enc_ctx->codec_id = (enum AVCodecID)audio_encodec_id;
					    }
					    
					    encoder = avcodec_find_encoder(enc_ctx->codec_id);		    	
					    if (!encoder) {
							LOG4CPLUS_ERROR(logger, "Necessary audio encoder not found");
					        return -1;
					    }		    

					    enc_ctx->codec_type 	= AVMEDIA_TYPE_AUDIO;					//编码器的类型(音频)
					    enc_ctx->channel_layout = dec_ctx->channel_layout;				//音频声道布局
					    enc_ctx->channels       = av_get_channel_layout_nb_channels(enc_ctx->channel_layout);	//音频声道数
					    enc_ctx->sample_rate    = dec_ctx->sample_rate;					//音频采样率
					    //enc_ctx->sample_fmt     = encoder->sample_fmts[0];				//音频采样格式
					    enc_ctx->sample_fmt     = dec_ctx->sample_fmt;				//音频采样格式
					    //enc_ctx->bit_rate       = OUTPUT_BIT_RATE;						//音频比特率
					    enc_ctx->bit_rate       = dec_ctx->bit_rate;						//音频比特率
					    if(enc_ctx->codec_id == AV_CODEC_ID_AAC)
					    {
							/** Allow the use of the experimental AAC encoder */
					    	enc_ctx->strict_std_compliance = FF_COMPLIANCE_EXPERIMENTAL;	//#define FF_COMPLIANCE_EXPERIMENTAL -2 ///< Allow nonstandardized experimental things.
					    }					    
					    /** Set the sample rate for the container. */
						enc_ctx->time_base 	  = dec_ctx->time_base;						//根据该参数，可以把PTS转化成为实际的时间(单位为秒S)
					    out_stream->time_base	  = in_stream->time_base;
				    }			
					
					/** Initialize the AVCodecContext to use the given AVCodec */
					ret =avcodec_open2(enc_ctx, encoder, NULL);
					if (ret < 0) {  
		               LOG4CPLUS_ERROR(logger, "open encoder for output_stream failed");  
		                return ret;  
		            } 					
					
				}		
				else if(dec_ctx->codec_type == AVMEDIA_TYPE_UNKNOWN)
				{
					LOG4CPLUS_FATAL(logger, "Elementary stream #" << i << " is of unknown type, cannot proceed");
		            return AVERROR_INVALIDDATA;
				}
				else	//subtitle
				{
					/* if this stream must be remuxed */
					out_stream = avformat_new_stream(*ofmt_ctx, NULL);			
				    if (!out_stream) 
				    {
				        LOG4CPLUS_ERROR(logger, "Failed allocating output stream");
				        return AVERROR_UNKNOWN;
				    }
					
					ret = avcodec_copy_context((*ofmt_ctx)->streams[i]->codec,
		                    ifmt_ctx->streams[i]->codec);
		            if (ret < 0) 
		            {
		                LOG4CPLUS_ERROR(logger, "Copying stream context failed");
		                return ret;
		            }
				}         

				/**
			     * Some container formats (like MP4) require global headers to be present
			     * Mark the encoder so that it behaves accordingly.
			     */
			    if ((*ofmt_ctx)->oformat->flags & AVFMT_GLOBALHEADER)
		        	enc_ctx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
		    }
		    
		    /** Print detailed information about the output format */
		    av_dump_format(*ofmt_ctx, 0, filename, 1);	//print => stdin
		    av_print_format(*ofmt_ctx, 0, filename, 1);	//print => logfile
		    if (!((*ofmt_ctx)->oformat->flags & AVFMT_NOFILE)) 
		    {
		        ret = avio_open(&(*ofmt_ctx)->pb, filename, AVIO_FLAG_WRITE);
		        if (ret < 0) 
		        {
		            LOG4CPLUS_ERROR(logger, "Could not open output file " << filename);
		            return ret;
		        }
		    }
		    /* init muxer, write output file header */
		    ret = avformat_write_header(*ofmt_ctx, NULL);
		    if (ret < 0) 
		    {
		        LOG4CPLUS_ERROR(logger, "avformat_write_header failed, error: " << ErrToString(ret));
		        return ret;
		    }
		    return 0;
		}

		/** Encode one frame to the output file. */
		int FFMPEGAPI::encode_write_frame(AVFrame *frame, AVCodecContext *output_codec_context, int stream_index, int *got_frame)
		{
			/** Packet used for temporary storage. */
		    AVPacket output_packet;
		    int ret;
		    int got_frame_local;
		    init_packet(&output_packet);

		    if(!got_frame)
		    {
				got_frame = &got_frame_local;
		    }

		    /** Set a timestamp based on the sample rate for the container. */
		    #if 0
		    if (frame) 
		    {
		        frame->pts = pts[stream_index];	
		        pts[stream_index] += frame->nb_samples;
		    }
		    #endif

		    int (*enc_func)(AVCodecContext *, AVPacket *, const AVFrame *, int *) =
		        (output_codec_context->codec_type == AVMEDIA_TYPE_VIDEO) ? avcodec_encode_video2 : avcodec_encode_audio2;
		    
		    /**
		     * Encode the frame and store it in the temporary packet.
		     * The output stream encoder is used to do this.
		     * ==> avcodec_encode_audio2
		     * Param:[in]frame: containing the raw audio data to be encoded. 
		     *					May be NULL when flushing an encoder that has the AV_CODEC_CAP_DELAY capability set. 
		     *					If AV_CODEC_CAP_VARIABLE_FRAME_SIZE is set, then each frame can have any number of samples. 
		     *					If it is not set, frame->nb_samples must be equal to output_codec_context->frame_size for all frames except the last. 
		     *					The final frame may be smaller than output_codec_context->frame_size.
		     * 		 [out]got_frame: is set to 1 by libavcodec if the output packet is non-empty, and to 0 if it is empty.
		     */

			#if 0
			if(frame)
		    	LOG4CPLUS_DEBUG(logger, "encoding frame->nb_samples: " << frame->nb_samples << ", output_codec_context->frame_size: " << output_codec_context->frame_size);
		    else
		    	LOG4CPLUS_DEBUG(logger, "frame is NULL, flash output_codec_context");
			#endif
		    	
		    ret = enc_func(output_codec_context, &output_packet, frame, got_frame);
		    if(ret < 0)
		    {
				LOG4CPLUS_ERROR(logger, "encode frame failed, error: "<<ErrToString(ret));
				return ret;
		    }

			
		    /** Set Packet PTS/DTS. */
		    //此处，如果packet压缩的是视频类型的码流，不设置正确的packet.PTS 和 DTS，视频播放顺序失控，不能和音频同步
			output_packet.stream_index = stream_index;			
			
			//LOG4CPLUS_DEBUG(logger, "[output_packet] " << get_packet_info(&output_packet));

			if(*got_frame)
			{
				if ((ret = av_write_frame(ofmt_ctx, &output_packet)) < 0) 
				{
					LOG4CPLUS_ERROR(logger, "write frame failed, error: "<<ErrToString(ret));		            
		            av_packet_unref(&output_packet);
		            return ret;
		        }
		        av_packet_unref(&output_packet);
			}

			return 0;
		}
		
		int FFMPEGAPI::flush_encoder(AVCodecContext *output_codec_context, int stream_index)
		{
			int ret;
			int got_frame;
			/** [in_param]frame May be NULL when flushing an encoder that has the AV_CODEC_CAP_DELAY capability set.*/
			if (!(output_codec_context->codec->capabilities &
						AV_CODEC_CAP_DELAY))
				return 0;
				
			while (1) {				
				ret = encode_write_frame(NULL, output_codec_context, stream_index, &got_frame);
				if (ret < 0)
					break;
				if (!got_frame)
					return 0;
			}
			return ret;
		}
		
	}
}
