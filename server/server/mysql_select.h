#include <iostream>
#include <string>
#include <string.h>
#include <stdio.h>
#include <cstdio>
#include <stdlib.h>
#include <mysql.h>
#include <time.h>
#include <iomanip>
#include <unistd.h>//sleep
#include <sys/time.h>

#include"MD5.h"

using namespace std;


#define tempsize 256
#define BLOCKSIZE 1024 

#define _SUCCESS 0
#define _ERROR -1
#define _WRONGDATA -2
#define _USERWRONG -3

//默认目录
#define PathResource    "myResource"
#define PathApp         "myApp"


//表示没有需要上传的块
#define NOBLOCK -4
//表示所有块正在被上传，等待并持续发送请求块
#define WAITBLOCK -5
//表示上传的文件重名
#define SAMENAME -6
//表示将文件夹移动到他的子文件夹里面
#define MOVETOSON -7
//表示将文件/夹移动/复制到文件中
#define WRONGTOFILE -8


#define codelength 12

//ifusing 
#define ISUSING 1
#define NOUSING 0

//iffolder
#define ISCATALOG 1
#define ISFILE 0

//if_finished
#define FINISHED 1
#define CONTINUE 2

//block传送超时时间.ms
#define ALARM 10

struct NowDate
{
    char tmp0[16]; //年月日
    char tmp1[16]; //时分秒
    char tmp2[4];  //毫秒
};

//将当前时间转换为NowDate结构体
NowDate getTime();

//获得毫秒数量级
long getCurrentTimeMsec();

class MyDB;

//记录每个文件的上传进度，每次从表中查询得到一个块号，发送给web端，并将表项中的用户占有置为true
//file_block
typedef struct file_block
{
    string MD5;
    int block_no;
    bool ifusing;
    long usingtime;
}file_block;

//为每个表建立一个结构体，方便赋值的时候
//user
typedef struct user
{
    int userid;
    string username;
    string pwd;
    NowDate regedit_date;
} user;

//每个文件的实际存储空间
//file_store
typedef struct file_store
{
    int memid;
    string path;//文件的实际存储路径
    string MD5;
    int filesize;//文件的总大小
    int usernum;//文件的用户持有数
} file_store;

//与客户端沟通数据的结构体，并不能全权代表数据库中的表结构
//file_format
typedef struct file_format
{
    int fileid;//文件唯一编号
    string filename;//文件名
    string username;//创建的用户名 *from web
    NowDate create_date;//创建日期 
    string MD5;//文件的MD5值 *from web
    int parentid;//父目录编号 
    bool isfolder;//文件夹标识 *from web
    int filesize;//文件大小 *from web
    string filepath;//文件路径，不带用户名 *from web
    int block_num;//块数 *from web
    friend MyDB;
} file_format;

////////////////////////////////////////////////////////
class MyDB
{
public:
    friend file_format;
    MyDB();
    ~MyDB();
    bool initDB(string host, string user, string passwd, string db_name); //连接mysql
    bool exeSQL(string sql);
    //执行sql语句

    //关闭与MySQL的链接
    void closeSQL();
    //输出select的结果
    bool outselect();



    /********************用户操作//**********************************************************
    func:   判断用户注册是否可以成功
            密码格式：长度不小于8，必须同时含有字母/数字/特殊字符，并且不能 含有非法字符(<=32)
            封装if_signup，给客户端用
    input:  string user
            string code
    output: int 
            成功返回true，失败返回错误原因
            失败的原因：_USEREXIST：用户名已经存在
                       _WRONGDATA：密码设置不符规定
    ***********************************************************/
    int signup(string user, string code);

    /**********************************************************
    func:   判断用户登陆操作是否合法
    input:  string user 用户名
            string code
    output: int 
            合法返回 _SUCCESS
            密码不匹配返回 _WRONGDATA
            用户名不存在返回 _USERWRONG
    ***********************************************************/
    int if_signin(string user, string code); //判断用户登陆是否成功

    /********************文件操作**************************************/
    
