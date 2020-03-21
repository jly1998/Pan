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

//Ĭ��Ŀ¼
#define PathResource    "myResource"
#define PathApp         "myApp"


//��ʾû����Ҫ�ϴ��Ŀ�
#define NOBLOCK -4
//��ʾ���п����ڱ��ϴ����ȴ����������������
#define WAITBLOCK -5
//��ʾ�ϴ����ļ�����
#define SAMENAME -6
//��ʾ���ļ����ƶ����������ļ�������
#define MOVETOSON -7
//��ʾ���ļ�/���ƶ�/���Ƶ��ļ���
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

//block���ͳ�ʱʱ��.ms
#define ALARM 10

struct NowDate
{
    char tmp0[16]; //������
    char tmp1[16]; //ʱ����
    char tmp2[4];  //����
};

//����ǰʱ��ת��ΪNowDate�ṹ��
NowDate getTime();

//��ú���������
long getCurrentTimeMsec();

class MyDB;

//��¼ÿ���ļ����ϴ����ȣ�ÿ�δӱ��в�ѯ�õ�һ����ţ����͸�web�ˣ����������е��û�ռ����Ϊtrue
//file_block
typedef struct file_block
{
    string MD5;
    int block_no;
    bool ifusing;
    long usingtime;
}file_block;

//Ϊÿ������һ���ṹ�壬���㸳ֵ��ʱ��
//user
typedef struct user
{
    int userid;
    string username;
    string pwd;
    NowDate regedit_date;
} user;

//ÿ���ļ���ʵ�ʴ洢�ռ�
//file_store
typedef struct file_store
{
    int memid;
    string path;//�ļ���ʵ�ʴ洢·��
    string MD5;
    int filesize;//�ļ����ܴ�С
    int usernum;//�ļ����û�������
} file_store;

//��ͻ��˹�ͨ���ݵĽṹ�壬������ȫȨ�������ݿ��еı�ṹ
//file_format
typedef struct file_format
{
    int fileid;//�ļ�Ψһ���
    string filename;//�ļ���
    string username;//�������û��� *from web
    NowDate create_date;//�������� 
    string MD5;//�ļ���MD5ֵ *from web
    int parentid;//��Ŀ¼��� 
    bool isfolder;//�ļ��б�ʶ *from web
    int filesize;//�ļ���С *from web
    string filepath;//�ļ�·���������û��� *from web
    int block_num;//���� *from web
    friend MyDB;
} file_format;

////////////////////////////////////////////////////////
class MyDB
{
public:
    friend file_format;
    MyDB();
    ~MyDB();
    bool initDB(string host, string user, string passwd, string db_name); //����mysql
    bool exeSQL(string sql);
    //ִ��sql���

    //�ر���MySQL������
    void closeSQL();
    //���select�Ľ��
    bool outselect();



    /********************�û�����//**********************************************************
    func:   �ж��û�ע���Ƿ���Գɹ�
            �����ʽ�����Ȳ�С��8������ͬʱ������ĸ/����/�����ַ������Ҳ��� ���зǷ��ַ�(<=32)
            ��װif_signup�����ͻ�����
    input:  string user
            string code
    output: int 
            �ɹ�����true��ʧ�ܷ��ش���ԭ��
            ʧ�ܵ�ԭ��_USEREXIST���û����Ѿ�����
                       _WRONGDATA���������ò����涨
    ***********************************************************/
    int signup(string user, string code);

    /**********************************************************
    func:   �ж��û���½�����Ƿ�Ϸ�
    input:  string user �û���
            string code
    output: int 
            �Ϸ����� _SUCCESS
            ���벻ƥ�䷵�� _WRONGDATA
            �û��������ڷ��� _USERWRONG
    ***********************************************************/
    int if_signin(string user, string code); //�ж��û���½�Ƿ�ɹ�

    /********************�ļ�����**************************************/
    
    /**********************************************************
    func:   �û��ϴ�һ���ļ�������file_format��
            �൱�ڷ�װ��upload_file
    input:  file_format ���Կͻ��˵Ľṹ�壬�������Լ��
    output: ��upload_file�ķ���ֵ��ͬ
            int ��־λ
            _SUCCESS    ��file_format�в�����Ӧ��¼
                        ��file_store�в�����Ӧ��¼
            <0 ����error��file_format��Ч
                SAMENAME �ļ�����
    ***********************************************************/
    int sealed_upload(file_format file);

    /**********************************************************
    func:   ɾ��һ���ļ�����ɾ�������ڴ�
            �������ݱ�ɾ��file_format�еļ�¼������file_store
            �൱�ڷ�װ��delete_file
    input:  string path ·�������û�������Ŀ¼
    output: void
    ***********************************************************/
    void sealed_delete(file_format file);

