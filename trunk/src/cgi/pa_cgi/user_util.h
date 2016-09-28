#ifndef _CGI_USER_UTIL_H_
#define _CGI_USER_UTIL_H_

#define UserInfoPB2Any(pb, any) \
{ \
	any["openid_md5"] = int_2_str(pb.openid_md5()); \
	any["portrait_pic_id"] = int_2_str(pb.portrait_pic_id()); \
	any["wx_portrait_url"] = pb.portrait_pic_url_wx(); \
	any["nick"] = pb.nick(); \
	any["wx_nick"] = pb.nick_wx(); \
	any["addr_country"] = pb.addr_country(); \
	any["addr_city"] = pb.addr_city(); \
	any["sex"] = pb.sex(); \
	any["phone_no"] = int_2_str(pb.phone_no()); \
	any["email"] = pb.email(); \
	any["self_desc"] = pb.self_desc(); \
	any["main_page_cover_pic_id"] = int_2_str(pb.main_page_cover_pic_id()); \
	any["hoosho_no"] = int_2_str(pb.hoosho_no()); \
	any["birthday"] = pb.birthday_ts(); \
}

#define FeedInfoPB2Any(pb, any) \
{ \
	any["pa_appid_md5"] = int_2_str(pb.pa_appid_md5()); \
	any["openid_md5"] = int_2_str(pb.openid_md5()); \
	any["feed_id"] = int_2_str(pb.feed_id()); \
	any["cover_pic_id"] = int_2_str(pb.cover_pic_id()); \
	any["pic_id_list"] = pb.pic_id_list(); \
	any["content"] = pb.content(); \
	any["num_favorite"] = pb.num_favorite(); \
	any["num_share"] = pb.num_share(); \
	any["num_comment"] = pb.num_comment(); \
	any["num_read"] = pb.num_read(); \
	any["create_ts"] = pb.create_ts(); \
	any["del_ts"] = pb.del_ts(); \
	any["feed_type"] = pb.feed_type(); \
	any["extra_data_0"] = pb.extra_data_0(); \
	any["extra_data_1"] = pb.extra_data_1(); \
	any["extra_data_2"] = pb.extra_data_2(); \
}

#endif



