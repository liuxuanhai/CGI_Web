#include "sys_notice_fsm_state.h"
#include "global_var.h"
#include "dao.h"
/*******************************************************************************************************/
/*					                       SysNoticeFsmState						       					   */
/*******************************************************************************************************/
IMPL_LOGGER(SysNoticeFsmState, logger);

#define IMPL_FSM_STATE(classname, name) classname SysNoticeFsmState::name(#classname)
IMPL_FSM_STATE(SysNoticeFsmStateInit, state_init);
IMPL_FSM_STATE(SysNoticeFsmStateWaitQueryDB, state_wait_query_db);
IMPL_FSM_STATE(SysNoticeFsmStateWaitUpdateDB, state_wait_update_db);
IMPL_FSM_STATE(SysNoticeFsmStateEnd, state_end);
#undef IMPL_FSM_STATE

void SysNoticeFsmState::enter(SysNoticeFsm& fsm)
{

}

void SysNoticeFsmState::exit(SysNoticeFsm& fsm)
{

}

void SysNoticeFsmState::client_req_event(SysNoticeFsm & fsm, lce::net::ConnectionInfo& conn, const hoosho::msg::Msg& stMsg)
{
	LOG4CPLUS_DEBUG(logger, "default  client_req_event, state: "<<this->name());
}

void SysNoticeFsmState::db_reply_event(SysNoticeFsm& fsm, const ExecutorThreadResponseElement& element)
{
	LOG4CPLUS_DEBUG(logger, "default  db_reply_event, state: "<<this->name());
}

void SysNoticeFsmState::timeout_event(SysNoticeFsm& fsm, void* param)
{
	LOG4CPLUS_DEBUG(logger, "default  timeout_event, state: "<<this->name());
}

