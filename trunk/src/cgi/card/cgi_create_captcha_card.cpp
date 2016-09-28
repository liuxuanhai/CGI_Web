#include "hoosho_cgi_card.h"
#include "wx_https_req.h"

#include <gd.h>
#include <stddef.h>
#include <paths.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/queue.h>
#include <err.h>
#include <time.h>
#include <ctype.h>
#include <math.h>
#include <stdio.h>


#define MIN(a,b)    ((a)>(b)?(b):(a))

class CgiCreateCaptchaCard: public HooshoCgiCard
{
     public:
		CgiCreateCaptchaCard() : HooshoCgiCard(0, "config_card.ini", "logger.properties_card", NO_LOGIN)
        {

        }


        void Return404()
        {
        	lce::cgi::CHttpHeader& stHttpRspHeader = GetHeader();
        	stHttpRspHeader.AddHeader("Status: 404");
        	stHttpRspHeader.Output();
        }

        void Return200(const string& strPicData)
        {
			GetHeader().AddHeader("Cache-Control: max-age=0, must-revalidate");
			GetHeader().SetContentType("image/jpeg");

			GetHeader().Output();
			if(!std::cout.good())
			{
				std::cout.clear();
				std::cout.flush();
			}

			std::cout<<strPicData;

			return;
        }

