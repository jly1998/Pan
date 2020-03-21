
CREATE DATABASE IF NOT EXISTS db1750762;
use db1750762;

/*---------------------------------------------user--------------------------------------------------*/

-- 创建一个表user，存储网盘的用户信息
CREATE TABLE IF NOT EXISTS user(
    userid INT(11) NOT NULL AUTO_INCREMENT,-- 用户的编号，根据创建时间自增
    username VARCHAR(45) NOT NULL,-- 用户�?
    pwd VARCHAR(45) NOT NULL,-- 用户密码
    regedit_date DATE DEFAULT '1900-01-01',
    PRIMARY KEY (userid)
);

-- 查找表user
-- SELECT * FROM user;

--  删除表user
-- DROP TABLE IF EXISTS user;

/*---------------------------------------------file_block--------------------------------------------------*/
CREATE TABLE IF NOT EXISTS file_block(
    MD5 CHAR(32) NOT NULL, -- 文件的唯一标识，主�?
    block_no INT(11) NOT NULL, -- 尚未被上传的块号
    ifusing TINYINT(1) NOT NULL,-- 是否正在被上传，是为1，否则为0
    usingtime BIGINT(21) DEFAULT NULL,-- 块最近一次被上传的时�?
    PRIMARY KEY(MD5,block_no) 
);

/*---------------------------------------------file_store--------------------------------------------------*/
-- 文件的实际存储描�?
CREATE TABLE IF NOT EXISTS file_store(
    memid  INT(11) NOT NULL AUTO_INCREMENT,-- 在内存中占有一席之�?
    path  VARCHAR(256) NOT NULL,-- 文件的实际存储位�?
    MD5 CHAR(32) NOT NULL,-- MD5信息摘要算法得到�?128位长度的散列值，用于判断文件是否已经被上传过
    filesize INT(11) NOT NULL,-- 文件总大�?
    usernum INT(11)DEFAULT 0,-- 拥有该文件的用户�?
    PRIMARY KEY (memid)
);

/*---------------------------------------------file_format--------------------------------------------------*/

-- 创建一个表file_format，存储文件格�?
CREATE TABLE IF NOT EXISTS file_format(
    fileid INT(11) NOT NULL  AUTO_INCREMENT,
                            -- 文件的编号，不能根据创建时间自增？因为如果删除文件，占用的编号不能复用，编号会耗尽
                            -- 不同的用户的文件编号可以重叠�?
                            -- mysql中规定自增列必须为唯一主码，那么好吧，给所有文件以唯一编号
    filename VARCHAR(45) DEFAULT "新建文件",-- 文件�?
    username VARCHAR(45) NOT NULL,-- 文件的创建用�?
    create_date DATE DEFAULT NULL,-- 文件创建时间
    MD5 CHAR(32) DEFAULT NULL,-- 在内存中的映�?
    parentid INT(11) NOT NULL,-- 文件的上层文件夹
    isfolder TINYINT(1) NOT NULL,-- 是否是文件夹，文件夹�?1，文件为0 
    nowsize INT(11)DEFAULT NULL,-- 文件上传进度
    filepath VARCHAR(256) NULL,-- 文件在网盘中的存�?
                               -- 文件在网盘中的存储的根目录是用户名，这样一个文件唯一对应一个路径名
                               -- 传输�?/来自前端的路径不包用户名，需做处�?
    PRIMARY KEY(fileid)-- 主码为文件编�?
);


-- 查找表file_format
-- SELECT * FROM file_format;

--  删除表file_format
-- DROP TABLE IF EXISTS file_format;

-- 删除表中所有内�?
-- DELETE FROM user;
-- DELETE FROM file_block;
-- DELETE FROM file_format;
-- DELETE FROM file_store;



/*
-- file_tree

-- 创建表file_tree，存储文件树
CREATE TABLE IF NOT EXISTS file_tree(
    fid INT(11) NOT NULL,-- 文件编号
    cid INT(11) NOT NULL,-- 文件的创建用�?
    pid INT(11) NOT NULL,-- 文件的上层文件夹
    layer INT(11) NOT NULL,-- 文件所在的层数
    FOREIGN KEY (cid) REFERENCES user(userid),
    FOREIGN KEY (fid) REFERENCES file_format(fileid),
    FOREIGN KEY (pid) REFERENCES file_format(fileid),
    PRIMARY KEY (fileid)
);

-- 向file_tree中插入数�?
INSERT file_tree(fileid,creator_id,parent_file) values('1','1','1');

-- 查找表user
SELECT * FROM file_tree;

--  删除表file_tree
DROP TABLE IF EXISTS file_tree;

*/
