#include "cgi_feeds_base.h"
//#include <fstream>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>

class CgiListenFolow: public CgiFeedsBase
{
public:
    CgiListenFolow():
            CgiFeedsBase(0, "config.ini", "logger.properties", true)
    {

    }

    void Return404()
    {
    	lce::cgi::CHttpHeader& stHttpRspHeader = GetHeader();
    	stHttpRspHeader.AddHeader("Status: 404");
    	stHttpRspHeader.Output();
    }

    void Return403()
    {
    	//no previledges
    	lce::cgi::CHttpHeader& stHttpRspHeader = GetHeader();
    	stHttpRspHeader.AddHeader("Status: 403");
    	stHttpRspHeader.Output();
    }

	void Return200(const string& strMediaData, const uint64_t qwMediaType)
	{
		//GetHeader().AddHeader("Cache-Control: max-age=0, must-revalidate");
		GetHeader().AddHeader("Cache-Control: max-age=864000");
        if(qwMediaType == ::hoosho::j::commenum::MEDIA_TYPE_VOICE)
        {
            GetHeader().SetContentType("audio/mpeg");
        }
        else
        {
            Return404();
            return;
        }

		GetHeader().Output();
		if(!std::cout.good())
		{
			std::cout.clear();
			std::cout.flush();
		}

		std::cout<<strMediaData;

		return;
	}

    bool InnerProcess()
    {
        //get HTTP params
        string strOpenid = m_cookie_value_key;
        uint64_t qwFollowId = strtoul( ((string)GetInput().GetValue("follow_id")).c_str(), NULL, 10);

        if(qwFollowId == 0)
        {
            LOG4CPLUS_ERROR(logger, "follow_id = 0, invalid");
            Return404();
            return true;
        }

        //拉取listen详情
        //1.判断用户是否付费过
        //2.获得多媒体id

        uint64_t qwErrcode = 0;
        std::string strErrMsg = "";

        ::hoosho::j::commstruct::ListenInfo stListenInfo;
        ::hoosho::j::commstruct::FollowInfo stFollowInfo;

        if(FetchListenDetail(m_feeds_server_ip, m_feeds_server_port, qwFollowId, strOpenid,
                         qwErrcode, strErrMsg, stListenInfo, stFollowInfo) < 0)
        {
            LOG4CPLUS_ERROR(logger, "proto io failed, errmsg = " << strErrMsg);
            Return404();
            return true;
        }
        if(qwErrcode != ::hoosho::msg::E_OK)
        {
            LOG4CPLUS_ERROR(logger, "server return " << qwErrcode);
            Return403();
            return true;
        }
        LOG4CPLUS_DEBUG(logger, stListenInfo.Utf8DebugString());
        LOG4CPLUS_DEBUG(logger, stFollowInfo.Utf8DebugString());


        //下载多媒体
        string strMediaId = stFollowInfo.content_id();
        uint64_t qwMediaType = stFollowInfo.content_type();

        LOG4CPLUS_DEBUG(logger, "media_id = " << strMediaId);
//        string strMediaId = "ffffabcdefghijkl";
//        uint64_t qwMediaType = hoosho::j::commenum::MEDIA_TYPE_VOICE;

        string strMediaData = "";
        string strPath = strMediaId;

        /*
        const int BUF_LEN = 2048;
        char szBuf[BUF_LEN];
        ifstream fin(strPath, ios::binary);
        while(fin.read(szBuf, sizeof(char) * (BUF_LEN - 1)))
        {
            strMediaData.append(szBuf, fin.gcount());
        }
         */

        struct stat st;
        int iRet = stat(strPath.c_str(), &st);
        if(iRet < 0)
        {
            LOG4CPLUS_ERROR(logger, "file not found, strPath="<<strPath);
            Return404();
            return true;
        }

        iRet = ::open(strPath.c_str(), O_RDONLY);
        if(iRet < 0)
        {
            LOG4CPLUS_ERROR(logger, "iRet = " << iRet << ", errno = " << errno);
            LOG4CPLUS_ERROR(logger, "open file error, strPath="<<strPath);
            Return404();
            return true;
        }

        const int BUF_LEN = 2048;
        char szBuf[BUF_LEN];
        int iNReadNum = -1;
        while((iNReadNum  = read(iRet, szBuf, BUF_LEN - 1)) > 0)
        {
            strMediaData.append(szBuf, iNReadNum);
        }

        LOG4CPLUS_DEBUG(logger, "media_size = " << strMediaData.size());

        Return200(strMediaData, qwMediaType);
        return true;
    }

};

int main()
{
    CgiListenFolow cgi;
    if(!cgi.Run())
    {
        return -1;
    }
    return 0;
}