    /**********************************************************
    func:   用户上传一份文件，更新file_format；
            相当于封装了upload_file
    input:  file_format 来自客户端的结构体，按照相关约定
    output: 与upload_file的返回值相同
            int 标志位
            _SUCCESS    在file_format中插入相应记录
                        在file_store中插入相应记录
            <0 出现error，file_format无效
                SAMENAME 文件重名
    ***********************************************************/
    int sealed_upload(file_format file);

    /**********************************************************
    func:   删除一份文件，不删除物理内存
            更改两份表，删除file_format中的记录，更新file_store
            相当于封装了delete_file
    input:  string path 路径包含用户名（根目录
    output: void
    ***********************************************************/
    void sealed_delete(file_format file);

    /**********************************************************
    func:   移动文件/夹
            相当于封装了move_file和move_catalog
    input:  int from_id 起始文件id
            int to_id 终止路径的文件夹id
                    需要内部自己转换为结构体
    output: void
    *********************************************************      */
    int sealed_move(int from_id,int to_id);

     /**********************************************************
     func:   复制文件/夹
             相当于封装了move_file和move_catalog
             同文件夹下复制会追加数字区分文件名
     input:  int from_id 起始文件id
             int to_id 终止路径的文件夹id
                     需要内部自己转换为结构体
     output: 文件/夹复制结果
     **********************************************************     */
     int sealed_copy(int from_id,int to_id);


    




    /**********************************************************
    func:   用户传来一个路径，通过地址去查询数据库，返回给用户  该层表结构
    input:  file_format 来自客户端的结构体，按照相关约定
            有效值   filepath
                    username
            file_format 数组，即该目录下的文件结构
    output: int 返回查询到的结果的行数
    ******************************************************  *****/
    int find_content(const file_format file);

    /**********************************************************
    func:   此函数与find_content(const file_format file)配  套使用,
            此时result中已存储了查询结果
    input:  file_format* 数组，即该目录下的文件结构，由该函数   写入  
    output: int 返回查询到的结果的行数，用于动态申请    file_format数组的大小
    ******************************************************  *****/
    void store_format(file_format * file);

    /**********************************************************
    func:   查询重命名，是否可以成功，给客户端用
    input:  file_format 
            有效值  username
                    filepath
            string newname
    output: int 返回操作结果
                _SUCCESS 成功重命名并更改file_format中相应的    表记录
                SAMENAME 发生重名问题，重命名失败
    ******************************************************  *****/
    int ifrename(file_format file,string newname);


    /********************块操作**************************************/

    /**********************************************************
    func:   查看某文件是否还有为未上传块，客户端调用
            每次查看块表，若有块被占用，去查占用时间，超时则该块也  在可用列表
    input:  string MD5 文件的唯一标识
    output: int 
            若文件尚有未上传块，返回值为下一个需要上传的块号，并修  改ifusing为ISUSING
            否则 若文件所有块已经被上传结束，返回NOBLOCK
                 若文件中仍有块未被上传但是它们都正在被上传，返回WAITBLOAK，等待一段时间后继续发送传包请求，直到文件上传结束或自动“退出上传”
    **********************************************************  */
    int next_block(string MD5);

    /**********************************************************
    func:   用户上传一个块成功，删除file_block中的块记录，客户端调用
    input:  file_block block 结构体
                string MD5 文件的唯一标识，
                int block_no 块号
    output: int 
            删除成功返回_SUCCESS
            删除失败返回_ERROR
    ***********************************************************/
    int upblock_success(file_block block);

    /**********************************************************
    func:   用户上传一个块失败，更新file_block中的块记录，将用户占用位清    空，客户端调用
    input:  file_block block 结构体
                string MD5 文件的唯一标识，
                int block_no 块号
    output: int 
            设置成功返回_SUCCESS
            设置失败返回_ERROR
    ***********************************************************/
    int upblock_failed(file_block block);

    /**********************************************************
    func:   返回当前文件已经传输的块数，给客户端
    input:      string MD5 文件的唯一标识，
                int block_num 文件一共的块数
    output: int 
            传输完成的块数
    ***********************************************************/
    int finished_block(string MD5,int block_num);

