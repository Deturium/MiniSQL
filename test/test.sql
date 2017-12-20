create table student(
	sid char(20),
	name char(20),
	age int,
	score int unique,
	primary key(sid)
);

create index score on student(score);

select * from student;

insert into student values("3150102418", "Zhang Zhuohao", 20, 95);
insert into student values("3150103086", "Liang yangfan", 20, 100);
insert into student values("3150101111", "Nobody", "20", "60");

select * from student;

select * from student where score >= 90;

delete from student where name = "Nobody";

delete from student;

select * from student;

drop table student;