	int GenImage( char * out_word, int max_len, string * img_buff, string & strErrMsg )
	{
		int width  = 120;
		int height = 60;
		//char font[256];
		//strncpy(font,m_font_path.c_str(),sizeof(font));
	 	//font[sizeof(font)-1]='\0';
		char *font = new char[m_font_path.length()+1];
	 	strncpy(font,m_font_path.c_str(),m_font_path.length());
	 	font[m_font_path.length()]='\0';


		string symbols   = "23456789abcdegkmnpqsuvxyz";
		int symbols_len = symbols.length();
		double font_size = height * 0.5;

		int fluctuation_amplitude = 8; // symbol's vertical fluctuation amplitude 10

		int img_bg_color[] = {49, 56, 75};
		int img_fg_color[] = {51, 102, 204};

		struct s_fts {
			int   x;
			int   y;
			char  c;
			gdImagePtr i;
		} fts[36];

		// setup font


		srand((unsigned)time(0));

		int i, j, x, y, brect[8];
		char *err, s[2];
		int ft_color, trans;


		gdFTStringExtra strex = {0};
		gdImagePtr im;

		for (i = 0; i < symbols_len; i++) {
			if (!isalpha(symbols[i]) && !isdigit(symbols[i])) {
				strErrMsg = "failed to create response buffer";
				return -1;
			}

			sprintf(s, "%c", symbols[i]);

			err = gdImageStringFT(NULL,&brect[0],0,font,font_size,0.,0,0,s);
			if (err) {
				strErrMsg = "Failed to font " + (string)err;
				return -1;
			}

			x = brect[2] - brect[6];
			y = brect[3] - brect[7];

			fts[i].c  = symbols[i];
			fts[i].x  = x;
			fts[i].y  = y;

			im = gdImageCreateTrueColor(x, y);
			gdImageAlphaBlending(im, 0);
			gdImageSaveAlpha(im, 1);

			gdImageColorAllocate(im, 255, 255, 255);
			ft_color = gdImageColorAllocate(im, 0, 0, 0);
			trans    = gdImageColorAllocateAlpha(im, 255, 255, 255, 1);

			gdImageFilledRectangle(im, 0, 0, 74, 74, trans);

			x = 0 - brect[6];
			y = 0 - brect[7];

			err = gdImageStringFTEx(im, brect, ft_color, font, font_size, 0.0, x, y, s, &strex);
			if (err) {
				strErrMsg = "Failed to font" + (string)err;
				return -1;
			}

			fts[i].i = im;
		}
		
		delete []font;
		


		// make captcha


		gdImagePtr img, imo;

		img = gdImageCreateTrueColor(width, height);
		gdImageAlphaBlending(img, 1);

		int white = gdImageColorAllocate(img, 255, 255, 255);

		gdImageFilledRectangle(img, 0, 0, width - 1, height - 1, white);

		x = 1;
		y = 1;
		int shift = 0, sx, sy;
		int rgb, opacity, left, px, py;
		int odd = rand()%2;
		if (odd == 0) odd =-1;
		float color, color_x, color_y, color_xy;

		int word_len = 4;

		char word[ 256 ] = { 0 };

		for (i = 0; i < word_len; i++) {
			j = rand() % symbols_len;
			y = ((i%2) * fluctuation_amplitude - fluctuation_amplitude/2) * odd
				+ (rand() % ((int)(fluctuation_amplitude*0.33 + 0.5) * 2 + 1) - ((int)(fluctuation_amplitude*0.33 + 0.5)))
				+ (height - font_size)/2;

			shift = 0;

			if (i > 0) {
				shift=10000;

				for (sy = 1; sy < fts[j].y; sy++) {
					for (sx = 1; sx < fts[j].x; sx++) {
						rgb = gdImageTrueColorPixel(fts[j].i, sx, sy);
						opacity = rgb>>24;
						if (opacity < 127) {
							left = sx - 1 + x;
							py = sy + y;
							if (py > height) break;
							for (px = MIN(left, width-1); px > left-200 && px >= 0; px -= 1) {
								color = gdImageTrueColorPixel(img, px, py) & 0xff;
								if (color + opacity<170) { // 170 - threshold
									if (shift > left - px) {
										shift = left - px;
									}
									break;
								}
							}
							break;
						}
					}
				}

				if (shift == 10000){
					shift = rand() % 3 + 4;
				}
			}

			gdImageCopy(img, fts[j].i, x - shift, y, 0, 0, fts[j].x, fts[j].y);

			x += fts[j].x - shift;
			word[i] = fts[j].c;
		}

		int center = x/2;

		// periods
		float rand1 = (rand() % 45 + 75)/1000.0;
		float rand2 = (rand() % 45 + 75)/1000.0;
		float rand3 = (rand() % 45 + 75)/1000.0;
		float rand4 = (rand() % 45 + 75)/1000.0;
		// phases
		float rand5 = (rand() % 314)/100.0;
		float rand6 = (rand() % 314)/100.0;
		float rand7 = (rand() % 314)/100.0;
		float rand8 = (rand() % 314)/100.0;
		// amplitudes
		float rand9 = (rand() %  90 + 330)/110.0;
		float rand10= (rand() % 120 + 330)/100.0;

		float newr, newg, newb;
		float frsx, frsy, frsx1, frsy1;
		float newcolor, newcolor0;

		imo = gdImageCreateTrueColor(width, height);

		gdImageAlphaBlending(imo, 1);
		int bg = gdImageColorAllocate(imo, img_bg_color[0], img_bg_color[1], img_bg_color[2]);
		gdImageFilledRectangle(imo, 0, 0, width - 1, height - 1, bg);


		for (x = 0; x < width; x++) {
			for (y = 0; y < height; y++) {

				sx = x +(sin(x*rand1+rand5)+sin(y*rand3+rand6))*rand9-width/2+center+1;
				sy = y +(sin(x*rand2+rand7)+sin(y*rand4+rand8))*rand10;

				if (sx<0 || sy<0 || sx>=width-1 || sy>=height-1){
					continue;
				} else {
					color   = gdImageTrueColorPixel(img, sx, sy)    & 0xFF;
					color_x = gdImageTrueColorPixel(img, sx+1, sy)  & 0xFF;
					color_y = gdImageTrueColorPixel(img, sx, sy+1)  & 0xFF;
					color_xy= gdImageTrueColorPixel(img, sx+1, sy+1)& 0xFF;
				}

				if (color==255 && color_x==255 && color_y==255 && color_xy==255){
					continue;
				} else if(color==0 && color_x==0 && color_y==0 && color_xy==0){
					newr = img_fg_color[0];
					newg = img_fg_color[1];
					newb = img_fg_color[2];
				} else {
					frsx  = sx - floor(sx);
					frsy  = sy - floor(sy);
					frsx1 = 1 - frsx;
					frsy1 = 1 - frsy;

					newcolor = ( (color * frsx1 * frsy1) + (color_x * frsx * frsy1)
							+ (color_y * frsx1 * frsy) + (color_xy * frsx * frsy) );

					if (newcolor > 255) newcolor = 255;
					newcolor = newcolor/255;
					newcolor0 = 1 - newcolor;

					newr  = newcolor0 * img_fg_color[0] + newcolor * img_bg_color[0];
					newg  = newcolor0 * img_fg_color[1] + newcolor * img_bg_color[1];
					newb  = newcolor0 * img_fg_color[2] + newcolor * img_bg_color[2];
				}

				gdImageSetPixel(imo, x, y, gdImageColorAllocate(imo, newr, newg, newb));
			}
		}


		int SIZE;
		char* DATA = (char*)gdImageJpegPtr(imo, &SIZE, -100);

		img_buff->assign( DATA, sizeof( char ) * SIZE );



		free( DATA );

		strncpy( out_word, word, max_len );

		gdImageDestroy(img);
		gdImageDestroy(imo);	

		return 0;
	}

