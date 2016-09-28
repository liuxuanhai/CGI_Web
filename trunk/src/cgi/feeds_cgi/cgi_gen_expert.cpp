#ifndef _HOOSHO_CGI_GEN_EXPERT_H_
#define _HOOSHO_CGI_GEN_EXPERT_H_

#include "cgi/cgi.h"
#include "jsoncpp/json.h"
#include "common_util.h"
#include "proto_io_tcp_client.h"
#include "util/lce_util.h"
#include "../cgi_ret_code_def.h"
#include <sstream>
#include <stdio.h>
#include <algorithm>

class CgiGenExpert
{
public:

	 CgiGenExpert()
	 {

	 }

public:
	virtual bool DerivedInit()
	{
		m_db_ip = "127.0.0.1";
		m_db_user = "root";
		m_db_pass = ".hoosho,9942";

		m_db_name_feed = "d_feeds";
		m_table_prefix_feed = "t_feed_";
		m_table_prefix_feed_index = "t_feed_index_on_appid_";
		m_db_name_user = "d_user";
		m_table_prefix_user_follow = "t_user_follow_";


		m_mysql_feed.Init(m_db_ip, m_db_name_feed, m_db_user, m_db_pass);
		m_mysql_user.Init(m_db_ip, m_db_name_user, m_db_user, m_db_pass);
		return true;
	}

	string IntToHexStr(uint32_t num)
	{
	    char index[4];
		sprintf(index, "%02x", num);
		return string(index);
	}

