#ifndef _QR_CODE_H_
#define _QR_CODE_H_

#include <string>

using namespace std;

class QRPng
{
	public:
		static int url_2_png_easy(const std::string& strUrl, string& strPng, string& strErrMsg);
		static int url_2_png(const std::string& strUrl, string& strPng, string& strErrMsg, int version, int margin, int size, int dpi);
};


#endif
