-- 链接数据库
mysql -hpup	--h:地址 p:端口号 u:用户名 p:密码
mysql -hlocalhost -p3306 -upj -p0000

--查看数据库支持的存储引擎
show engines;

--查看数据库当前的存储引擎
show variables like '%storage_engine%';

--建立单值索引
create index idx_user_name on user(name);

--建立复合索引
create index idx_user_nameEmail on user(name,email);

--查看索引
show index from table_name\G

-- 建立员工表
create table staffs
(
 id int primary key auto_increment,
 name varchar(24) not null default '' comment '姓名',
 age int not null default 0 comment '年龄',
 pos varchar(20) not null default '' comment '职位',
 add_time timestamp not null default current_timestamp comment '入职时间',
)charset utf8 comment '员工记录表';

--插入数据
insert into staffs(name, age, pos, add_time) VALUES('z3', 22, 'manager', now());

--添加复合索引
alter table staffs add index idx_staffs_nameAgePos(name, age, pos);