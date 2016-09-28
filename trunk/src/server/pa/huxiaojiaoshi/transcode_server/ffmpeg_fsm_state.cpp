#include "ffmpeg_fsm_state.h"
#include "global_var.h"

    /*******************************************************************************************************/
    /*					                       FfmpegState						       					   */
    /*******************************************************************************************************/
    IMPL_LOGGER(FfmpegState, logger);

#define IMPL_FSM_STATE(classname, name) classname FfmpegState::name(#classname)
    IMPL_FSM_STATE(FfmpegFsmStateInit, state_init);
    IMPL_FSM_STATE(FfmpegStateWaitQueryDB, state_wait_query_db);
    IMPL_FSM_STATE(FfmpegStateEnd, state_end);
#undef IMPL_FSM_STATE

    void FfmpegState::enter(FfmpegFsm& fsm)
    {

    }

    void FfmpegState::exit(FfmpegFsm& fsm)
    {

    }

    void FfmpegState::client_req_event(FfmpegFsm & fsm,  lce::net::ConnectionInfo& conn, const hoosho::msg::Msg& stMsg)
    {
        LOG4CPLUS_DEBUG(logger, "default  client_req_event, state: "<<this->name());
    }

	void FfmpegState::db_reply_event(FfmpegFsm& fsm, ExecutorThreadResponseElement& element)
    {
		LOG4CPLUS_DEBUG(logger, "default  db_reply_event, state: "<<this->name());
    }

    void FfmpegState::timeout_event(FfmpegFsm& fsm, void* param)
    {
		LOG4CPLUS_DEBUG(logger, "default  timeout_event, state: "<<this->name());
    }

    /*******************************************************************************************************/
    /*                                         FfmpegFsmStateInit                                         */
    /*******************************************************************************************************/
    void FfmpegFsmStateInit::client_req_event(FfmpegFsm& fsm, lce::net::ConnectionInfo& conn, const hoosho::msg::Msg& stMsg)
    {
        //save async req param to fsm
        fsm._conn_id = conn.get_id();
        fsm._msg.CopyFrom(stMsg);

        if(::hoosho::msg::J_TRANSCODE_REQ == stMsg.head().cmd())
        {
			ExecutorThreadRequestElement req(ExecutorThreadRequestType::T_FFMPEG_TRANSCODE_REQ, fsm._id);
            
            req.need_reply();
            req.m_media_type = stMsg.transcode_req().type();
            req.m_wx_media_id = stMsg.transcode_req().wx_media_id();
            g_executor_thread_processor->send_request(req);          
        }
		
		fsm.set_state(FfmpegState::state_wait_query_db);
		return;		
    }


    /*******************************************************************************************************/
    /*                                         FfmpegStateWaitQueryDB                               */
    /*******************************************************************************************************/
    void FfmpegStateWaitQueryDB::enter(FfmpegFsm& fsm)
    {
        fsm.reset_timer(g_server->config().get_int_param("FSM_CONTAINER", "timeout"));
    }

    void FfmpegStateWaitQueryDB::exit(FfmpegFsm& fsm)
    {
        fsm.cancel_timer();
    }

    void FfmpegStateWaitQueryDB::db_reply_event(FfmpegFsm& fsm, ExecutorThreadResponseElement& reply)
    {
        //cancel timer first
        fsm.cancel_timer();

		if(reply.m_result_code != ExecutorThreadRequestType::E_OK)
		{
			LOG4CPLUS_ERROR(logger, "FfmpegStateWaitQueryDB::db_reply_event failed"
                        <<", fsmid:"<<fsm._id
                        <<", result code="<<reply.m_result_code);
                        
			fsm.reply_fail(::hoosho::msg::E_SERVER_INNER_ERROR);
			return;
		}
		
		//resp msg
		::hoosho::msg::Msg stRespMsg;
        ::hoosho::msg::MsgHead* header = stRespMsg.mutable_head();        
	    header->set_result(::hoosho::msg::E_OK);
        header->set_seq(fsm._msg.head().seq());

		if(::hoosho::msg::J_TRANSCODE_REQ == fsm._msg.head().cmd())
		{
			::hoosho::j::transcode::TranscodeRes* pFfmpegTranscodeRes = stRespMsg.mutable_transcode_res();
			pFfmpegTranscodeRes->set_local_media_id(reply.m_local_media_id);
			
			header->set_cmd(::hoosho::msg::J_TRANSCODE_RES);
		}       

        g_client_processor->send_datagram(fsm._conn_id, stRespMsg);
		fsm.set_state(FfmpegState::state_end);
		return;					
    }

    void FfmpegStateWaitQueryDB::timeout_event(FfmpegFsm& fsm, void* param)
    {
        LOG4CPLUS_ERROR(logger, "FfmpegStateWaitQueryDB time_out."
                        <<" fsmid:"<<fsm._id
                        <<" connid:"<<fsm._conn_id);
		fsm.reply_fail(::hoosho::msg::E_SERVER_TIMEOUT);
        fsm.set_state(FfmpegState::state_end);
        return;
    }


    /*******************************************************************************************************/
    /*                                         FfmpegStateEnd                                         */
    /*******************************************************************************************************/
    void FfmpegStateEnd::enter(FfmpegFsm& fsm)
    {
        g_ffmpeg_fsm_container->del_fsm(&fsm);
    }

