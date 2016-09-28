#include "image_helper.h"
#include "Magick++.h"
#include <cassert>
#include <sstream>
#include <exception>

IMPL_LOGGER(ImageHelper, logger);


std::string ImageHelper::type_string(const ImageType& imagetype)
{
    int type = static_cast<int> (imagetype);
    std::string rv = "";

#define IMAGE_HELPER_IMAGE_TYPE_CASE(t, s) \
case t: \
	rv = s; \
	break;

    switch(type)
    {
        IMAGE_HELPER_IMAGE_TYPE_CASE(T_MIN, "unknown(min)");
        IMAGE_HELPER_IMAGE_TYPE_CASE(T_BMP, "BMP");
        IMAGE_HELPER_IMAGE_TYPE_CASE(T_JPEG, "JPEG");
        IMAGE_HELPER_IMAGE_TYPE_CASE(T_PNG, "PNG");
        IMAGE_HELPER_IMAGE_TYPE_CASE(T_GIF, "GIF");
        IMAGE_HELPER_IMAGE_TYPE_CASE(T_MAX, "unknown(max)");

    default:
        rv = "unknown(default)";
    }
#undef IMAGE_HELPER_IMAGE_TYPE_CASE

    return rv;
}

std::string ImageHelper::error_string(const ErrorType& ec)
{
    int error = static_cast<int>(ec);
    std::string rv = "";

#define IMAGE_HELPER_ERROR_TYPE_CASE(e) \
	case e: \
		rv = #e; \
		break;

    switch(error)
    {
        IMAGE_HELPER_ERROR_TYPE_CASE(E_OK);
        IMAGE_HELPER_ERROR_TYPE_CASE(E_BUFFER_EMPTY);
        IMAGE_HELPER_ERROR_TYPE_CASE(E_BUFFER_NOT_ENOUGH);
        IMAGE_HELPER_ERROR_TYPE_CASE(E_INVALID_TYPE);
        IMAGE_HELPER_ERROR_TYPE_CASE(E_INVALID_CONTENT);
        IMAGE_HELPER_ERROR_TYPE_CASE(E_INVALID_FORMAT);
        IMAGE_HELPER_ERROR_TYPE_CASE(E_UNKNOWN);

    default:
        assert(false);
    }
#undef IMAGE_HELPER_ERROR_TYPE_CASE

    return rv;

}

ImageHelper::ImageHelper()
    : _buffer(NULL)
    , _blen(0)
    , _image_width(0)
    , _image_heigth(0)
{
}

ImageHelper::~ImageHelper()
{

}

int ImageHelper::build(char* buffer, size_t buffer_len)
{
    if((NULL==buffer) || (0==buffer_len))
    {
        return E_BUFFER_EMPTY;
    }

    _buffer = buffer;
    _blen = buffer_len;

    try
    {
        Magick::Blob r_blob = Magick::Blob(_buffer, _blen);
		Magick::Image image;
        image.read(r_blob);
        _image_width = image.columns();
        _image_heigth = image.rows();
        std::string format = image.magick();
        LOG4CPLUS_DEBUG(logger, "ImageHelper::build, format="<<format);
        if(format.find("BMP") != std::string::npos)
        {
            _image_format = T_BMP;
        }
        else if("JPEG" == format)
        {
            _image_format = T_JPEG;
        }
        else if("PNG" == format)
        {
            _image_format = T_PNG;
        }
        else if("GIF" == format)
        {
            _image_format = T_GIF;
        }
        else
        {
            return E_INVALID_FORMAT;
        }
    }
    catch(std::exception& e)
    {
        return E_INVALID_CONTENT;
    }
    catch(...)
    {
        return E_INVALID_CONTENT;
    }

    return E_OK;
}

int ImageHelper::fixed_square_thumbnails(std::string & out
        , size_t size
        , ImageType dst_image_type)
{
    if((dst_image_type<=T_MIN) || (dst_image_type>=T_MAX))
    {
        return E_INVALID_TYPE;
    }

    try
    {
        Magick::Blob r_blob = Magick::Blob(_buffer, _blen);
        Magick::Image image;
        image.read(r_blob);

        //count center part domain
        size_t chop_x, chop_y = 0;
        size_t crop_x, crop_y = 0;

        if(_image_width >= _image_heigth)
        {
            // heigth is smaller
            chop_x = (_image_width - _image_heigth)/2;
            chop_y = 0;
            crop_x = _image_heigth;
            crop_y = _image_heigth;
        }
        else
        {
            chop_x = 0;
            chop_y = (_image_heigth - _image_width)/2;
            crop_x = _image_width;
            crop_y = _image_width;
        }

        image.chop(Magick::Geometry(chop_x, chop_y));
        image.crop(Magick::Geometry(crop_x, crop_y));

        std::ostringstream oss;
        oss.str("");
        oss<<size<<"X"<<size;
        image.zoom(oss.str());
        image.magick(type_string(dst_image_type));

        Magick::Blob w_blob;
        image.write(&w_blob);
        out.clear();
        out.assign((const char*)w_blob.data(), w_blob.length());
    }
    catch(std::exception& e)
    {
        return E_INVALID_CONTENT;
    }
    catch(...)
    {
        return E_INVALID_CONTENT;
    }

    return E_OK;
}

int ImageHelper::constrain_thumbnails(
                                      size_t dst_width
                                      , size_t dst_heigth
                                      , ImageType dst_image_type
                                      , std::string& out
                                      , size_t & real_width
                                      , size_t & real_heigth)
{
    if((dst_image_type<=T_MIN) || (dst_image_type>=T_MAX))
    {
        return E_INVALID_TYPE;
    }

    try
    {
        Magick::Blob r_blob = Magick::Blob(_buffer, _blen);
        Magick::Image image;
        image.read(r_blob);

        std::ostringstream oss;
        oss.str("");
        oss<<dst_width<<"X"<<dst_heigth;

        image.zoom(oss.str());
        image.magick(type_string(dst_image_type));

        Magick::Blob w_blob;
        image.write(&w_blob);
        out.clear();
        out.assign((const char*)w_blob.data(), w_blob.length());
        real_width = image.columns();
        real_heigth = image.rows();
    }
    catch(std::exception& e)
    {
        return E_INVALID_CONTENT;
    }
    catch(...)
    {
        return E_INVALID_CONTENT;
    }

    return E_OK;
}

int ImageHelper::crop(size_t x_offset 
				, size_t y_offset
				, size_t witdth
				, size_t height
				, ImageType dst_image_type
				, std::string& out)
{
	if((dst_image_type<=T_MIN) || (dst_image_type>=T_MAX))
    {
        return E_INVALID_TYPE;
    }

    try
    {
        Magick::Blob r_blob = Magick::Blob(_buffer, _blen);
        Magick::Image image;
        image.read(r_blob);

		Magick::Geometry g(witdth, height, x_offset, y_offset);
        image.crop(g);
        image.magick(type_string(dst_image_type));

        Magick::Blob w_blob;
        image.write(&w_blob);
        out.clear();
        out.assign((const char*)w_blob.data(), w_blob.length());
    }
    catch(std::exception& e)
    {
        return E_INVALID_CONTENT;
    }
    catch(...)
    {
        return E_INVALID_CONTENT;
    }

    return E_OK;
}