/*******************************************************************************************************/
/*                                         SysNoticeFsm                                         */
/*******************************************************************************************************/
void SysNoticeFsmStateInit::client_req_event(SysNoticeFsm& fsm, lce::net::ConnectionInfo& conn, const hoosho::msg::Msg& stMsg)
{
	//save async req param to fsm
	fsm._conn_id = conn.get_id();
	fsm._req_type = stMsg.head().cmd();
	fsm._req_seq_id = stMsg.head().seq();

	//dispatch
	uint32_t dwReqType = stMsg.head().cmd();
// ---------------------------------sys_notice--------------------------------------//
	if (hoosho::msg::QUERY_SYS_NOTICE_ALL_REQ == dwReqType)
	{
		const hoosho::sysnotice::QuerySysNoticeAllReq& stQueryReq = stMsg.sys_notice_query_all_req();

		//if not exsts in cache, query db
		ExecutorThreadRequestElement req(ExecutorThreadRequestType::T_SYS_NOTICE_ALL_QUERY);
		req.m_fsm_id = fsm._id;
		req.m_sys_notice.m_pa_appid_md5 = stQueryReq.pa_appid_md5();
		req.m_sys_notice.m_create_ts = stQueryReq.limit_ts();
		req.m_pagesize = stQueryReq.pagesize();
		req.need_reply();
		g_executor_thread_processor->send_request(req);
		fsm.set_state(SysNoticeFsmState::state_wait_query_db);
		return;
	}
	if (hoosho::msg::QUERY_SYS_NOTICE_NEW_REQ == dwReqType)
	{
		const hoosho::sysnotice::QuerySysNoticeNewReq& stQueryReq = stMsg.sys_notice_query_new_req();
		ExecutorThreadRequestElement req(ExecutorThreadRequestType::T_SYS_NOTICE_NEW_QUERY);
		req.m_fsm_id = fsm._id;
		req.m_sys_notice.m_pa_appid_md5 = stQueryReq.pa_appid_md5();
		req.m_pagesize = stQueryReq.pagesize();
		req.m_openid_md5 = stQueryReq.openid_md5();
		req.m_sys_notice.m_create_ts = stQueryReq.limit_ts();
		req.need_reply();
		g_executor_thread_processor->send_request(req);
		fsm.set_state(SysNoticeFsmState::state_wait_query_db);
		return;
	}

	if (hoosho::msg::UPDATE_SYS_NOTICE_REQ == dwReqType)
	{
		ExecutorThreadRequestElement req(ExecutorThreadRequestType::T_SYS_NOTICE_SELECT_UPDATE);
		req.m_fsm_id = fsm._id;
		req.need_reply();

		const hoosho::sysnotice::UpdateSysNoticeReq& stUpdateReq = stMsg.sys_notice_update_req();

		fsm._sys_notice.m_pa_appid_md5 = stUpdateReq.sys_notice().pa_appid_md5();
		fsm._sys_notice.m_create_ts = stUpdateReq.sys_notice().create_ts();
		fsm._sys_notice.m_title = stUpdateReq.sys_notice().title();
		fsm._sys_notice.m_content = stUpdateReq.sys_notice().content();

		req.m_sys_notice = fsm._sys_notice;

		g_executor_thread_processor->send_request(req);
		fsm.set_state(SysNoticeFsmState::state_wait_update_db);
		return;
	}
	if (hoosho::msg::ADD_SYS_NOTICE_REQ == dwReqType)
	{
		ExecutorThreadRequestElement req(ExecutorThreadRequestType::T_SYS_NOTICE_ADD);
		req.m_fsm_id = fsm._id;
		req.need_reply();

		const hoosho::sysnotice::AddSysNoticeReq& stUpdateReq = stMsg.sys_notice_add_req();

		fsm._sys_notice.m_pa_appid_md5 = stUpdateReq.sys_notice().pa_appid_md5();
		fsm._sys_notice.m_create_ts = time(0);
		fsm._sys_notice.m_title = stUpdateReq.sys_notice().title();
		fsm._sys_notice.m_content = stUpdateReq.sys_notice().content();

		req.m_sys_notice = fsm._sys_notice;

		g_executor_thread_processor->send_request(req);
		fsm.set_state(SysNoticeFsmState::state_wait_update_db);
		return;
	}
	if (hoosho::msg::DELETE_SYS_NOTICE_REQ == dwReqType)
	{
		const hoosho::sysnotice::DeleteSysNoticeReq& stDeleteReq = stMsg.sys_notice_del_req();
		ExecutorThreadRequestElement req(ExecutorThreadRequestType::T_SYS_NOTICE_DELETE);
		req.m_fsm_id = fsm._id;
		req.need_reply();

		req.m_sys_notice.m_create_ts = stDeleteReq.create_ts();
		req.m_sys_notice.m_pa_appid_md5 = stDeleteReq.pa_appid_md5();
		g_executor_thread_processor->send_request(req);
		fsm.set_state(SysNoticeFsmState::state_wait_update_db);
		LOG4CPLUS_DEBUG(logger, "db reply: DELETE_SYS_NOTICE_REQ: "<<req.m_sys_notice.ToString());
		return;
	}
//----------------------------------------notice_notify----------------------------------------------//
	if (hoosho::msg::QUERY_NOTICE_NOTIFY_SYS_NOTICE_REQ == dwReqType)
	{
		const hoosho::sysnotice::QueryNoticeNotifySysNoticeReq& stQueryReq = stMsg.notice_notify_sys_notice_query_req();

		//if not exsts in cache, query db
		ExecutorThreadRequestElement req(ExecutorThreadRequestType::T_NOTICE_NOTIFY_SYS_NOTICE_QUERY);
		req.m_fsm_id = fsm._id;
		req.m_openid_md5 = stQueryReq.openid_md5();
		req.m_sys_notice.m_pa_appid_md5 = stQueryReq.pa_appid_md5();

		req.need_reply();
		g_executor_thread_processor->send_request(req);
		fsm.set_state(SysNoticeFsmState::state_wait_query_db);
		return;
	}

	if (hoosho::msg::QUERY_NOTICE_NOTIFY_REQ == dwReqType)
	{
		const hoosho::noticenotify::QueryNoticeNotifyReq& stQueryReq = stMsg.notice_notify_query_req();

		//if not exsts in cache, query db
		ExecutorThreadRequestElement req(ExecutorThreadRequestType::T_NOTICE_NOTIFY_QUERY);
		req.m_fsm_id = fsm._id;
		req.m_notice_user.m_openid_md5 = stQueryReq.openid_md5();
		req.m_notice_user.m_pa_appid_md5 = stQueryReq.pa_appid_md5();
		req.m_sys_notice.m_pa_appid_md5 = stQueryReq.pa_appid_md5();
		req.m_openid_md5 = stQueryReq.openid_md5();
		req.need_reply();
		g_executor_thread_processor->send_request(req);
		fsm.set_state(SysNoticeFsmState::state_wait_query_db);
		return;
	}

	if (hoosho::msg::QUERY_NOTICE_NOTIFY_TYPELIST_REQ == dwReqType)
	{
		const hoosho::noticenotify::QueryNoticeNotifyTypeListReq& stQueryReq = stMsg.notice_notify_typelist_query_req();

		//if not exsts in cache, query db
		ExecutorThreadRequestElement req(ExecutorThreadRequestType::T_NOTICE_NOTIFY_TYPE_LIST_QUERY);
		req.m_fsm_id = fsm._id;
		req.m_notice_user.m_openid_md5 = stQueryReq.openid_md5();
		req.m_notice_user.m_pa_appid_md5 = stQueryReq.pa_appid_md5();
		req.m_notice_user.m_type = stQueryReq.type();

		req.need_reply();
		g_executor_thread_processor->send_request(req);
		fsm.set_state(SysNoticeFsmState::state_wait_query_db);
		return;
	}
//-------------------------------------------notice_record------------------------------------------//
	if (hoosho::msg::QUERY_NOTICE_RECORD_REQ == dwReqType)
	{
		const hoosho::noticenotify::QueryNoticeRecordReq& stQueryReq = stMsg.notice_record_query_req();

		//if not exsts in cache, query db
		ExecutorThreadRequestElement req(ExecutorThreadRequestType::T_NOTICE_RECORD_QUERY);
		req.m_fsm_id = fsm._id;
		req.m_notice_user.m_openid_md5 = stQueryReq.openid_md5();
		req.m_notice_user.m_pa_appid_md5 = stQueryReq.pa_appid_md5();
		req.m_notice_user.m_type = stQueryReq.type();
		req.m_pagesize = stQueryReq.pagesize();
		req.m_notice_user.m_create_ts = stQueryReq.limit_ts();
		req.need_reply();
		g_executor_thread_processor->send_request(req);
		fsm.set_state(SysNoticeFsmState::state_wait_query_db);
		return;
	}
	if (hoosho::msg::QUERY_NOTICE_RECORD_REQ == dwReqType)
	{
		const hoosho::noticenotify::QueryNoticeRecordReq& stQueryReq = stMsg.notice_record_query_req();
		if (stQueryReq.type() > 3 || stQueryReq.type() < 1)
		{
			return;
		}
		//if not exsts in cache, query db
		ExecutorThreadRequestElement req(ExecutorThreadRequestType::T_NOTICE_RECORD_QUERY);
		req.m_fsm_id = fsm._id;
		req.m_notice_user.m_openid_md5 = stQueryReq.openid_md5();
		req.m_notice_user.m_pa_appid_md5 = stQueryReq.pa_appid_md5();
		req.m_notice_user.m_type = stQueryReq.type();
		req.m_pagesize = stQueryReq.pagesize();
		req.m_notice_user.m_create_ts = stQueryReq.limit_ts();
		req.need_reply();
		g_executor_thread_processor->send_request(req);
		fsm.set_state(SysNoticeFsmState::state_wait_query_db);
		return;
	}

	if (hoosho::msg::UPDATE_NOTICE_RECORD_REQ == dwReqType)
	{
		const hoosho::noticenotify::UpdateNoticeRecordReq& stQueryReq = stMsg.notice_record_update_req();

		if (stQueryReq.notice_record().type() > 3 || stQueryReq.notice_record().type() < 1)
		{
			return;
		}
		//if not exsts in cache, query db
		ExecutorThreadRequestElement req(ExecutorThreadRequestType::T_NOTICE_RECORD_SELECT_UPDATE);
		req.m_fsm_id = fsm._id;
		req.m_notice_user.m_openid_md5 = stQueryReq.notice_record().openid_md5();
		req.m_notice_user.m_pa_appid_md5 = stQueryReq.notice_record().pa_appid_md5();
		req.m_notice_user.m_type = stQueryReq.notice_record().type();
		req.m_notice_user.m_status = stQueryReq.notice_record().status();
		req.m_notice_user.m_create_ts = stQueryReq.notice_record().create_ts();
		req.m_notice_user.m_extra_data_0 = stQueryReq.notice_record().extra_data_0();
		req.m_notice_user.m_extra_data_1 = stQueryReq.notice_record().extra_data_1();
		req.m_notice_user.m_extra_data_2 = stQueryReq.notice_record().extra_data_2();

		req.need_reply();
		g_executor_thread_processor->send_request(req);
		fsm.set_state(SysNoticeFsmState::state_wait_update_db);
		return;
	}
	if (hoosho::msg::ADD_NOTICE_RECORD_REQ == dwReqType)
	{
		const hoosho::noticenotify::AddNoticeRecordReq& stQueryReq = stMsg.notice_record_add_req();

		if (stQueryReq.notice_record().type() > 3 || stQueryReq.notice_record().type() < 1)
		{
			return;
		}

		//if not exsts in cache, query db
		ExecutorThreadRequestElement req(ExecutorThreadRequestType::T_NOTICE_RECORD_ADD);
		req.m_fsm_id = fsm._id;
		req.m_notice_user.m_openid_md5 = stQueryReq.notice_record().openid_md5();
		req.m_notice_user.m_pa_appid_md5 = stQueryReq.notice_record().pa_appid_md5();
		req.m_notice_user.m_type = stQueryReq.notice_record().type();
		req.m_notice_user.m_status = 0;
		timeval tv;
		gettimeofday(&tv, NULL);
		uint64_t qwUS = (uint64_t)tv.tv_sec * 1000000;
		qwUS += ((uint64_t)tv.tv_usec);

	//	req.m_notice_user.m_create_ts = time(0);
		req.m_notice_user.m_create_ts = qwUS;
		req.m_notice_user.m_extra_data_0 = stQueryReq.notice_record().extra_data_0();
		req.m_notice_user.m_extra_data_1 = stQueryReq.notice_record().extra_data_1();
		req.m_notice_user.m_extra_data_2 = stQueryReq.notice_record().extra_data_2();

		req.need_reply();
		g_executor_thread_processor->send_request(req);
		fsm.set_state(SysNoticeFsmState::state_wait_update_db);
		return;
	}

	if (hoosho::msg::READ_NOTICE_RECORD_REQ == dwReqType)
	{

		const hoosho::noticenotify::ReadNoticeRecordReq& stQueryReq = stMsg.notice_record_read_req();
		if (stQueryReq.notice_record().type() > 3 || stQueryReq.notice_record().type() < 1)
		{
			return;
		}
		//if not exsts in cache, query db
		ExecutorThreadRequestElement req(ExecutorThreadRequestType::T_NOTICE_RECORD_SELECT_UPDATE);
		req.m_fsm_id = fsm._id;
		req.m_notice_user.m_openid_md5 = stQueryReq.notice_record().openid_md5();
		req.m_notice_user.m_pa_appid_md5 = stQueryReq.notice_record().pa_appid_md5();
		req.m_notice_user.m_type = stQueryReq.notice_record().type();
		req.m_notice_user.m_create_ts = stQueryReq.notice_record().create_ts();
		req.m_notice_user.m_status = 0;

		req.need_reply();
		g_executor_thread_processor->send_request(req);
		fsm.set_state(SysNoticeFsmState::state_wait_update_db);
		return;
	}

	if (hoosho::msg::DELETE_NOTICE_RECORD_REQ == dwReqType)
	{
		const hoosho::noticenotify::DeleteNoticeRecordReq& stQueryReq = stMsg.notice_record_delete_req();

		if (stQueryReq.type() > 3 || stQueryReq.type() < 1)
		{
			return;
		}
		//if not exsts in cache, query db
		ExecutorThreadRequestElement req(ExecutorThreadRequestType::T_NOTICE_RECORD_DELETE);
		req.m_fsm_id = fsm._id;
		req.m_notice_user.m_openid_md5 = stQueryReq.openid_md5();
		req.m_notice_user.m_pa_appid_md5 = stQueryReq.pa_appid_md5();
		req.m_notice_user.m_create_ts = stQueryReq.create_ts();
		req.m_notice_user.m_type = stQueryReq.type();
		req.need_reply();
		g_executor_thread_processor->send_request(req);
		fsm.set_state(SysNoticeFsmState::state_wait_update_db);
		return;
	}

}

