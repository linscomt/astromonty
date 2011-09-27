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

LOAD DATA LOCAL INFILE "F:\\astromonty\\Programms\\MontyTest\\WBVR.DAT" INTO TABLE Stars;

DROP TABLE IF EXISTS Tycho2;
CREATE TABLE Tycho2 (
	recno 	INT UNSIGNED NOT NULL AUTO_INCREMENT, #���� �����
	TYC1  	INT UNSIGNED,
	TYC2  	INT UNSIGNED,
	TYC3  	INT UNSIGNED,
	pmRA  	DOUBLE NOT NULL,
	pmDE	DOUBLE NOT NULL,
	BTmag	DOUBLE NOT NULL,	
	VTmag	DOUBLE NOT NULL,
	RA		DOUBLE NOT NULL,
	DE		DOUBLE NOT NULL,	
	PRIMARY KEY (recno)	
);
LOAD DATA LOCAL INFILE "F:\\astromonty\\Programms\\MontyTest\\Tyho2.tsv" INTO TABLE Tycho2;