	virtual bool Process()
	{
		//uint64_t tmp[] = {1, 95553253925486975, 222222, 1234455555};
//		uint64_t tmp[] = {1, 222222, 1234455555};
		uint64_t tmp[] = {6353185739184837437};
		cout << ( sizeof(tmp)/sizeof(uint64_t) ) << endl;
		vector<uint64_t> m_appid_list(tmp, tmp + sizeof(tmp)/sizeof(uint64_t));


        const uint64_t time_of_day = 60 * 60 * 24;
        const uint64_t time_count = time_of_day * 7;

        timeval tv;
        gettimeofday(&tv, NULL);
		uint64_t cur_ts = (uint64_t)tv.tv_sec;
		cout << "cur_ts: " << cur_ts << endl;
//		cur_ts = cur_ts / time_of_day * time_of_day;
//        cout << "cur_ts: " << cur_ts << endl;
        const uint64_t begin_ts = cur_ts - time_count;
        cout << "begin_ts: " << begin_ts << endl;
        const uint64_t begin_feed_id = (begin_ts * 1000 ) << 16;

		const uint64_t count_feed_type = 5;
		const uint64_t weight_fan = 40;
		const uint64_t weight_comment = 35;
		const uint64_t weight_favorite = 15;
		const uint64_t weight_feed = 10;
		const uint64_t count_expert = 50;

		for(size_t i = 0; i < m_appid_list.size(); i++)
		{

			uint64_t appid = m_appid_list[i];
			cout << appid << endl;
			vector<uint64_t> m_feed_id_list;
			vector<uint64_t> m_open_id_list;


			ostringstream oss;

			cout << endl;
			cout << "APPID: " << appid << endl;

			oss.str("");
			oss << "select * from " << m_table_prefix_feed_index << IntToHexStr( appid % 256)
				<< " where pa_appid_md5 = " << appid
				<< " and feed_type & " << count_feed_type << " = feed_type"
				<< " and feed_id > " << begin_feed_id;

			if(! m_mysql_feed.Query(oss.str()))
			{
				cout << oss.str() << "\n " << m_mysql_feed.GetErrMsg() << endl;
			}
			while(m_mysql_feed.GetRowCount() && m_mysql_feed.Next())
		    {
				m_feed_id_list.push_back( strtoul(m_mysql_feed.GetRow(2), NULL, 10) );
		    }
			/*
			for(size_t j = 0; j < m_feed_id_list.size(); j++)
			{
				cout << "F:" << m_feed_id_list[j] << endl;
			}
			*/

			oss.str("");
			oss << "select distinct(openid_md5) from " << m_table_prefix_feed_index << IntToHexStr( appid % 256)
				<< " where pa_appid_md5 = " << appid
				<< " and feed_type & " << count_feed_type << " = feed_type";

			if(! m_mysql_feed.Query(oss.str()))
			{
				cout << oss.str() << "\n " << m_mysql_feed.GetErrMsg() << endl;
			}

			while(m_mysql_feed.GetRowCount() && m_mysql_feed.Next())
		    {
				m_open_id_list.push_back( strtoul(m_mysql_feed.GetRow(0), NULL, 10) );
		    }
			/*
			for(size_t j = 0; j < m_feed_id_list.size(); j++)
			{
				cout << "U:" << m_open_id_list[j] << endl;
			}
			*/


			vector<uint64_t> m_score_list( m_open_id_list.size(), 0);
			cout << m_score_list.size() << endl;

            // get user
			uint64_t num_fan;
			m_open_id_list.push_back(2);
			m_score_list.push_back(0);

			for(size_t j = 0; j < m_open_id_list.size(); j++)
			{


				oss.str("");
				oss << "select count(openid_md5_to) from " << m_table_prefix_user_follow << IntToHexStr( m_open_id_list[j] % 256 )
					<< " where openid_md5_from = " << m_open_id_list[j]
					<< " and relation = 2";

				if(! m_mysql_user.Query(oss.str()))
				{
					cout << oss.str() << "\n " << m_mysql_user.GetErrMsg() << endl;
				}

				if(m_mysql_user.GetRowCount() && m_mysql_user.Next())
			    {
					num_fan =  strtoul(m_mysql_user.GetRow(0), NULL, 10) ;
			    }
				m_score_list[j] = num_fan * weight_fan;
				//cout << num_fan << " , " << m_score_list[j] << endl;
			}

			for(size_t j = 0; j < m_open_id_list.size(); j++)
			{
				cout << m_open_id_list[j] << ":: " << m_score_list[j] << endl;
				cout << endl;
			}


            //get feed
			uint64_t num_favorite, num_comment, openid;
			for(size_t j = 0; j < m_feed_id_list.size(); j++)
			{
				oss.str("");
				oss << "select * from " << m_table_prefix_feed<< IntToHexStr( m_feed_id_list[j] % 256 )
					<< " where feed_id = " << m_feed_id_list[j];
				if(! m_mysql_feed.Query(oss.str()))
				{
					cout << oss.str() << "\n " << m_mysql_feed.GetErrMsg() << endl;
				}

				if(m_mysql_feed.GetRowCount() && m_mysql_feed.Next())
			    {
			    	openid = strtoul(m_mysql_feed.GetRow(2), NULL, 10);
					num_favorite = strtoul(m_mysql_feed.GetRow(6), NULL, 10);
					num_comment = strtoul(m_mysql_feed.GetRow(8), NULL, 10);
			    }
			    cout << "f: " << m_feed_id_list[j] << endl;
				cout << "o: " << openid << endl;
				cout << "nf: " << num_favorite << endl;
				cout << "nc: " << num_comment << endl;
				cout << endl;
				for(size_t k = 0; k < m_open_id_list.size(); k++)
				{
					if(m_open_id_list[k] == openid)
					{
						m_score_list[k] += num_favorite * weight_favorite
											+ num_comment * weight_comment
											+ weight_feed;
						break;
					}
				}
			}
			/*
			for(size_t j = 0; j < m_open_id_list.size(); j++)
			{
				cout << m_open_id_list[j] << ":: " << m_score_list[j] << endl;
			}
			*/

			//statistic
			uint64_t count = count_expert < m_open_id_list.size() ? count_expert : m_open_id_list.size();
			for(size_t j = 0; j < count; j++)
			{
				size_t m = j;
				for(size_t k = j; k < m_open_id_list.size(); k++)
				{
					if(m_score_list[k] > m_score_list[m])
					{
						m = k;
					}
				}
				uint64_t tmp;
				tmp = m_score_list[j];
				m_score_list[j] = m_score_list[m];
				m_score_list[m] = tmp;

				tmp = m_open_id_list[j];
				m_open_id_list[j] = m_open_id_list[m];
				m_open_id_list[m] = tmp;
			}

			for(size_t j = 0; j < m_open_id_list.size(); j++)
			{
				cout << m_open_id_list[j] << ":: " << m_score_list[j] << endl;
			}
		}

		return true;
	}

protected:
	//DB
	string m_db_ip;
	string m_db_user;
	string m_db_pass;

	string m_db_name_feed;
	string m_table_prefix_feed;
	string m_table_prefix_feed_index;
	string m_db_name_user;
	string m_table_prefix_user_follow;

	lce::cgi::CMysql m_mysql_feed;
	lce::cgi::CMysql m_mysql_user;
protected:
	DECL_LOGGER(logger);
};

IMPL_LOGGER(CgiGenExpert, logger);

int main(int argc, char** argv)
{
	CgiGenExpert cgi;
   	cgi.DerivedInit();
    cgi.Process();

    return 0;
}

#endif

