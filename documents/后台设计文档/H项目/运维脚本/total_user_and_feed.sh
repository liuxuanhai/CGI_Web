#!/bin/sh
#统计当前总的feed数和用户数

total=0;
for((i=0;i<256;i++))
do
    index=`printf "%02x" $i` ;
    ret=$(mysql -uroot -p.hoosho,9942 d_user -e "select count(*) from t_user_info_$index" 2>/dev/null | awk 'NR>1') ;
    total=`expr $total + $ret`;
done;

echo "user: $total";

total=0;
for((i=0;i<256;i++))
do
    index=`printf "%02x" $i` ;
    ret=$(mysql -uroot -p.hoosho,9942 d_feeds -e "select count(*) from t_feed_$index" 2>/dev/null | awk 'NR>1') ;
    total=`expr $total + $ret`;
done;

echo "feed: $total";
