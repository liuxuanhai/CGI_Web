#include "cgi_order_base.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "ffmpeg_api_transcode.h"
#include <errno.h>

class CgiOrderMediaGet: public CgiOrderBase
{
public:
    CgiOrderMediaGet():
            CgiOrderBase(0, "config.ini", "logger.properties")
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

	void Return200(const string& strMediaData, uint64_t qwMediaType)
	{
		GetHeader().AddHeader("Cache-Control: max-age=864000");
		if(qwMediaType == ORDER_MEDIA_TYPE_AUDIO)
		{
        	GetHeader().SetContentType("audio/mpeg");
        	//GetHeader().SetContentType("audio/ogg");
		}
		else if(qwMediaType == ORDER_MEDIA_TYPE_PICTURE)
		{
			GetHeader().SetContentType("image/png");
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
        uint64_t qwOrderId = strtoul(((string)GetInput().GetValue("order_id")).c_str(), NULL, 10);
        uint64_t qwMediaType = strtoul(((string)GetInput().GetValue("media_type")).c_str(), NULL, 10);
        if(qwOrderId == 0)
        {
            LOG4CPLUS_ERROR(logger, "order_id = 0, invalid");
            Return404();
            return true;
        }
        if(qwMediaType == 0)
        {
            LOG4CPLUS_ERROR(logger, "media_type = 0, invalid");
            Return404();
            return true;
        }

		
        //下载多媒体
		std::string strDbIp = "";
		std::string strDbUser = "";
		std::string strDbPasswd = "";
		std::string strDbName = "";
		std::string strDbTableNameOrderMediaInfo = "";
		GetConfig().GetValue("DB", "db_ip", strDbIp, "127.0.0.1");
		GetConfig().GetValue("DB", "db_name", strDbName, "");
		GetConfig().GetValue("DB", "db_user", strDbUser, "");
		GetConfig().GetValue("DB", "db_passwd", strDbPasswd, "");
		GetConfig().GetValue("DB", "table_order_media_info_name", strDbTableNameOrderMediaInfo, "");
		
		lce::cgi::CMysql mysql;
		mysql.Init(strDbIp, strDbName, strDbUser, strDbPasswd);
		std::ostringstream oss;
		oss.str("");
		oss << "select media_content from " << strDbTableNameOrderMediaInfo
			<< " where order_id = " << qwOrderId
			<< " and media_type = "<< qwMediaType;
		if(!mysql.Query(oss.str()))
		{
			LOG4CPLUS_ERROR(logger, "mysql query failed, msg = " <<mysql.GetErrMsg());
			Return403();
			return true;
		}

		if(0 == mysql.GetRowCount())
		{
			LOG4CPLUS_DEBUG(logger, "not exists");
			Return404();
			return true;
		}

		mysql.Next();
        const char* pContentAddr = mysql.GetRow(0);
		unsigned long ulContentLen = mysql.GetRowLength(0);

		if(qwMediaType == ORDER_MEDIA_TYPE_AUDIO)
		{
			LOG4CPLUS_DEBUG(logger, "before transcode, amr.len="<<ulContentLen);

			const uint32_t dwLenContentTranscodeBuffer = 4 * 1024 * 1024;
			static char aczContentTranscodeBuffer [dwLenContentTranscodeBuffer];
			::common::ffmpegapi::FFMPEGAPI_TRANSCODE stFFMPEGAPI_TRANSCODE;
			int iLenMp3 = stFFMPEGAPI_TRANSCODE.TranscodeFile(pContentAddr
											, ulContentLen
											, aczContentTranscodeBuffer
											, dwLenContentTranscodeBuffer
											, AV_CODEC_ID_MP3);
			if(iLenMp3 < 0)
			{
				LOG4CPLUS_ERROR(logger, "TranscodeFile failed");
				Return404();
				return true;
			}

			LOG4CPLUS_DEBUG(logger, "after transcode, mp3.len="<<iLenMp3);


			string strMediaContent = "";
			strMediaContent.assign(aczContentTranscodeBuffer, iLenMp3);

			LOG4CPLUS_DEBUG(logger, "qwOrderId="<<qwOrderId<<", qwMediaType="<<qwMediaType<<", strMediaContent.size="<<strMediaContent.size());

        	Return200(strMediaContent, qwMediaType);
        	return true;
		}
		else if(qwMediaType == ORDER_MEDIA_TYPE_PICTURE)
		{
			LOG4CPLUS_DEBUG(logger, "pic.len="<<ulContentLen);
			string strMediaContent = "";
			strMediaContent.assign(pContentAddr, ulContentLen);
			Return200(strMediaContent, qwMediaType);
			return true;
		}

		LOG4CPLUS_DEBUG(logger, "invalid media_type = " << qwMediaType);
		Return404();
		return true;
    }

};

int main()
{
    CgiOrderMediaGet cgi;
    if(!cgi.Run())
    {
        return -1;
    }
    return 0;
}

