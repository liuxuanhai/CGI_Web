#!/usr/bin/sh

echo ""

db_user="root"
db_password=".hoosho,9942"

db_name_feeds="d_feeds"
table_prefix_feed_index="t_feed_index_on_appid_"
table_prefix_feed="t_feed_"

db_name_user="d_user"
table_prefix_user_follow="t_user_follow_"

db_name_pa="d_pa"
table_prefix_expert="t_pa_expert"
table_name_pa="t_pa_info"

db_err_path="/dev/null"
db_err_path="./dberr"
db_err_path="./dberr"
echo "db_err_path: $db_err_path"

#统计的feed类型：普通feed + 话题下的feed
count_feed_type=5

#达人的筛选公式：
#粉丝数（40%） + 评论数（35%）+ 点赞数（15%） + 贴子数（10%）
weight_fan=40
weight_comment=35
weight_favorite=15
weight_feed=10

#达人总数
count_expert=50


day_time=`expr 60 \* 60 \* 24`
echo "day_time: $day_time"
#统计时间
count_time=`expr $day_time \* 7`
echo "count_time: $count_time"
#count_time=1

cur_ts=`date +%s`
echo "cur_ts: $cur_ts"
#cur_ts=`expr $cur_ts / $day_time \* $day_time`
#echo "cur_ts: $cur_ts"

begin_ts=`expr $cur_ts - $count_time `
echo "begin_ts: $begin_ts "

begin_feed_id=`expr $begin_ts \* 1000 \* 256 \* 256`
echo "begin_feed_id: $begin_feed_id"


appid_list=()

#获取公众号列表
function get_appid_list()
{
    appid_list=()
    sql="select distinct(pa_appid_md5) from $table_prefix_feed_index"
    n=0
    for (( i=0; i<256; i++ ))
    do
        index=`printf %02x $i`
        #echo "i: $index"
        ret=$(mysql -u${db_user} -p${db_password} $db_name_feeds -e "$sql$index"  2>$db_err_path | awk 'NR > 1')
        #echo "sql: $sql$index"
        #echo "ret: $ret"
        for appid in ${ret[@]}
        do
            #echo "a: $appid"
            appid_list[$n]=`expr 0 + $appid`
            n=`expr $n + 1`
        done
    done

    echo "PA SIZE: ${#appid_list[@]}"
}

feedid_list=()
openid_list=()
#获取公众号下的feedid和openid
function get_feedid_and_openid_list()
{
	#echo "parm $1"
	appid=$1
	#echo "app $appid"
    echo ""
	echo ""
	echo "======================================="
	echo "        APPID: $appid"
	echo "======================================="

	index=`expr $appid % 256`
	#echo "index: $index"
	index=`printf "%02x" $index`
	echo "hex index: $index"

	sql="from $table_prefix_feed_index$index where pa_appid_md5 = $appid and feed_type & $count_feed_type = feed_type and feed_id > $begin_feed_id"

	#origin data, for check
	#echo "select * $sql \G" | mysql -u${db_user} -p${db_password} $db_name_feeds 2>>$db_err_path

	ret=$(mysql -u${db_user} -p${db_password} $db_name_feeds -e "select feed_id $sql" 2>>$db_err_path | awk 'NR > 1')

	#str 2 array
	feedid_list=()
	i=0
	for feedid in ${ret[@]}
	do
		#echo "f: $feedid"
		feedid_list[$i]=$feedid
		i=`expr $i + 1`
	done
	echo "feed size:  ${#feedid_list[@]}"

	ret=$(mysql -u${db_user} -p${db_password} $db_name_feeds -e "select distinct(openid_md5) $sql" 2>>$db_err_path | awk 'NR > 1'  )
	#str 2 array
	openid_list=()
	i=0
	for openid in ${ret[@]}
	do
		#echo "o: $openid"
		openid_list[$i]=$openid
		i=`expr $i + 1`
	done
	echo "user size: ${#openid_list[@]}"

}

score_list=()
function get_user_follow()
{
    score_list=()

    echo ""
	echo "+---------------+"
	echo "|GET USER FOLLOW|"
	echo "+---------------+"
	echo "BEGIN"

	#get num_fans, init score_list
	i=0
	for openid in ${openid_list[@]}
	do
		echo ""
		echo "----------------------"
		echo "OPENID: $openid"

		index=`expr $openid % 256`
		#echo "index: $index"
		index=`printf "%02x" $index`
		#echo "hex index: $index"
		sql="select count(openid_md5_to) from $table_prefix_user_follow$index where openid_md5_from = $openid and relation = 2";
		num_fan=$(mysql -u${db_user} -p${db_password} $db_name_user -e "$sql"  2>>$db_err_path | awk 'NR > 1')
		echo "num_fan: $num_fan"

		score_list[$i]=`expr $num_fan \* $weight_fan`
		echo "sco: ${score_list[$i]}"

		i=`expr $i + 1`
		echo "----------------------"
	done
	echo "END"
}