	int CaptchabBuild(const string key, string * img_buff, string &strErrMsg)
	{
		char word[ 256 ] = { 0 };
		string errmsg;



		if(GenImage( word, sizeof( word ), img_buff, errmsg ) != 0){
			LOG4CPLUS_DEBUG( logger,"word: " << word << ", errmsg =" << errmsg );
			strErrMsg = errmsg;
			return -1;
		}

		
		// save key-word


		if(!CaptchaAdd(key, (std::string)word, strlen(word), strErrMsg))
		{
			strErrMsg = "fail to call CaptchaAdd func, errmsg="+ strErrMsg;
			return -1;
		}


		return 0;

	}

		bool InnerProcess()
		{

		    string strErrMsg = "";
		    string key = (string)GetInput().GetValue("captcha_token");

            if(key.empty())
            {
                DoReply(CGI_RET_CODE_INVALID_PARAM);
                LOG4CPLUS_ERROR(logger, "invalid captcha_token");
                return true;
            }





		    string img;
		    // check whether the code exists
 		    std::string data = "";
 		    int ret = CaptchaExist(key, strErrMsg);
            if(ret == -1)
            {
                DoReply(CGI_RET_CODE_SERVER_BUSY);
 				LOG4CPLUS_ERROR(logger, "fail to call CaptchaExist, errmsg="<<strErrMsg);
 				return true;
            }



            if(ret)
            {
            	// key exists in memcached,   delete in memcached
            	LOG4CPLUS_DEBUG(logger, "key exists in memcached, delete in memcached");
                if(!MemcachedDelete(key, strErrMsg))
                {
                    DoReply(CGI_RET_CODE_INVALID_PARAM);
    				LOG4CPLUS_ERROR(logger, "delete captcha token failed, errmsg="<<strErrMsg);
    				return true;
                }

            }



		    if(CaptchabBuild(key, &img, strErrMsg) != 0)
            {
		    	Return404();
				LOG4CPLUS_ERROR(logger, "fail to call CaptchabBuild func, errmsg="<<strErrMsg);
				return true;
            }


		    LOG4CPLUS_DEBUG( logger, " create captcha done key="<<key<<",size="<<img.size());

            Return200(img);

			return true;


		}
};


int main()
{
	CgiCreateCaptchaCard cgi;
    if (!cgi.Run())
    {
        return -1;
    }
    return 0;
}



// height:30px,width:60px,font_size= height*0.7
// 字35,90,199
// 背景220,230,255
//
