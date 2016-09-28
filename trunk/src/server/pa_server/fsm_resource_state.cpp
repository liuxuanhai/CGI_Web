#include "fsm_resource_state.h"
#include "global_var.h"

    /*******************************************************************************************************/
    /*					                       FsmResource						       					   */
    /*******************************************************************************************************/
    IMPL_LOGGER(FsmResourceState, logger);

#define IMPL_FSM_STATE(classname, name) classname FsmResourceState::name(#classname)
    IMPL_FSM_STATE(FsmResourceStateInit, state_init);
    IMPL_FSM_STATE(FsmResourceStateWaitThreadDealResource, state_wait_thread_deal_resource);
    IMPL_FSM_STATE(FsmResourceStateEnd, state_end);
#undef IMPL_FSM_STATE

    void FsmResourceState::enter(FsmResource& fsm)
    {

    }

    void FsmResourceState::exit(FsmResource& fsm)
    {

    }

    void FsmResourceState::download_req_event(FsmResource & fsm,  lce::net::ConnectionInfo& conn, const hoosho::msg::Msg& stMsg)
    {
        LOG4CPLUS_DEBUG(logger, "default download_req_event, state: "<<this->name());
    }

	void FsmResourceState::download_reply_event(FsmResource& fsm, const ExecutorThreadResponseElement& element)
    {
		LOG4CPLUS_DEBUG(logger, "default download_reply_event, state: "<<this->name());
    }

    void FsmResourceState::timeout_event(FsmResource& fsm, void* param)
    {
		LOG4CPLUS_DEBUG(logger, "default timeout_event, state: "<<this->name());
    }

    /*******************************************************************************************************/
    /*                                                                                  FsmResourceStateInit                                                                                         */
    /*******************************************************************************************************/
    void FsmResourceStateInit::download_req_event(FsmResource& fsm, lce::net::ConnectionInfo& conn, const hoosho::msg::Msg& stMsg)
    {
        //save async req param to fsm
        fsm._conn_id = conn.get_id();
        fsm._req_msg.CopyFrom(stMsg);

		//if not exsts in cache, query db
        ExecutorThreadRequestElement req(ExecutorThreadRequestType::T_RESOURCE_DOWNLOAD, FsmContainer<int>::FSM_TYPE_RESOURCE, fsm._id);
		req.need_reply();
		req.m_pa_access_token = g_client_processor->get_pa_access_token();
		req.m_media_id = fsm._req_msg.download_pic_resource_req().media_id();
        g_executor_thread_processor->send_request(req);
        fsm.set_state(FsmResourceState::state_wait_thread_deal_resource);
        return;		
    }


    /*******************************************************************************************************/
    /*                                                                           FsmResourceStateWaitThreadDealResource                                                                         */
    /*******************************************************************************************************/
    void FsmResourceStateWaitThreadDealResource::enter(FsmResource& fsm)
    {
        fsm.reset_timer(g_server->config().get_int_param("FSM_CONTAINER", "timeout"));
    }

    void FsmResourceStateWaitThreadDealResource::exit(FsmResource& fsm)
    {
        fsm.cancel_timer();
    }

    void FsmResourceStateWaitThreadDealResource::download_reply_event(FsmResource& fsm, const ExecutorThreadResponseElement& reply)
    {
        //cancel timer first
        fsm.cancel_timer();

		//resp msg
		::hoosho::msg::Msg stRespMsg;
        ::hoosho::msg::MsgHead* header = stRespMsg.mutable_head();
        header->set_cmd(hoosho::msg::DOWNLOAD_PIC_RESOURCE_RES);
        header->set_seq(fsm._req_msg.head().seq());
        header->set_result(::hoosho::msg::E_OK);

		if(reply.m_result_code != ExecutorThreadRequestType::E_OK)
		{
			header->set_result(::hoosho::msg::E_SERVER_INNER_ERROR);
			g_client_processor->send_datagram(fsm._conn_id, stRespMsg);
			fsm.set_state(FsmResourceState::state_end);
			return;
		}

		::hoosho::pa::DownLoadPicResourceRes* pDownLoadPicResourceRes = stRespMsg.mutable_download_pic_resource_res();
		pDownLoadPicResourceRes->set_media_data(reply.m_resource_data);
		g_client_processor->send_datagram(fsm._conn_id, stRespMsg);
		fsm.set_state(FsmResourceState::state_end);
		
		return;
    }

    void FsmResourceStateWaitThreadDealResource::timeout_event(FsmResource& fsm, void* param)
    {
        LOG4CPLUS_ERROR(logger, "FsmResourceStateWaitThreadDealResource time_out."
                        <<" fsmid:"<<fsm._id
                        <<" connid:"<<fsm._conn_id);
		fsm.reply_timeout();
        fsm.set_state(FsmResourceState::state_end);
        return;
    }


    /*******************************************************************************************************/
    /*                                                                                     FsmResourceEnd                                                                                     */
    /*******************************************************************************************************/
    void FsmResourceStateEnd::enter(FsmResource& fsm)
    {
        g_fsm_resource_container->del_fsm(&fsm);
    }

