-- 链接数据库
mysql -hpup	--h:地址 p:端口号 u:用户名 p:密码
mysql -hlocalhost -p3306 -upj -p0000

--查看数据库支持的存储引擎
show engines;

--查看数据库当前的存储引擎
show variables like '%storage_engine%';