/*******************************************************************************************************/
/*                                         SysNoticeFsmStateWaitQueryDB                               */
/*******************************************************************************************************/
void SysNoticeFsmStateWaitQueryDB::enter(SysNoticeFsm& fsm)
{
	fsm.reset_timer(g_server->config().get_int_param("FSM_CONTAINER", "timeout"));
}

void SysNoticeFsmStateWaitQueryDB::exit(SysNoticeFsm& fsm)
{
	fsm.cancel_timer();
}

void SysNoticeFsmStateWaitQueryDB::db_reply_event(SysNoticeFsm& fsm, const ExecutorThreadResponseElement& reply)
{
	//cancel timer first
	fsm.cancel_timer();

	//resp msg
	hoosho::msg::Msg stRespMsg;
	hoosho::msg::MsgHead* header = stRespMsg.mutable_head();
	header->set_seq(fsm._req_seq_id);
	header->set_result(::hoosho::msg::E_OK);

	uint32_t dwReqType = fsm._req_type;
	uint32_t dwStatus = reply.m_result_code;
//-----------------------------------------------------------------------------------------------------//
//-----------------------------------------------------------------------------------------------------//
	const vector<DaoSysNotice>& daoSysNoticeList = reply.m_sys_notice_list.m_dao_sys_notice_list;
//-----------------------------------------------------------------------------------------------------//
//---------------------------------------------sys_notice------------------------------------------------//
//-----------------------------------------------------------------------------------------------------//
	if (hoosho::msg::QUERY_SYS_NOTICE_ALL_REQ == dwReqType)
	{
		header->set_cmd(hoosho::msg::QUERY_SYS_NOTICE_ALL_RES);
		if (ExecutorThreadRequestType::E_OK == dwStatus)
		{
			//	header->set_result(::hoosho::msg::E_OK);
			//update cache
			hoosho::sysnotice::QuerySysNoticeAllRes* pBody = stRespMsg.mutable_sys_notice_query_all_res();
			int iSize = daoSysNoticeList.size();
			LOG4CPLUS_DEBUG(logger, "sysnoticealllist size="<<iSize);
			for (int i = 0; i < iSize; i++)
			{
				hoosho::commstruct::SysNotice* pBodySysNotice = pBody->add_sys_notice_list();
				pBodySysNotice->set_pa_appid_md5(daoSysNoticeList[i].m_pa_appid_md5);
				pBodySysNotice->set_create_ts(daoSysNoticeList[i].m_create_ts);
				pBodySysNotice->set_title(daoSysNoticeList[i].m_title);
				pBodySysNotice->set_content(daoSysNoticeList[i].m_content);
			}
		}
		else if (ExecutorThreadRequestType::E_NOT_EXIST == dwStatus)
		{
			header->set_result(::hoosho::msg::E_OK);
		}
		else
		{
			header->set_result(::hoosho::msg::E_SERVER_INNER_ERROR);
		}

		g_client_processor->send_datagram(fsm._conn_id, stRespMsg);
		fsm.set_state(SysNoticeFsmState::state_end);
		return;
	}
	if (hoosho::msg::QUERY_SYS_NOTICE_NEW_REQ == dwReqType)
	{
		LOG4CPLUS_DEBUG(logger, "IN reply:QUERY_SYS_NOTICE_NEW_REQ:");
		header->set_cmd(hoosho::msg::QUERY_SYS_NOTICE_NEW_RES);
		if (ExecutorThreadRequestType::E_OK == dwStatus)
		{
			//		header->set_result(::hoosho::msg::E_OK);

			hoosho::sysnotice::QuerySysNoticeNewRes* pBody = stRespMsg.mutable_sys_notice_query_new_res();
			int iSize = daoSysNoticeList.size();
			LOG4CPLUS_DEBUG(logger, "sysnoticenewlist size="<<iSize);
			for (int i = 0; i < iSize; i++)
			{
				hoosho::commstruct::SysNotice* pBodySysNotice = pBody->add_sys_notice_list();
				pBodySysNotice->set_pa_appid_md5(daoSysNoticeList[i].m_pa_appid_md5);
				pBodySysNotice->set_create_ts(daoSysNoticeList[i].m_create_ts);
				pBodySysNotice->set_title(daoSysNoticeList[i].m_title);
				pBodySysNotice->set_content(daoSysNoticeList[i].m_content);
			}
		}
		else if (ExecutorThreadRequestType::E_NOT_EXIST == dwStatus)
		{
			header->set_result(::hoosho::msg::E_OK);
		}
		else
		{
			header->set_result(::hoosho::msg::E_SERVER_INNER_ERROR);
		}

		g_client_processor->send_datagram(fsm._conn_id, stRespMsg);
		fsm.set_state(SysNoticeFsmState::state_end);
		return;
	}
//-----------------------------------------------------------------------------------------------------//
//---------------------------------------------------------------------------------------------------//
	const vector<DaoNoticeUserByAll::TypeStatus>& daoNoticeUserByAllList = reply.m_notice_user_list.m_type_status_list;
//-----------------------------------------------------------------------------------------------------//
//------------------------------------------notice_notify---------------------------------------------//
//-----------------------------------------------------------------------------------------------------//
	//------------------------------------sys_notice-------------------//
	if (hoosho::msg::QUERY_NOTICE_NOTIFY_SYS_NOTICE_REQ == dwReqType)
	{
		header->set_cmd(hoosho::msg::QUERY_NOTICE_NOTIFY_SYS_NOTICE_RES);
		if (ExecutorThreadRequestType::E_OK == dwStatus)
		{
			//repeated
			hoosho::sysnotice::QueryNoticeNotifySysNoticeRes* pBody = stRespMsg.mutable_notice_notify_sys_notice_query_res();
			pBody->set_status(1);
		}
		else if (ExecutorThreadRequestType::E_NOT_EXIST == dwStatus)
		{
			hoosho::sysnotice::QueryNoticeNotifySysNoticeRes* pBody = stRespMsg.mutable_notice_notify_sys_notice_query_res();
			pBody->set_status(0);
		}
		else
		{
			//other query error
			header->set_result(::hoosho::msg::E_SERVER_INNER_ERROR);
		}
		g_client_processor->send_datagram(fsm._conn_id, stRespMsg);
		fsm.set_state(SysNoticeFsmState::state_end);
		return;
	}
	//---------------------------------------notice_user------------------------------------//
	if (hoosho::msg::QUERY_NOTICE_NOTIFY_REQ == dwReqType)
	{
		header->set_cmd(hoosho::msg::QUERY_NOTICE_NOTIFY_RES);
		if (ExecutorThreadRequestType::E_OK == dwStatus)
		{
			//repeated
			hoosho::noticenotify::QueryNoticeNotifyRes* pBody = stRespMsg.mutable_notice_notify_query_res();
			pBody->set_status(1);
		}
		else if (ExecutorThreadRequestType::E_NOT_EXIST == dwStatus)
		{
			hoosho::noticenotify::QueryNoticeNotifyRes* pBody = stRespMsg.mutable_notice_notify_query_res();
			pBody->set_status(0);
		}
		else
		{
			//other query error
			header->set_result(::hoosho::msg::E_SERVER_INNER_ERROR);
		}
		g_client_processor->send_datagram(fsm._conn_id, stRespMsg);
		fsm.set_state(SysNoticeFsmState::state_end);
		return;
	}

	if (hoosho::msg::QUERY_NOTICE_NOTIFY_TYPELIST_REQ == dwReqType)
	{
		header->set_cmd(hoosho::msg::QUERY_NOTICE_NOTIFY_TYPELIST_RES);
		if (ExecutorThreadRequestType::E_OK == dwStatus)
		{
			//repeated
			hoosho::noticenotify::QueryNoticeNotifyTypeListRes* pBody = stRespMsg.mutable_notice_notify_typelist_query_res();
			int iSize = daoNoticeUserByAllList.size();
			LOG4CPLUS_DEBUG(logger, "daoNoticeUserByAllList size="<<iSize);
			for (int i = 0; i < iSize; i++)
			{
				hoosho::noticenotify::TypeStatus* pBodyTypeStatus = pBody->add_type_status_list();
				pBodyTypeStatus->set_type(daoNoticeUserByAllList[i].type);
				pBodyTypeStatus->set_status(daoNoticeUserByAllList[i].status);

			}
		}
		else if (ExecutorThreadRequestType::E_NOT_EXIST == dwStatus)
		{
			hoosho::noticenotify::QueryNoticeNotifyTypeListRes* pBody = stRespMsg.mutable_notice_notify_typelist_query_res();
			for (int i = 1; i < 4; i++)
			{
				hoosho::noticenotify::TypeStatus* pBodyTypeStatus = pBody->add_type_status_list();
				pBodyTypeStatus->set_type(i);
				pBodyTypeStatus->set_status(0);

			}
		}
		else
		{
			//other query error
			header->set_result(::hoosho::msg::E_SERVER_INNER_ERROR);
		}
		g_client_processor->send_datagram(fsm._conn_id, stRespMsg);
		fsm.set_state(SysNoticeFsmState::state_end);
		return;
	}
//-----------------------------------------------------------------------------------------------------//
//------------------------------------------------------------------------------------------------------//
	const vector<DaoNoticeUser>& daoNoticeUserList = reply.m_notice_user_list.m_dao_notice_user_list;
//-----------------------------------------------------------------------------------------------------//
//----------------------------------------------notice_record-----------------------------------------//
//-----------------------------------------------------------------------------------------------------//
	if (hoosho::msg::QUERY_NOTICE_RECORD_REQ == dwReqType)
	{
		header->set_cmd(hoosho::msg::QUERY_NOTICE_RECORD_RES);
		if (ExecutorThreadRequestType::E_OK == dwStatus)
		{
			//	header->set_result(::hoosho::msg::E_OK);
			//update cache
			hoosho::noticenotify::QueryNoticeRecordRes * pBody = stRespMsg.mutable_notice_record_query_res();
			int iSize = daoNoticeUserList.size();
			LOG4CPLUS_DEBUG(logger, "daoNoticeUserList size="<<iSize);
			for (int i = 0; i < iSize; i++)
			{
				hoosho::commstruct::NoticeRecord* pBodyNoticeRecord = pBody->add_notice_record_list();
				pBodyNoticeRecord->set_pa_appid_md5(daoNoticeUserList[i].m_pa_appid_md5);
				pBodyNoticeRecord->set_openid_md5(daoNoticeUserList[i].m_openid_md5);
				pBodyNoticeRecord->set_type(daoNoticeUserList[i].m_type);
				pBodyNoticeRecord->set_status(daoNoticeUserList[i].m_status);
				pBodyNoticeRecord->set_create_ts(daoNoticeUserList[i].m_create_ts);
				pBodyNoticeRecord->set_extra_data_0(daoNoticeUserList[i].m_extra_data_0);
				pBodyNoticeRecord->set_extra_data_1(daoNoticeUserList[i].m_extra_data_1);
				pBodyNoticeRecord->set_extra_data_2(daoNoticeUserList[i].m_extra_data_2);

			}
		}
		else if (ExecutorThreadRequestType::E_NOT_EXIST == dwStatus)
		{
			header->set_result(::hoosho::msg::E_OK);
		}
		else
		{
			header->set_result(::hoosho::msg::E_SERVER_INNER_ERROR);
		}

		g_client_processor->send_datagram(fsm._conn_id, stRespMsg);
		fsm.set_state(SysNoticeFsmState::state_end);
		return;
	}

	LOG4CPLUS_ERROR(logger, "error event type! QueryDB");

}

