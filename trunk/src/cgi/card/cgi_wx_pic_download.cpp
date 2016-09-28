#include "hoosho_cgi_card.h"
#include "wx_https_req.h"
class CgiWXPicDownload: public HooshoCgiCard
{
     public:
        CgiWXPicDownload() : HooshoCgiCard(0, "config_card.ini", "logger.properties_card",NO_LOGIN)
        {

        }

    	//image/png
        void Return404ForView()
        {
        	lce::cgi::CHttpHeader& stHttpRspHeader = GetHeader();
        	stHttpRspHeader.AddHeader("Status: 404");
        	stHttpRspHeader.Output();
        }
        
		void Return404ForDownload()
        {
        	lce::cgi::CHttpHeader& stHttpRspHeader = GetHeader();
        	stHttpRspHeader.AddHeader("Status: 404");
			GetHeader().SetContentType("text/html; charset=utf-8");
        	stHttpRspHeader.Output();
			std::cout<<"图片不存在";
        }

		void Return200ForView(const string& strPicData)
		{
			GetHeader().AddHeader("Cache-Control: max-age=172800, must-revalidate");
			GetHeader().SetContentType("image/png");

			GetHeader().Output();
			if(!std::cout.good())
			{
				std::cout.clear();
				std::cout.flush();
			}

			std::cout<<strPicData;

			return;
		}


		void Return200ForDisposition(const string& strPicData)
		{
			GetHeader().SetContentType("application/octet-stream");
			GetHeader().AddHeader("Content-Disposition: attachment; filename=qrcode.jpg");
			
			GetHeader().Output();
			if(!std::cout.good())
			{
				std::cout.clear();
				std::cout.flush();
			}

			std::cout<<strPicData;

			return;
		}
		
		void Return200(bool bQRCodeDownload, const string& strPicData)
		{
			if(bQRCodeDownload)
			{
				Return200ForDisposition(strPicData);
			}
			else
			{
				Return200ForView(strPicData);
			}
		}

		void Return404(bool bQRCodeDownload)
		{
			if(bQRCodeDownload)
			{
				Return404ForDownload();
			}
			else
			{
				Return404ForView();
			}
		}


		bool InnerProcess()
		{
			string strUrl = (string)GetInput().GetValue("wx_url");
			bool bQRCodeDownload = (strUrl.find("showqrcode") != std::string::npos);

			if(strUrl.empty())
			{
				Return404(bQRCodeDownload);
				LOG4CPLUS_ERROR(logger, "invalid wx_url ");
				return true;
			}

			int iRet = 0;
			string strPic="";
			string strErrMsg ="";
			iRet = WXHttpsReq::WXPictureDownLoad(strUrl, strPic,strErrMsg);
			if(iRet!=0)
			{
				Return404(bQRCodeDownload);
				LOG4CPLUS_ERROR(logger, "invalid wx_url,Msg="<<strErrMsg);
				return true;
			}

			LOG4CPLUS_DEBUG(logger, "read pic.size="<<strPic.size());

			Return200(bQRCodeDownload, strPic);
			return true;
		}
};


int main()
{
    CgiWXPicDownload cgi;
    if (!cgi.Run())
    {
        return -1;
    }
    return 0;
}