function get_feed_info()
{
    echo ""
	echo "+-------------+"
	echo "|GET FEED INFO|"
	echo "+-------------+"
	echo "BEGIN"

	for feedid in  ${feedid_list[@]}
	do
		echo ""
		echo "----------------------"
		echo "FEEDID: $feedid"
		index=`expr $feedid % 256`
		index=`printf "%02x" $index`
#		echo "hex index: $index"

		sql="from $table_prefix_feed$index where feed_id = $feedid"

		#origin data, for check
#echo "select * $sql \G" | mysql -u${db_user} -p${db_password} $db_name_feeds  2>>$db_err_path

		num_comment=$(mysql -u${db_user} -p${db_password} $db_name_feeds -e "select num_comment $sql"  2>>$db_err_path  | awk 'NR > 1')
		echo "num_comment: $num_comment"

		num_favorite=$(mysql -u${db_user} -p${db_password} $db_name_feeds -e "select num_favorite $sql"  2>>$db_err_path  | awk 'NR > 1')
		echo "num_favoirte: $num_favorite"

		owner=$(mysql -u${db_user} -p${db_password} $db_name_feeds -e "select openid_md5 $sql"  2>>$db_err_path  | awk 'NR > 1')
		echo "owner: $owner"

		score=`expr $num_comment \* $weight_comment + $num_favorite \* $weight_favorite + $weight_feed`
		echo "cur sco: $score"

		i=0
		for openid in ${openid_list[@]}
		do
			if [ "$openid" == "$owner" ]
			then
				echo "bef sco: ${score_list[$i]}"
				score_list[$i]=`expr ${score_list[$i]} + $score`
				echo "aft sco: ${score_list[$i]}"
				break
			fi
			i=`expr $i + 1`
			#echo $i
		done

		echo "----------------------"
	done
	echo "END"
}

function sort_score()
{
	echo ""
	echo ""
	echo "+---------+"
	echo "|STATISTIC|"
	echo "+---------+"
	echo ""


	for (( i=0; i<${#openid_list[@]}; i++))
	do
		echo "user = ${openid_list[$i]}, score = ${score_list[$i]} "
	done

	#sort
	if [ ${#openid_list[@]} -gt $count_expert ]
	then
		count=$count_expert
	else
		count=${#openid_list[@]}
	fi

	for (( i=0; i<$count; i++))
	do
		max=$i
		for (( j=$i; j<${#openid_list[@]}; j++))
		do
			if [ ${score_list[$j]} -gt ${score_list[$max]} ]
			then
			max=$j
			fi
		done

		#echo ""
		#echo "GT ${score_list[$i]} , ${score_list[$max]} GT"
		#echo "GT ${openid_list[$i]} , ${openid_list[$max]} GT"
		tmp=${score_list[$i]}
		score_list[$i]=${score_list[$max]}
		score_list[$max]=$tmp

		tmp=${openid_list[$i]}
		openid_list[$i]=${openid_list[$max]}
		openid_list[$max]=$tmp

		#echo "GT ${score_list[$i]} , ${score_list[$max]} GT"
		#echo "GT ${openid_list[$i]} , ${openid_list[$max]} GT"
	done

	echo ""
	echo "After Sort"
	echo ""

	for (( i=0; i<${#openid_list[@]}; i++))
	do
		echo "user = ${openid_list[$i]}, score = ${score_list[$i]} "
	done

	echo ""

	#clear old data
	echo "Clear old data ..."
	index=`expr $appid % 256`
#	echo "index: $index"
	index=`printf "%02x" $index`
#	echo "hex index: $index"

	sql="delete from $table_prefix_expert$index"
#	echo "DDD sql: $sql"
	ret=$(mysql -u${db_user} -p${db_password} $db_name_pa -e " $sql "  2>>$db_err_path )
	echo "Clear old data end"
	echo ""

	#insert
	echo "Insert new data ..."
	create_ts=$cur_ts
	for (( i=0; i<$count; i++))
	do
		sql="insert into $table_prefix_expert$index set appid_md5 = $appid , openid_md5 = ${openid_list[$i]}, create_ts = $create_ts, extra_data_0='${score_list[$i]}'"
		#echo "III sql: $sql"
		ret=$(mysql -u${db_user} -p${db_password} $db_name_pa -e " $sql "  2>>$db_err_path )
		create_ts=`expr $create_ts - 1`
	done
	echo "Insert new data end"
	echo ""

}

get_appid_list

for appid in ${appid_list[@]}
do

	get_feedid_and_openid_list $appid

	get_user_follow

    get_feed_info

    sort_score
done

