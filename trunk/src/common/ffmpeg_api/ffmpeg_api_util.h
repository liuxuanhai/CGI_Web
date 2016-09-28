#ifndef _FFMPEG_API_UTIL_H_
#define _FFMPEG_API_UTIL_H_

#include <string>

extern "C"
{
#include <libavutil/avutil.h>
#include <libavutil/error.h>
#include <libavutil/samplefmt.h>
}

namespace common
{
	namespace ffmpegapi
	{
		#define OUTPUT_BIT_RATE 96000			/** The output bit rate in kbit/s */
		#define OUTPUT_CHANNELS 2				/** The number of output channels */

		typedef struct{
			const char 	*ptr;
			int 		size;
		}BufferData; 

		std::string ErrToString(int _errno);		
		std::string MediaTypeToString(int _type);	
		std::string AVSampleFmtToString(int _type);
	}
}

#endif