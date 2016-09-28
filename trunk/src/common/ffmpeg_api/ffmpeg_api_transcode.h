#ifndef _FFMEPG_API_TRANSCODE_H_
#define _FFMEPG_API_TRANSCODE_H_

#include "ffmpeg_api.h"

namespace common
{
	namespace ffmpegapi
	{
		class FFMPEGAPI_TRANSCODE:public FFMPEGAPI
		{
			public:
				FFMPEGAPI_TRANSCODE()
				{
					resample_ctx = NULL;
					fifo = NULL;
				}
				~FFMPEGAPI_TRANSCODE(){};			

			private:
				
				int init_resampler(SwrContext **resample_context,
												AVCodecContext *input_codec_context,
												AVCodecContext *output_codec_context);			
				
				int init_fifo(AVAudioFifo **fifo, AVCodecContext *output_codec_context);				

				int init_output_frame(AVFrame **frame,
										 AVCodecContext *output_codec_context,
										 int frame_size);

				int init_converted_samples(uint8_t ***converted_samples,
									  AVCodecContext *output_codec_context,
									  int converted_samples_size);

				int convert_samples(SwrContext *resample_context,
									uint8_t **converted_data, const int converted_size,
									const uint8_t **input_data, const int input_size);
				int add_samples_to_fifo(AVAudioFifo *fifo,
								   uint8_t **audio_data,
								   const int data_size);
				int convert_and_store(AVAudioFifo *fifo, 
							SwrContext *resampler_context, 
							AVFrame *input_frame, 
							AVCodecContext *output_codec_context);
				int load_encode_and_write(AVAudioFifo *fifo, AVCodecContext *output_codec_context, int stream_index);



				

			public:				
				int TranscodeAudio(AVAudioFifo *fifo, 
							SwrContext *resampler_context, 
							AVFrame *input_frame, 
							unsigned int stream_index);
				int TranscodeVideo();
				int TranscodeFmtCtx(AVFormatContext *ifmt_ctx, AVFormatContext *ofmt_ctx);
				int TranscodeFile(const char *in_filename, const char *out_filename,
										int audio_encodec_id = AV_CODEC_ID_NONE,
										int vedio_encodec_id = AV_CODEC_ID_NONE);
				int TranscodeFile(const char *in_buffer, unsigned int in_buffer_size, const char *out_filename,
										int audio_encodec_id = AV_CODEC_ID_NONE,
										int vedio_encodec_id = AV_CODEC_ID_NONE);	
				int TranscodeFile(const char *in_buffer, unsigned int in_buffer_size, 
													char *out_buffer, unsigned int out_buffer_size,
													int audio_encodec_id = AV_CODEC_ID_NONE,
													int vedio_encodec_id = AV_CODEC_ID_NONE);
				
			private:			
				//audio transcode container
				SwrContext *resample_ctx;
				AVAudioFifo *fifo;	
			
		};
	}	
}


#endif