    /**********************************************************
    func:   �ƶ��ļ�/��
            �൱�ڷ�װ��move_file��move_catalog
    input:  int from_id ��ʼ�ļ�id
            int to_id ��ֹ·�����ļ���id
                    ��Ҫ�ڲ��Լ�ת��Ϊ�ṹ��
    output: void
    *********************************************************      */
    int sealed_move(int from_id,int to_id);

     /**********************************************************
     func:   �����ļ�/��
             �൱�ڷ�װ��move_file��move_catalog
             ͬ�ļ����¸��ƻ�׷�����������ļ���
     input:  int from_id ��ʼ�ļ�id
             int to_id ��ֹ·�����ļ���id
                     ��Ҫ�ڲ��Լ�ת��Ϊ�ṹ��
     output: �ļ�/�и��ƽ��
     **********************************************************     */
     int sealed_copy(int from_id,int to_id);


    




    /**********************************************************
    func:   �û�����һ��·����ͨ����ַȥ��ѯ���ݿ⣬���ظ��û�  �ò��ṹ
    input:  file_format ���Կͻ��˵Ľṹ�壬�������Լ��
            ��Чֵ   filepath
                    username
            file_format ���飬����Ŀ¼�µ��ļ��ṹ
    output: int ���ز�ѯ���Ľ��������
    ******************************************************  *****/
    int find_content(const file_format file);

    /**********************************************************
    func:   �˺�����find_content(const file_format file)��  ��ʹ��,
            ��ʱresult���Ѵ洢�˲�ѯ���
    input:  file_format* ���飬����Ŀ¼�µ��ļ��ṹ���ɸú���   д��  
    output: int ���ز�ѯ���Ľ�������������ڶ�̬����    file_format����Ĵ�С
    ******************************************************  *****/
    void store_format(file_format * file);

    /**********************************************************
    func:   ��ѯ���������Ƿ���Գɹ������ͻ�����
    input:  file_format 
            ��Чֵ  username
                    filepath
            string newname
    output: int ���ز������
                _SUCCESS �ɹ�������������file_format����Ӧ��    ���¼
                SAMENAME �����������⣬������ʧ��
    ******************************************************  *****/
    int ifrename(file_format file,string newname);


    /********************�����**************************************/

    /**********************************************************
    func:   �鿴ĳ�ļ��Ƿ���Ϊδ�ϴ��飬�ͻ��˵���
            ÿ�β鿴������п鱻ռ�ã�ȥ��ռ��ʱ�䣬��ʱ��ÿ�Ҳ  �ڿ����б�
    input:  string MD5 �ļ���Ψһ��ʶ
    output: int 
            ���ļ�����δ�ϴ��飬����ֵΪ��һ����Ҫ�ϴ��Ŀ�ţ�����  ��ifusingΪISUSING
            ���� ���ļ����п��Ѿ����ϴ�����������NOBLOCK
                 ���ļ������п�δ���ϴ��������Ƕ����ڱ��ϴ�������WAITBLOAK���ȴ�һ��ʱ���������ʹ�������ֱ���ļ��ϴ��������Զ����˳��ϴ���
    **********************************************************  */
    int next_block(string MD5);

    /**********************************************************
    func:   �û��ϴ�һ����ɹ���ɾ��file_block�еĿ��¼���ͻ��˵���
    input:  file_block block �ṹ��
                string MD5 �ļ���Ψһ��ʶ��
                int block_no ���
    output: int 
            ɾ���ɹ�����_SUCCESS
            ɾ��ʧ�ܷ���_ERROR
    ***********************************************************/
    int upblock_success(file_block block);

    /**********************************************************
    func:   �û��ϴ�һ����ʧ�ܣ�����file_block�еĿ��¼�����û�ռ��λ��    �գ��ͻ��˵���
    input:  file_block block �ṹ��
                string MD5 �ļ���Ψһ��ʶ��
                int block_no ���
    output: int 
            ���óɹ�����_SUCCESS
            ����ʧ�ܷ���_ERROR
    ***********************************************************/
    int upblock_failed(file_block block);

    /**********************************************************
    func:   ���ص�ǰ�ļ��Ѿ�����Ŀ��������ͻ���
    input:      string MD5 �ļ���Ψһ��ʶ��
                int block_num �ļ�һ���Ŀ���
    output: int 
            ������ɵĿ���
    ***********************************************************/
    int finished_block(string MD5,int block_num);

    /********************��������**************************************/
    void getresult(string **temp_result);

