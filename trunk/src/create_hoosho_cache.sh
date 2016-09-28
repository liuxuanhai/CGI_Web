#!/bin/sh

#官网cache
SESSION_PID_FILE=/tmp/memcached_session.pid
VC_PID_FILE=/tmp/memcached_vc.pid
if [ -f $SESSION_PID_FILE ]; then
	kill -9 `cat $SESSION_PID_FILE`
	rm -rf $SESSION_PID_FILE
fi
if [ -f $VC_PID_FILE ]; then
	kill -9 `cat $VC_PID_FILE`
	rm -rf $VC_PID_FILE
fi

touch $SESSION_PID_FILE
touch $VC_PID_FILE
/usr/local/bin/memcached -d -m 10 -u root -l 127.0.0.1 -p 39001 -c 256 -P $SESSION_PID_FILE
/usr/local/bin/memcached -d -m 10 -u root -l 127.0.0.1 -p 39002 -c 256 -P $VC_PID_FILE

#卡券cache
SESSION_PID_FILE=/tmp/memcached_session_card.pid
VC_PID_FILE=/tmp/memcached_vc_card.pid
if [ -f $SESSION_PID_FILE ]; then
        kill -9 `cat $SESSION_PID_FILE`
        rm -rf $SESSION_PID_FILE
fi
if [ -f $VC_PID_FILE ]; then
        kill -9 `cat $VC_PID_FILE`
        rm -rf $VC_PID_FILE
fi

touch $SESSION_PID_FILE
touch $VC_PID_FILE
/usr/local/bin/memcached -d -m 10 -u root -l 127.0.0.1 -p 49001 -c 256 -P $SESSION_PID_FILE
/usr/local/bin/memcached -d -m 10 -u root -l 127.0.0.1 -p 49002 -c 256 -P $VC_PID_FILE

#管理端cache
SESSION_PID_FILE=/tmp/memcached_session.pid
VC_PID_FILE=/tmp/memcached_vc.pid
if [ -f $SESSION_PID_FILE ]; then
	kill -9 `cat $SESSION_PID_FILE`
	rm -rf $SESSION_PID_FILE
fi
if [ -f $VC_PID_FILE ]; then
	kill -9 `cat $VC_PID_FILE`
	rm -rf $VC_PID_FILE
fi

touch $SESSION_PID_FILE
touch $VC_PID_FILE
/usr/local/bin/memcached -d -m 10 -u root -l 127.0.0.1 -p 39901 -c 256 -P $SESSION_PID_FILE
/usr/local/bin/memcached -d -m 10 -u root -l 127.0.0.1 -p 39902 -c 256 -P $VC_PID_FILE


#添加其他cache， 注意端口号， 进程文件名
