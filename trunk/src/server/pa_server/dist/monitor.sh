#!/bin/bash

CTL=/home/dev/hoosho/server/pa_server/ctl_hoosho_pa_server.sh
SERVER_BIN=hoosho.pa_server

pn=`ps ax | awk '{print $5}' | grep $SERVER_BIN | wc -l`
if [ $pn -eq 0 ] ; then
	$CTL start 
fi