    /**********************************************************
    func:   ����MD5ֵ�����ظ��ͻ��ļ�ʵ�ʴ洢·�������û������൱   �����ݿ��д洢·����
            ���ͻ�����
    input:  string MD5
            string &filename "newfile"
            string &parentpath "/fenhui/jisuanji"
    output: int
            ����ѯʧ�ܣ�����_ERROR
            ����ѯ�ɹ�������_SUCCESS
    **********************************************************  */
    int getroad(string MD5,string &filename,string &   parentpath);

    /**********************************************************
    func:   �����û������õ����û���Ŀ¼�ṹ������
            ���ͻ�����
    input:  string username
            file_format &*file �ṹ����������ã�
    output: int
            ��ѯ�ɹ����ز�ѯ��������
            ��ѯʧ�ܷ���_ERROR
    **********************************************************  */
    int user_filetree(string username,file_format *&   file);

private:
    MYSQL *mysql;      //����mysql���ָ��
    MYSQL_RES *result; //ָ���ѯ�����ָ��
    MYSQL_ROW row;     //���з��صĲ�ѯ��Ϣ



    /********************�û�����**************************************/

    
    /**********************************************************
    func:   ��ѯs�Ƿ��Ѿ���table�У�sΪ��ѡ��
    input:  string table 
            string s
    output: bool
            �Ѿ����ڷ���false�����򷵻�true
    ***********************************************************/
    bool if_insert(string table, string s);

    /**********************************************************
    func:   �ж��û�ע���Ƿ���Գɹ�
            �����ʽ�����Ȳ�С��8������ͬʱ������ĸ/����/�����ַ������Ҳ��� ���зǷ��ַ�(<=32)
    input:  string user
            string code
    output: int 
            �ɹ�����true��ʧ�ܷ��ش���ԭ��
            ʧ�ܵ�ԭ��_USEREXIST���û����Ѿ�����
                       _WRONGDATA���������ò����涨
    ***********************************************************/
    int if_signup(string user, string code); //�ж��û�ע���Ƿ�ɹ�



    /********************�ļ�����**************************************/
    /**********************************************************
    func:   Ϊÿһ���½��û�����һ��Ĭ���ļ�Ŀ¼
            ��ʱ��δ���ļ��������Ĳ���ֻ����file_format����������ļ���
    input:  string user
    output: int
            �����ɹ�����_SUCCESS
            ����ʧ�ܷ���_ERROR
    ***********************************************************/
    int init_catalog(string user);

    /**********************************************************
    func:   �û��ϴ�һ���ļ������Ȳ�ѯfile_store��
            ������У������û�����������file_store�в�����Ӧ���¼
    input:  string path �ļ�·��
            string MD5 �ļ�MD5ֵ
            int filesize �ļ���С
    output: bool
            ���з���true�����򷵻�false
    ***********************************************************/
    bool ifexist_store(string path,string MD5, int filesize);

   /**********************************************************
    func:   ͬ�û�ͬ�ļ������Ƿ���������
    input:  struct file_format
            ��Чֵ  filepath
    output: bool
            ����������false������������true
    **********************************************************  */
    bool ifsame_filename(file_format file);
    /*********************************************************
    func:   ����·�����õ�һ���ļ��� ��ţ�
    input:  string path
    output: int parentid
    **********************************************************/
    int path_to_id(string path);

    /******************************************************* ***
    func:   ���ݱ�Ŵ����ݿ��в�ѯ��һ��file_format����
    input:  int fileid
    output: file_format
    ******************************************************* ****/
    file_format id_to_format(int fileid);
 

    /*********************************************************
    func:   �ж��ļ�2�ǲ����ļ�1�����ļ�
    input:  string id1
            string id2
    output: bool
            ����Ƿ���true
            ���Ƿ���false
    *********************************************************             */
    bool i_am_your_father(string id1,string id2);



    /**********************************************************
    func:   �ļ������������������⣬׷��������ʾ����
    input:  string &filepath
    output: void
    **********************************************************    */
    void newfilename_deal(string &filepath);

    /**********************************************************
    func:   �û��ϴ�һ���ļ�������file_format,file_store
    input:  string path �ļ�·�������ļ���
            char *MD5 �ļ�MD5ֵ
            int isfolder ��־λ���Ƿ����ļ���
            int filesize �ļ���С
            int block_num ����
    output: int ��־λ
            _SUCCESS    ��file_format�в�����Ӧ��¼
                        ��file_store�в�����Ӧ��¼
            <0 ����error��file_format��Ч
                SAMENAME �ļ�����
    ***********************************************************/
    int upload_file(string path, string MD5, int isfolder, int    filesize,int block_num);

