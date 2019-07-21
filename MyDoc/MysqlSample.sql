-- �������ݿ�
mysql -hpup	--h:��ַ p:�˿ں� u:�û��� p:����
mysql -hlocalhost -p3306 -upj -p0000

--�鿴���ݿ�֧�ֵĴ洢����
show engines;

--�鿴���ݿ⵱ǰ�Ĵ洢����
show variables like '%storage_engine%';

--������ֵ����
create index idx_user_name on user(name);

--������������
create index idx_user_nameEmail on user(name,email);

--�鿴����
show index from table_name\G

-- ����Ա����
create table staffs
(
 id int primary key auto_increment,
 name varchar(24) not null default '' comment '����',
 age int not null default 0 comment '����',
 pos varchar(20) not null default '' comment 'ְλ',
 add_time timestamp not null default current_timestamp comment '��ְʱ��',
)charset utf8 comment 'Ա����¼��';

--��������
insert into staffs(name, age, pos, add_time) VALUES('z3', 22, 'manager', now());

--��Ӹ�������
alter table staffs add index idx_staffs_nameAgePos(name, age, pos);