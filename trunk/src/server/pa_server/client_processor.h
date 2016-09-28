#ifndef _HOOSHO_PA_SERVER_CLIENT_MESSAGE_PROCESSOR_H_
#define _HOOSHO_PA_SERVER_CLIENT_MESSAGE_PROCESSOR_H_

#include "util/logger.h"
#include <ext/hash_map>
#include "net/connectioninfo.h"
#include "net/imessageprocessor.h"
#include "msg.pb.h"
#include "app/timer.h"

using namespace lce;

class ClientProcessor : public lce::net::IMessageProcessor, public lce::app::TimerHandler
{
public:
	ClientProcessor();
	~ClientProcessor();

	void accept(lce::net::ConnectionInfo& conn);
	void remove(lce::net::ConnectionInfo& conn);

	void process_input(lce::net::ConnectionInfo& conn, lce::net::IDatagram& ogram);
	void process_pre_auth_code(lce::net::ConnectionInfo& conn, const ::hoosho::msg::Msg& msg);
	void process_jsapi_ticket(lce::net::ConnectionInfo& conn, const ::hoosho::msg::Msg& msg);
	void process_download_resource(lce::net::ConnectionInfo& conn, const ::hoosho::msg::Msg& msg);
	void process_pa_list_get(lce::net::ConnectionInfo& conn, const ::hoosho::msg::Msg& msg);
	void process_pa_tab_get(lce::net::ConnectionInfo& conn, const ::hoosho::msg::Msg& msg);	
	void process_pa_expert_query(lce::net::ConnectionInfo& conn, const ::hoosho::msg::Msg& msg);
	
	int send_datagram(uint32_t conn_id, const hoosho::msg::Msg& stMsg);
	void process_output();

	typedef __gnu_cxx::hash_map<uint32_t, lce::net::ConnectionInfo*> ConnMap;
	typedef __gnu_cxx::hash_map<uint32_t, lce::net::ConnectionInfo*>::iterator ConnMapIterator;

	ConnMap _conn_map;

public:
	std::string get_pa_access_token() {return _hx_pa_accesstoken;}

public:	
	virtual void handle_timeout(void * param);
	bool need_update_pa_token();
	void do_update_pa_token(const std::string& strAccessToekn, const std::string& strTicketJSAPI);

private:
	DECL_LOGGER(logger);

	std::string _hx_pa_accesstoken;
	std::string _hx_ticket_jsapi;
	uint64_t _hx_token_last_update_ts;
};

#endif

