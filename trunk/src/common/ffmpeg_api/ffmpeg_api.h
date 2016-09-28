#ifndef _FFMPEG_API_H_
#define _FFMPEG_API_H_

#include <stdio.h>
#include <stdint.h>

extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavformat/avio.h>
#include <libavfilter/avfiltergraph.h>
#include <libavfilter/buffersink.h>
#include <libavfilter/buffersrc.h>
#include <libavutil/opt.h>
#include <libavutil/pixdesc.h>
#include <libavutil/audio_fifo.h>
#include <libavutil/avassert.h>
#include <libavutil/avstring.h>
#include <libavutil/frame.h>
#include <libswresample/swresample.h>
}

#include "ffmpeg_api_logger.h"

namespace common
{
	namespace ffmpegapi
	{			
		class FFMPEGAPI
		{
			public:
				FFMPEGAPI()
				{
					/** Initialize libavformat and register all the muxers, demuxers and protocols. */
					av_register_all();		
				}
				~ FFMPEGAPI(){}

			public:
				

			protected:
				DECL_LOGGER(logger);

			public:		
				void init_packet(AVPacket *packet); 
				std::string get_packet_info(AVPacket *packet);
				void av_print_stream(AVStream *stream);
				void av_print_format(AVFormatContext *fmt_ctx, unsigned int stream_index, const char *filename, int is_output);

				static int read_packet(void *opaque, uint8_t *buf, int buf_size);
				int init_input_fmt(const char *in_buffer, size_t in_buffer_size, AVFormatContext **fmt_ctx);
				int init_AVOutputFormat(AVOutputFormat *ofmt, int audio_encodec_id, int vedio_encodec_id);
				int init_output_fmt(AVFormatContext *ifmt_ctx, 												
												AVFormatContext **ofmt_ctx, 
												int audio_encodec_id,
												int vedio_encodec_id,												
												char *_out_buf,
												int _out_buf_size,
												char **ret_ptr);
				
				int open_input_file(const char *filename, AVFormatContext **ifmt_ctx);
				int open_output_file(const char *filename, AVFormatContext *ifmt_ctx, AVFormatContext **ofmt_ctx,
											int audio_encodec_id = AV_CODEC_ID_NONE,
											int vedio_encodec_id = AV_CODEC_ID_NONE);
				
				int encode_write_frame(AVFrame *frame, AVCodecContext *output_codec_context, int stream_index, int *got_frame);				
				int flush_encoder(AVCodecContext *output_codec_context, int stream_index);				
				          

			public:				
				AVFormatContext *ifmt_ctx;
				AVFormatContext *ofmt_ctx;
				char *out_buf;
				int out_buf_size;
				int out_buf_pos;

		};
	}
}

#endif