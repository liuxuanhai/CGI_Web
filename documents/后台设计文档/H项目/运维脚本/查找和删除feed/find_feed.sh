#!/bin/sh

#查找含有关键词的feed
content=$1
echo "Key: $content"
echo ""
echo "Fromat: APPID, FEEDID, CONTENT"
echo ""
for((i=0;i<256;i++)) 
do
    index=`printf '%02x'  $i`
    #echo $index
    sql="select pa_appid_md5, feed_id, content from t_feed_$index where content like '%""$content""%' "
    #echo $sql
    ret=$(mysql -uroot -p.hoosho,9942 d_feeds  -e  "$sql" 2>/dev/null | awk 'NR > 1') ;

    #if [ "$ret" = "" ]
    #then
    #   continue
    #fi
    for r in ${ret[@]}
    do
        echo $r
    done
done