    /********************其他操作**************************************/
    void getresult(string **temp_result);

    /**********************************************************
    func:   根据MD5值，返回给客户文件实际存储路径（带用户名，相当   于数据库中存储路径）
            给客户端用
    input:  string MD5
            string &filename "newfile"
            string &parentpath "/fenhui/jisuanji"
    output: int
            若查询失败，返回_ERROR
            若查询成功，返回_SUCCESS
    **********************************************************  */
    int getroad(string MD5,string &filename,string &   parentpath);

    /**********************************************************
    func:   根据用户名，得到该用户的目录结构体数组
            给客户端用
    input:  string username
            file_format &*file 结构体数组的引用？
    output: int
            查询成功返回查询到的行数
            查询失败返回_ERROR
    **********************************************************  */
    int user_filetree(string username,file_format *&   file);

private:
    MYSQL *mysql;      //连接mysql句柄指针
    MYSQL_RES *result; //指向查询结果的指针
    MYSQL_ROW row;     //按行返回的查询信息



    /********************用户操作**************************************/

    
    /**********************************************************
    func:   查询s是否已经在table中，s为候选码
    input:  string table 
            string s
    output: bool
            已经存在返回false，否则返回true
    ***********************************************************/
    bool if_insert(string table, string s);

    /**********************************************************
    func:   判断用户注册是否可以成功
            密码格式：长度不小于8，必须同时含有字母/数字/特殊字符，并且不能 含有非法字符(<=32)
    input:  string user
            string code
    output: int 
            成功返回true，失败返回错误原因
            失败的原因：_USEREXIST：用户名已经存在
                       _WRONGDATA：密码设置不符规定
    ***********************************************************/
    int if_signup(string user, string code); //判断用户注册是否成功



    /********************文件操作**************************************/
    /**********************************************************
    func:   为每一个新建用户建立一个默认文件目录
            此时尚未有文件，所做的操作只是向file_format中添加两个文件夹
    input:  string user
    output: int
            建立成功返回_SUCCESS
            建立失败返回_ERROR
    ***********************************************************/
    int init_catalog(string user);

    /**********************************************************
    func:   用户上传一份文件，首先查询file_store；
            如果命中，更新用户数；否则，在file_store中插入相应表记录
    input:  string path 文件路径
            string MD5 文件MD5值
            int filesize 文件大小
    output: bool
            命中返回true；否则返回false
    ***********************************************************/
    bool ifexist_store(string path,string MD5, int filesize);

   /**********************************************************
    func:   同用户同文件夹下是否有重名；
    input:  struct file_format
            有效值  filepath
    output: bool
            有重名返回false，无重名返回true
    **********************************************************  */
    bool ifsame_filename(file_format file);
    /*********************************************************
    func:   根据路径名得到一个文件夹 编号；
    input:  string path
    output: int parentid
    **********************************************************/
    int path_to_id(string path);

    /******************************************************* ***
    func:   根据编号从数据库中查询到一个file_format返回
    input:  int fileid
    output: file_format
    ******************************************************* ****/
    file_format id_to_format(int fileid);
 

    /*********************************************************
    func:   判断文件2是不是文件1的子文件
    input:  string id1
            string id2
    output: bool
            如果是返回true
            不是返回false
    *********************************************************             */
    bool i_am_your_father(string id1,string id2);



    /**********************************************************
    func:   文件名的重命名处理问题，追加数字以示区分
    input:  string &filepath
    output: void
    **********************************************************    */
    void newfilename_deal(string &filepath);

    /**********************************************************
    func:   用户上传一份文件，更新file_format,file_store
    input:  string path 文件路径（含文件名
            char *MD5 文件MD5值
            int isfolder 标志位，是否是文件夹
            int filesize 文件大小
            int block_num 块数
    output: int 标志位
            _SUCCESS    在file_format中插入相应记录
                        在file_store中插入相应记录
            <0 出现error，file_format无效
                SAMENAME 文件重名
    ***********************************************************/
    int upload_file(string path, string MD5, int isfolder, int    filesize,int block_num);

