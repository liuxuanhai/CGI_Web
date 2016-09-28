#!/bin/sh
#删除feed
echo $#
if [ "$#" -ne "2" ]
then
    echo "Usage: del_feed.sh APPID FEEDID"  
    exit
fi

appid=$1
echo "Appid: $appid"
feedid=$2
echo "Feedid: $feedid"
echo ""

index=`expr $appid % 256`
echo $index
index=`printf "%02x" $index`
echo $index
echo ""

echo "SELECT"
sql="select * from t_feed_index_on_appid_$index where feed_id = $feedid"
echo $sql
ret=$(mysql -uroot -p.hoosho,9942 d_feeds -e "$sql" 2>/dev/null | awk 'NR > 1')
echo $ret
echo ""

echo "DELETE"
sql="delete from t_feed_index_on_appid_$index where feed_id = $feedid"
echo $sql
ret=$(mysql -uroot -p.hoosho,9942 d_feeds -e "$sql" 2>/dev/null | awk 'NR > 1')
echo $ret

