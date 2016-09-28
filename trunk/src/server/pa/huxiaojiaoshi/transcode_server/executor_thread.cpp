#include "executor_thread.h"
#include "global_var.h"
#include "common_util.h"
#include "ffmpeg_api_transcode.h"
#include <fstream>
#include "proto_io_tcp_client.h"
#include "wx_api_download_resource.h"
#include <lce/util/md5.h>

IMPL_LOGGER(ExecutorThread, logger)

ExecutorThread::ExecutorThread()
{
    m_queue = NULL;
}

ExecutorThread::~ExecutorThread()
{
}

int ExecutorThread::init(ExecutorThreadQueue * queue)
{
    m_queue = queue;	
	return 0;
}

void ExecutorThread::run()
{
    LOG4CPLUS_TRACE(logger, "ExecutorThread "<<pthread_self()<<" running ...");

    while(true)
    {
        if(m_queue->request_empty())
        {
            usleep(10);
            continue;
        }

        ExecutorThreadRequestElement request = m_queue->get_request();
        m_queue->pop_request();
		
        LOG4CPLUS_TRACE(logger, "ExecutorThread get request, "<<request.ToString());
        ExecutorThreadResponseElement reply(request.m_request_type, request.m_fsm_id);

        switch(request.m_request_type)
        {	
			case ExecutorThreadRequestType::T_FFMPEG_TRANSCODE_REQ:
				process_ffmpeg_transcode(request, reply);
				break;
			
            default:
                LOG4CPLUS_ERROR(logger, "ExecutorThread get unknown type:"
					           <<request.m_request_type<<"("<<ExecutorThreadRequestType::type_string(request.m_request_type)<<")");
                break;
        }

        if(!request.m_need_reply)//no need reply
        {
			continue;
        }

        while(m_queue->reply_full())        
		{
			usleep(50);
		}
		
        m_queue->push_reply(reply);
    }
}

void ExecutorThread::process_ffmpeg_transcode(ExecutorThreadRequestElement & request,ExecutorThreadResponseElement & reply)
{
	std::string wx_media_id = request.m_wx_media_id;
	
	//1. Get pa_access_token From token_server
	const lce::app::Config& stConfig = g_server->config();
	
	std::string strPaBaseAccessToken = "";
	std::string strJSAPITiket = "";

	int iRet = g_token_server_processor->get_pa_info(stConfig.get_string_param("PA_INFO", "appid"), strPaBaseAccessToken, strJSAPITiket);
	if(iRet < 0)
	{
		LOG4CPLUS_ERROR(logger, "process_ffmpeg_transcode get strPaBaseAccessToken failed!");
		reply.m_result_code = ExecutorThreadRequestType::E_FAIL;
		return;
	}

	//2. Download resource from WX
	::common::wxapi::WXAPIDownloadResource stWXAPIDownloadResource;
	//int WXAPIDownloadResource::DownloadResource(const std::string& strPAAccessToken, const std::string& strMediaId, std::string& strResource)
	std::string strResource="";
	iRet = stWXAPIDownloadResource.DownloadResource(strPaBaseAccessToken, wx_media_id, strResource);
	if(iRet < 0)
	{
		reply.m_result_code = ExecutorThreadRequestType::E_FAIL;
		return;
	}

	//3. write original binary to file	
	std::string local_media_id = "";	
	WXMEDIAID_TO_LOCALID(wx_media_id, local_media_id);
	
	std::string outFileName = stConfig.get_string_param("RESOURCE", "path") + local_media_id.substr(0, 2) + "/" + local_media_id.substr(2, 2) + "/" + local_media_id;	
	
	std::fstream outfile;
	outfile.open(outFileName.c_str(), ios::out|ios::binary);
	if(!outfile.is_open())
	{
		LOG4CPLUS_ERROR(logger, "Out File open error!, outfilename: "<<outFileName);
		reply.m_result_code = ExecutorThreadRequestType::E_FAIL;
		return;
	}

	outfile.write(strResource.c_str(), strResource.size());
	outfile.close();

	std::string absolute_path = "";
	absolute_path = outFileName;

	//int chmod(const char * path, mode_t mode);
	if(chmod(outFileName.c_str(), S_IRUSR|S_IWUSR|S_IXUSR|S_IRGRP|S_IWGRP|S_IXGRP|S_IROTH|S_IWOTH|S_IXOTH) < 0)
	{
		LOG4CPLUS_ERROR(logger, "Chmod File error!, outfilename: "<<outFileName<<", errno:"<<errno);		
	}

	//4. transcode 	
	if(stConfig.get_int_param("SWITCH", "transcode") == 1)
	{
		std::string src_path = outFileName;		
		std::string dst_path = outFileName+".mp3";

		LOG4CPLUS_DEBUG(logger, "\n  src_path:"<<src_path<<"\n  dst_path:"<<dst_path);		

		common::ffmpegapi::FFMPEGAPI_TRANSCODE stTranscode;
		if(stTranscode.TranscodeFile(strResource.c_str(), strResource.size(), dst_path.c_str(), AV_CODEC_ID_MP3) < 0)
		{
			LOG4CPLUS_ERROR(logger, "TranscodeFile failed!");
			reply.m_result_code = ExecutorThreadRequestType::E_FAIL;
			return;
		}		

		if(chmod(dst_path.c_str(), S_IRUSR|S_IWUSR|S_IXUSR|S_IRGRP|S_IWGRP|S_IXGRP|S_IROTH|S_IWOTH|S_IXOTH) < 0)
		{
			LOG4CPLUS_ERROR(logger, "Chmod File error!, outfilename: "<<dst_path<<", errno:"<<errno);		
		}

		absolute_path = dst_path;
	}
	

	//5. reply
	reply.m_local_media_id = absolute_path;
	return;
}