    /**********************************************************
    func:   ɾ��һ���ļ�����ɾ�������ڴ�
            �������ݱ�ɾ��file_format�еļ�¼������file_store
    input:  string path ·�������û�������Ŀ¼
    output: void
    ***********************************************************/
    void delete_file(string path);



    /**********************************************************
    func:   �û�ɾ���ļ��У�����file_format��
    input:  string path ·�������û�������Ŀ¼
    output: int ��־λ
            _SUCCESS    ��file_format�и�����Ӧ��¼
            _ERROR      �ļ������ڻ���������
    **********************************************************  */
    int delete_catalog(string path);

    /**********************************************************
    func:   �û��ƶ�һ���ļ�������file_format
    input:  string from_path �ƶ�ǰ��·���������ļ��������û���
            string to_path �ƶ����·���������ļ��������û���
            string username �û���
    output: int ��־λ
            _SUCCESS    ��file_format�и�����Ӧ��¼
            <0 ����error��file_format��Ч
                SAMENAME �ļ�����
    ***********************************************************/
    int move_file(string from_path,string to_path,string username);

    /**********************************************************
    func:   �û��ƶ��ļ��У�����file_format��
            ��Ӱ�쵽file_store���ļ��洢Ŀ¼�Ե�һ���ϴ�Ϊ׼����    Ӧͬ������
    input:  string from_path �ƶ�ǰ��·���������ļ��������û���
            string to_path �ƶ����·���������ļ��������û���
            string username �û���
    output: int ��־λ
            _SUCCESS    ��file_format�и�����Ӧ��¼
            <0 ����error��file_format��Ч
                SAMENAME �ļ�����
    **********************************************************  */
    int move_catalog(string from_path,string to_path, string username);


    /**********************************************************
    func:   �����ļ�����file_format�����һ�����¼
                    file_format����Ӧ�ļ����û�ռ����++
    input:  string username
            string from_path �ƶ�ǰ��·���������ļ��������û���
            string to_path �ƶ����·���������ļ��������û���
    output: int
            ��������SAMENAME
            ʧ�ܷ���_ERROR
            �������Ҹ��Ƴɹ�����_SUCCESS
    ***********************************************************/
    int copy_file(string username,string from_path,   string to_path);

    /**********************************************************
    func:   �����ļ��У��ݹ������file_format��������ɼ�¼
    input:  string username
            string from_path �ƶ�ǰ��·���������ļ��������û���
            string to_path �ƶ����·���������ļ��������û���
    output: int
            ��������SAMENAME
            ʧ�ܷ���_ERROR
            �������Ҹ��Ƴɹ�����_SUCCESS
    **********************************************************  */
    int copy_catalog(string username,string from_path,    string to_path);


    /**********************************************************
    func:   �û��ϴ�һ���ļ�����������file_bloak���Ƿ���δ�ϴ���ɵĿ飬���жϸ��ļ����ϴ��Ƿ��Ƕϵ����������ǣ�ɾ  ��֮ǰ��file_format           �н������ļ����¼
    input:  string path �ļ�·�������ļ��������û���
            string �ļ�MD5ֵ
    output: int ��־λ
                FINISHED    �ļ��Ѿ����ϴ����/���ļ�
                CONTINUE     �ļ���δ���ϴ���ɣ�ɾ����Ӧ�ı��¼
    **********************************************************  */
    int if_finished(string path,string MD5);




    /**********************************************************
    func:   ����·�����õ�һ��file_format�ṹ�壻
    input:  string path �ļ�·�������ļ���
            char *MD5 �ļ�MD5ֵ
            int isfolder �Ƿ����ļ���
    output: file_format
            ��Чֵ   username
                    filename
                    filepath
                    parentid
                    MD5
                    isfolder
                    create_date
    ***********************************************************/
    file_format path_to_format(string path,string MD5,int   isfolder);


    /*************************�����***************************/

    /**********************************************************
    func:   ��ʼ��һ���ļ��Ŀ飬���ļ������п춼���뵽file_block��
            ���Ŵ�1��ʼ
    input:  string MD5 �ļ���Ψһ��ʶ
            int block_num �ļ��Ŀ���
    output: int �ɹ����뷵��_SUCCESS
    ***********************************************************/
    int init_block(string MD5,int block_num);
};






//////////////////////////////////////////////////////////////
/**********************************************************
func:   ����·�����õ�һ��file_format�ṹ�壻
input:  string path �ļ�·��
        char *MD5 �ļ�MD5ֵ
        int isfolder �Ƿ����ļ���
output: file_format
***********************************************************/
file_format path_to_format(string path,string MD5,int isfolder);

/******************************************************
func:   ���������MD5����
input:  string pwd
        char* finalpwd
output: 
*******************************************************/
void MD5_code(string pwd,char *finalpwd);