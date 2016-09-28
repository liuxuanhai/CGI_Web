#!/bin/bash

CTL=/home/dev/hoosho/server/token_server/ctl_hoosho_token_server.sh
SERVER_BIN=hoosho.token_server

pn=`ps ax | awk '{print $5}' | grep $SERVER_BIN | wc -l`
if [ $pn -eq 0 ] ; then
	$CTL start 
fi