void SysNoticeFsmStateWaitQueryDB::timeout_event(SysNoticeFsm& fsm, void* param)
{
	LOG4CPLUS_ERROR(logger, "SysNoticeFsmStateWaitQueryDB time_out." <<" fsmid:"<<fsm._id <<" connid:"<<fsm._conn_id);
	fsm.reply_timeout();
	fsm.set_state(SysNoticeFsmState::state_end);
	return;
}

/*******************************************************************************************************/
/*                                         SysNoticeFsmStateWaitUpdateDB                               */
/*******************************************************************************************************/
void SysNoticeFsmStateWaitUpdateDB::enter(SysNoticeFsm & fsm)
{
	fsm.reset_timer(g_server->config().get_int_param("FSM_CONTAINER", "timeout"));
}

void SysNoticeFsmStateWaitUpdateDB::exit(SysNoticeFsm& fsm)
{
	fsm.cancel_timer();
}

void SysNoticeFsmStateWaitUpdateDB::db_reply_event(SysNoticeFsm& fsm, const ExecutorThreadResponseElement& reply)
{
//cancel timer first
	fsm.cancel_timer();

	uint32_t dwReqType = fsm._req_type;
	uint32_t dwStatus = reply.m_result_code;

//resp msg
	hoosho::msg::Msg stRespMsg;
	hoosho::msg::MsgHead* header = stRespMsg.mutable_head();
	header->set_seq(fsm._req_seq_id);
	header->set_result(::hoosho::msg::E_OK);
//---------------------------------------------------------------------------------------------------------------//
//-----------------------------------------------------------sys_notice-----------------------------------------//
//---------------------------------------------------------------------------------------------------------------//
	if (hoosho::msg::UPDATE_SYS_NOTICE_REQ == dwReqType)
	{
		header->set_cmd(hoosho::msg::UPDATE_SYS_NOTICE_RES);
	}
	else if (hoosho::msg::DELETE_SYS_NOTICE_REQ == dwReqType)
	{
		header->set_cmd(hoosho::msg::DELETE_SYS_NOTICE_RES);
	}
	else if (hoosho::msg::ADD_SYS_NOTICE_REQ == dwReqType)
	{
		header->set_cmd(hoosho::msg::ADD_SYS_NOTICE_RES);
	}
//---------------------------------------------------------------------------------------------------------------//
//-------------------------------------------------------------notice_record-----------------------------------//
//---------------------------------------------------------------------------------------------------------------//
	else if (hoosho::msg::UPDATE_NOTICE_RECORD_REQ == dwReqType)
	{
		header->set_cmd(hoosho::msg::UPDATE_NOTICE_RECORD_RES);
	}
	else if (hoosho::msg::ADD_NOTICE_RECORD_REQ == dwReqType)
	{
		header->set_cmd(hoosho::msg::ADD_NOTICE_RECORD_RES);
	}

	else if (hoosho::msg::DELETE_NOTICE_RECORD_REQ == dwReqType)
	{
		header->set_cmd(hoosho::msg::DELETE_NOTICE_RECORD_RES);
	}
	else if (hoosho::msg::READ_NOTICE_RECORD_REQ == dwReqType)
	{
		header->set_cmd(hoosho::msg::READ_NOTICE_RECORD_RES);
	}
	else
	{
		LOG4CPLUS_ERROR(logger, "error event type! UpdateDB");
	}
//---------------------------------------------------------------------------------------------------------------//
//----------------------------------------------------------------------------------------------------------------//
//---------------------------------------------------------------------------------------------------------------//
	if (dwStatus == ExecutorThreadRequestType::E_OK)
	{
		header->set_result(::hoosho::msg::E_OK);
	}
	else if (dwStatus == ExecutorThreadRequestType::E_DB_FAIL)
	{
		header->set_result(::hoosho::msg::E_SERVER_INNER_ERROR);
	}
	else if (dwStatus == ExecutorThreadRequestType::E_DB_ALREADY_EXIST)
	{
		header->set_result(::hoosho::msg::E_ALREADY_EXIST);
	}
	else if (dwStatus == ExecutorThreadRequestType::E_NOT_EXIST)
	{
		header->set_result(::hoosho::msg::E_INVALID_REQ_PARAM);
	}
	else
	{
		header->set_result(::hoosho::msg::E_SERVER_INNER_ERROR);
	}

	g_client_processor->send_datagram(fsm._conn_id, stRespMsg);
	fsm.set_state(SysNoticeFsmState::state_end);
	return;
}

void SysNoticeFsmStateWaitUpdateDB::timeout_event(SysNoticeFsm& fsm, void* param)
{
	LOG4CPLUS_ERROR(logger, "SysNoticeFsmStateWaitUpdateDB time_out." <<" fsmid:"<<fsm._id <<" connid:"<<fsm._conn_id);
	fsm.reply_timeout();
	fsm.set_state(SysNoticeFsmState::state_end);
	return;
}

/*******************************************************************************************************/
/*                                         SysNoticeFsmStateEnd                                         */
/*******************************************************************************************************/
void SysNoticeFsmStateEnd::enter(SysNoticeFsm& fsm)
{
	g_sys_notice_fsm_container->del_fsm(&fsm);
}

