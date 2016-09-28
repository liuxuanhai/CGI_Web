#ifndef _HOOSHO_COMMON_IMG_IMAGE_HELPER_H_
#define _HOOSHO_COMMON_IMG_IMAGE_HELPER_H_

#include <string>
#include "util/logger.h"

namespace common
{
	namespace image
	{
		class ImageHelper
		{
		public:
			enum ImageType
			{
				T_MIN			 = 0,
				T_BMP			 = 1,
				T_JPEG			 = 2,
				T_PNG			 = 3,
				T_GIF			 = 4,
				T_MAX			 = 5
			};
		
			enum ErrorType
			{
				E_OK				= 0,
				E_BUFFER_EMPTY		= 1,
				E_BUFFER_NOT_ENOUGH = 2,
				E_INVALID_TYPE		= 3,
				E_INVALID_CONTENT	= 4,
				E_INVALID_FORMAT	= 5,
				E_UNKNOWN			= 6
			};
		
			static std::string type_string(const ImageType& imagetype);
			static std::string error_string(const ErrorType& ec);
		
		public:
			ImageHelper();
			virtual ~ImageHelper();
			int build(char* buffer, size_t buffer_len);
		
		public:
			size_t get_width()
			{
				return _image_width;
			}
		
			size_t get_height()
			{
				return _image_heigth;
			}
		
			int get_format()
			{
				return _image_format;
			}
		
		public:
			int fixed_square_thumbnails(std::string& out
							, size_t size
							, ImageType dst_image_type);
							
			int constrain_thumbnails(
							  size_t dst_width		   // in, width you want, in pixels
							, size_t dst_heigth 	   // in, heigth you want, in pixels
							, ImageType dst_image_type // in, target format you want
							, std::string& out		   // out, target data
							, size_t& real_width	   // out, target real width, in pixels
							, size_t& real_heigth);    // out, target real heigth, in pixels
		
			int crop(size_t x_offset 
						, size_t y_offset
						, size_t witdth
						, size_t height
						, ImageType image_type
						, std::string& out_data);
		
		private:
			char*	  _buffer;
			size_t	  _blen;
			int 	  _image_format;
			size_t	  _image_width;
			size_t	  _image_heigth;
		
		protected:
			DECL_LOGGER(logger);
		};
				
	}
}

#endif




