#include "ffmpeg_api_util.h"

namespace common
{
	namespace ffmpegapi
	{
		std::string ErrToString(int _errno)
		{			
			char error_buffer[255];
    		av_strerror(_errno, error_buffer, sizeof(error_buffer));
    		return error_buffer;
		}		
		std::string MediaTypeToString(int _type)
		{
			switch(_type)
			{				
				case AVMEDIA_TYPE_VIDEO:
					return "AVMEDIA_TYPE_VIDEO";
				case AVMEDIA_TYPE_AUDIO:
					return "AVMEDIA_TYPE_AUDIO";
				case AVMEDIA_TYPE_DATA:
					return "AVMEDIA_TYPE_DATA";
				case AVMEDIA_TYPE_SUBTITLE:
					return "AVMEDIA_TYPE_SUBTITLE";
				case AVMEDIA_TYPE_ATTACHMENT:
					return "AVMEDIA_TYPE_ATTACHMENT";
				case AVMEDIA_TYPE_NB:
					return "AVMEDIA_TYPE_NB";
				default:
					return "AVMEDIA_TYPE_UNKNOWN";
			}
		}

		std::string AVSampleFmtToString(int _type)
		{
			switch(_type)
			{				
				case AV_SAMPLE_FMT_U8:
					return "AV_SAMPLE_FMT_U8";		//unsigned 8 bits
				case AV_SAMPLE_FMT_S16:	
					return "AV_SAMPLE_FMT_S16";		//signed 16 bits
				case AV_SAMPLE_FMT_S32:
					return "AV_SAMPLE_FMT_S32";		//signed 32 bits
				case AV_SAMPLE_FMT_FLT:
					return "AV_SAMPLE_FMT_FLT";		//float
				case AV_SAMPLE_FMT_DBL:
					return "AV_SAMPLE_FMT_DBL";		//double
				case AV_SAMPLE_FMT_U8P:
					return "AV_SAMPLE_FMT_U8P";		//unsigned 8 bits, planner
				case AV_SAMPLE_FMT_S16P:
					return "AV_SAMPLE_FMT_S16P";	//signed 16 bits, planner
				case AV_SAMPLE_FMT_S32P:
					return "AV_SAMPLE_FMT_S32P";	//signed 32 bits, planner
				case AV_SAMPLE_FMT_FLTP:
					return "AV_SAMPLE_FMT_FLTP";	//float, planner
				case AV_SAMPLE_FMT_DBLP:
					return "AV_SAMPLE_FMT_DBLP";	//double, planner
				case AV_SAMPLE_FMT_NB:
					return "AV_SAMPLE_FMT_NB";		//Number of sample formats, DO NOT USE if linking dynamically
				default:
					return "AV_SAMPLE_FMT_NONE";				
			}
		}
	}
}