    /**********************************************************
    func:   删除一份文件，不删除物理内存
            更改两份表，删除file_format中的记录，更新file_store
    input:  string path 路径包含用户名（根目录
    output: void
    ***********************************************************/
    void delete_file(string path);



    /**********************************************************
    func:   用户删除文件夹，更新file_format；
    input:  string path 路径包含用户名（根目录
    output: int 标志位
            _SUCCESS    在file_format中更新相应记录
            _ERROR      文件不存在或其他错误
    **********************************************************  */
    int delete_catalog(string path);

    /**********************************************************
    func:   用户移动一份文件，更新file_format
    input:  string from_path 移动前的路径名，含文件名，含用户名
            string to_path 移动后的路径名，含文件名，含用户名
            string username 用户名
    output: int 标志位
            _SUCCESS    在file_format中更新相应记录
            <0 出现error，file_format无效
                SAMENAME 文件重名
    ***********************************************************/
    int move_file(string from_path,string to_path,string username);

    /**********************************************************
    func:   用户移动文件夹，更新file_format；
            若影响到file_store（文件存储目录以第一份上传为准），    应同步更新
    input:  string from_path 移动前的路径名，含文件名，含用户名
            string to_path 移动后的路径名，含文件名，含用户名
            string username 用户名
    output: int 标志位
            _SUCCESS    在file_format中更新相应记录
            <0 出现error，file_format无效
                SAMENAME 文件重名
    **********************************************************  */
    int move_catalog(string from_path,string to_path, string username);


    /**********************************************************
    func:   复制文件，在file_format中添加一条表记录
                    file_format中相应文件的用户占有数++
    input:  string username
            string from_path 移动前的路径名，含文件名，含用户名
            string to_path 移动后的路径名，含文件名，含用户名
    output: int
            重名返回SAMENAME
            失败返回_ERROR
            不重名且复制成功返回_SUCCESS
    ***********************************************************/
    int copy_file(string username,string from_path,   string to_path);

    /**********************************************************
    func:   复制文件夹，递归查找在file_format中添加若干记录
    input:  string username
            string from_path 移动前的路径名，含文件名，含用户名
            string to_path 移动后的路径名，含文件名，含用户名
    output: int
            重名返回SAMENAME
            失败返回_ERROR
            不重名且复制成功返回_SUCCESS
    **********************************************************  */
    int copy_catalog(string username,string from_path,    string to_path);


    /**********************************************************
    func:   用户上传一份文件，根据其在file_bloak中是否还有未上传完成的块，来判断该文件的上传是否是断点续传，若是，删  除之前在file_format           中建立的文件表记录
    input:  string path 文件路径（含文件名，含用户名
            string 文件MD5值
    output: int 标志位
                FINISHED    文件已经被上传完成/新文件
                CONTINUE     文件尚未被上传完成，删除相应的表记录
    **********************************************************  */
    int if_finished(string path,string MD5);




    /**********************************************************
    func:   根据路径名得到一个file_format结构体；
    input:  string path 文件路径，含文件名
            char *MD5 文件MD5值
            int isfolder 是否是文件夹
    output: file_format
            有效值   username
                    filename
                    filepath
                    parentid
                    MD5
                    isfolder
                    create_date
    ***********************************************************/
    file_format path_to_format(string path,string MD5,int   isfolder);


    /*************************块操作***************************/

    /**********************************************************
    func:   初始化一个文件的块，将文件的所有快都插入到file_block中
            块编号从1开始
    input:  string MD5 文件的唯一标识
            int block_num 文件的块数
    output: int 成功插入返回_SUCCESS
    ***********************************************************/
    int init_block(string MD5,int block_num);
};






//////////////////////////////////////////////////////////////
/**********************************************************
func:   根据路径名得到一个file_format结构体；
input:  string path 文件路径
        char *MD5 文件MD5值
        int isfolder 是否是文件夹
output: file_format
***********************************************************/
file_format path_to_format(string path,string MD5,int isfolder);

/******************************************************
func:   将密码进行MD5加密
input:  string pwd
        char* finalpwd
output: 
*******************************************************/
void MD5_code(string pwd,char *finalpwd);