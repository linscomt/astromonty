DROP DATABASE IF EXISTS mybase; #�������, ���� ���� ����� ����
CREATE DATABASE mybase; #������� ����
USE mybase; # ��������, ��� ����� �� ����� ������������ ��� ����

#������� �������
DROP TABLE IF EXISTS Stars;
CREATE TABLE Stars (
	HD INT UNSIGNED NOT NULL AUTO_INCREMENT, #���� �����
	#HD INT NOT NULL , #����� HD
	Flags CHAR(16) default NULL,
	Alpha_grad INT NOT NULL,
	Alpha_Min DOUBLE NOT NULL,	
	Delta_grad INT NOT NULL,
	Delta_Min DOUBLE NOT NULL,
	Sp CHAR(16) default NULL, 
	W DOUBLE,
	B DOUBLE,
	V DOUBLE,
	R DOUBLE,
	N INT,
	c CHAR(16),
	PRIMARY KEY (HD)	
);
