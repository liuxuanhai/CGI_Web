#ifndef _GZIP_DEFLATER_H_
#define  _GZIP_DEFLATER_H_

#include <time.h>
#include <zlib.h>

namespace lce
{
    namespace util
    {
        class GzipDeflater
        {
        public:
            GzipDeflater();
            ~GzipDeflater();    

            int     deflate_gzip(char* in, int in_len, char* out, int& out_len, bool front = true, bool end = true);  //gzip  
            int     inflate_gzip(char* in, int in_len, char* out, int& out_len, bool front = true, bool end = true);  //ungzip

            int     deflate_deflate(char* in, int in_len, char* out, int& out_len, bool end = true);
            int     inflate_inflate(char* in, int in_len, char* out, int& out_len, bool end = true);

            void    reset();
        private:
            int     init_deflate();
            int     init_inflate();
        private:
            bool            _inited_deflater;
            bool            _inited_inflater;
            z_stream        _deflater;
            z_stream        _inflater;
            unsigned long   _crc;
        };
        
    }
}

#endif

