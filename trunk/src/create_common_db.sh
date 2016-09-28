#!/bin/sh

db_name="common"

echo "create database if not exists $db_name" | mysql -uroot -p.hoosho,9942

# table pic
pic_table_def="(
	  id char(255) not null default ''
	, type tinyint unsigned not null default 0
	, data mediumblob not null default '' 
	, source tinyint unsigned not null default 0
	, owner_uin char(255) not null default ''
	, primary key (id)
)ENGINE=InnoDB DEFAULT CHARSET=utf8 	
"
echo "create table if not exists pic $pic_table_def"  | mysql -uroot -p.hoosho,9942 $db_name


# table doc
doc_table_def="(
	  id char(255) not null default ''
	, type tinyint unsigned not null default 0
	, data mediumblob not null default '' 
	, source tinyint unsigned not null default 0
	, owner_uin char(255) not null default ''
	, primary key (id)
)ENGINE=InnoDB DEFAULT CHARSET=utf8 	
"
echo "create table if not exists doc $doc_table_def"  | mysql -uroot -p.hoosho,9942 $db_name
