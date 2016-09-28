#!/bin/bash

HOME=/home/dev/hoosho/server/co_msg_server/
ENV_HOME=CO_MSG_SERVER_HOME_PATH
SERVER_BIN=hoosho.co_msg_server

CMD_START=start
CMD_STOP=stop
CMD_RESTART=restart

function check_server()
{
		processnum=`ps ax|awk '{print $5}'|sed -n /^\\\\.\\\\/${SERVER_BIN}$/p|wc -l`
		return $processnum
}

function print_usage()
{
		echo "Usage: "
				echo -e "\t$1 $CMD_START|$CMD_STOP|$CMD_RESTART"
}

function init()
{
		cd $HOME
		export $ENV_HOME=$HOME
}



if [ $# -lt 1 ]; then
	
	print_usage $CTL_BIN
	exit 0

fi

cmd=$1
init

if [ $cmd = $CMD_START ]; then
	ulimit -c unlimited
	check_server
	if [ $? -ne 0 ]; 
	then
		echo "$SERVER_BIN already running"
		exit 0
	fi
	./${SERVER_BIN} -d
	
elif [ $cmd = $CMD_STOP ]; then

	killall -9 $SERVER_BIN

else
	print_usage $0
fi
