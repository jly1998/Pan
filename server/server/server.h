#include <stdbool.h>
#include <iostream>
#include <fstream>
#include <cstdio>
#include <stdio.h>
#include <cstdlib>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <string>
#include <errno.h>
#include <fcntl.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <netinet/tcp.h>
#include <netdb.h>
#include <sys/prctl.h>
#include <sys/stat.h>
#include "mysql_select.h"

//接受缓存区大�?
#define BUFSIZE 1024
#define LINEBUF 1024

#define _BACKLOG_ 20

//数据长度限制
#define USER_LEN 45
#define PSWD_LEN 45

//数据库密�?
#define USER "u1750762"
#define DBPASSWD "u1750762"
#define DBNAME "db1750762"

//临时文件�?
#define ReqFileName ".request.txt"

//数据存储根目�?
#define DataRoot "./data"

typedef enum
{
    Error,       //错误
    SignUp,      //注册
    SignIn,      //登录
    Upload_req,  //上传文件
    Upload_data, //文件数据
    Download,    //下载文件
    Delete,      //删除文件
    GetDir,      //获取目录结构
    Move,
    Copy,
} request_type;  //请求的类�?

typedef enum
{
    other,
    json,       //application/x-www-form-urlencoded
    formdata,   //multipart/form-data
} content_type; //请求的contenttype

/*************tcp****************/
/**********
网盘客户端主进程
_port   端口号（字符�?
**********/
int Disk_server(char *_port);

/**********
建立监听socket，返回描述符
_port   端口号（字符�?
**********/
int S_CreateListenSock(char *_port);

/**********
接受连接
listen_sock     监听socket
Maxfd           当前最大socket
**********/
int S_AcConncs(int listen_sock, int &Maxfd);

/**********
将socket设置为非阻塞�?
iSock   套接字描述符
**********/
int SetNonBlock(int iSock);

/**********
����������
responseBody     �ظ�����
**********/
int DealRequest(string &responseBody, char *ip);

/**********
为上传文件夹建立新目�?
path            待建目录的根目录
RelativePath    某文件的相对路径
user            用户�?
**********/
int CreateRelativePath(string path, string RelativePath, string user);

/*************http协议处理****************/
/**********
��ȡ�������ģ������������ݵĳ���(>=0); ��С��0˵������, body��ֵ������
request     ����������
body        ȥ��ͷ��������Ĳ���
ctype       �������contenttype
boundary    ������Ϊformdata����ֵ��Ч��Ϊformdata��boundary
**********/
int GetRequestBody(int req_fd, char *&body, content_type &ctype, string &boundary, char *ip);

/**********
处理json，返回枚举类型request_type
request_body    去除头部后的正文部分
data_begin      若类型正确，data_begin指向请求类型后的数据起始地址；若类型错误，该值不可信
**********/
request_type RequestType(char *request_body, char *&data_begin, int &ContentLen);

/**********
处理用户注册登录的数据，若返回值不是_SUCCESS则username passwd的值不可信
data_begin      请求类型后的数据起始地址
username        用户int名，需要在调用本函数前申请好长度为USERLEN+1的空�?
passwd          密码，需要在调用本函数前申请好长度为PSWDLEN+1的空�?
**********/
int GetUserPasswd(char *data_begin, char *username, char *passwd);

/**********
处理待传输文件的详细信息，返回路径id，若返回_ERROR，则file_info结果不可�?
data_begin      指向请求类型后的数据起始地址
file_info       文件信息
**********/
int GetFileformat(char *&data_begin, file_format &file_info);

/**********
将目录编号转换为路径字符�?
DirNum          目录在数据库中的编号
num             目录结构表的大小
Cur_Dir         用户当前目录结构
**********/
string TransPath(int DirNum, int num, file_format *Cur_Dir);

/**********
处理上传到服务器的文件数据，返回收到的块编号，若<0则错�?
data_begin      请求类型后的数据起始地址
boundary        boundary
DataLen         收到的数据的长度
file_data       文件数据的起始地址，无需申请空间，需要释�?
file_info       文件信息，结构体中MD5和chunk总数有意�?
**********/
int GetFileData(char *&data_begin, string boundary, int &DataLen, char *&file_data, file_format &file_info);

/**********
获取用户的目录，返回string为json字符�?
file      数组的头指针
**********/
string GetFileTree(int num, file_format *file);

/**********
获取用户名，若返回值不是_SUCCESS则username的值不可信
data_begin      请求类型后的数据起始地址
username        用户�?
**********/
int GetUsername(char *data_begin, string &username);

/**********
处理用户删除文件的数据，若返回值不是_SUCCESS则fileid的值不可信
data_begin      请求类型后的数据起始地址
fileid          文件id int�?
**********/
int Getfileid(char *data_begin, int &fileid);

/**********
处理用户移动/复制文件/夹的数据，若返回值不是_SUCCESS则from_id，to_id的值不可信
data_begin      请求类型后的数据起始地址
from_id         待移�?/删除的文件id
to_id           将移�?/删除至的文件夹id
**********/
int Getfromtoid(char *data_begin, int &from_id, int &to_id);

/*************other****************/

/**********
������Ӧ����
socket  Ŀ��socket
code    ״̬��
Status  ���ݿ�����ķ���ֵ
body    ��Ӧ���ĵ����ݣ�����������bodyΪNULL
**********/
int SendResponse(int socket, int code, int Status, char *body, char *ip);

/**********
设置回复报文的正文内�?
Total       总包�?
Uploaded    服务器中已有包数
No          下个包的序号，若已传输结束则�?0
**********/
string SetResponseBody(int Total, int Uploaded, int No);

/**********
read方式获取一整行
fd  文件描述�?
linebuf 结果
**********/
int MyGetline(int fd, string &linebuf);

/**********
存储当前�?
ChunkNo         Chunk�?
DataLen         收到的数据的长度
file_data       文件数据的起始地址，无需申请空间，需要释�?
file_path       文件路径
**********/
int SaveFile(const int ChunkNo, const int DataLen, const char *file_data, const string file_path, const string file_name);

/**********
建立新目�?
path            待建目录的根目录
dirName         待建目录�?
**********/
int CreateDir(const char *path, const char *dirName);
