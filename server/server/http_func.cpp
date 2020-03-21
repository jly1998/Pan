#include "server.h"
//#include "mysql_select.h"
using namespace std;

/**********
获取请求正文，返回请求内容的长度(>=0); 若小于0说明出错, body的值不可信
request     完整的请求
body        去除头部后的正文部分
ctype       该请求的contenttype
boundary    若请求为formdata，该值有效，为formdata的boundary
**********/
int GetRequestBody(int req_fd, char *&body, content_type &ctype, string &boundary, char *ip)
{
    char line_buf[LINEBUF];
    char *leftp;
    int len = -1, ret;
    string line;

    //contentlen
    char len_temp[] = "Content-Length: ";
    //contenttype
    char type_temp[] = "Content-Type: ";
    char ip_temp[] = "Origin: ";

    while (ret = (MyGetline(req_fd, line) == _SUCCESS))
    {
        int strl = line.copy(line_buf, LINEBUF - 1);
        line_buf[strl] = '\0';
        //content len
        if (len < 0)
        {
            leftp = strstr(line_buf, len_temp);
            if (leftp)
            {
                leftp += strlen(len_temp);

                len = atoi(leftp);

                if (len == 0)
                    return len;
            }
        }

        //contnet type
        if (len > 0)
        {
            leftp = strstr(line_buf, ip_temp);
            if (leftp)
            {
                leftp += strlen(ip_temp);
                char *rightp;
                rightp = strstr(leftp, "\n");
                *rightp = '\0';
                strcpy(ip, leftp);
                *rightp = '\n';
                //cout << ip << endl;
            }

            leftp = strstr(line_buf, type_temp);
            if (leftp)
            {
                leftp += strlen(type_temp);

                char json_temp[] = "application/x-www-form-urlencoded";
                char form_temp[] = "multipart/form-data";

                if (strstr(line_buf, json_temp))
                    ctype = json;
                else if (strstr(line_buf, form_temp))
                {
                    ctype = formdata;
                    char b_temp[] = "boundary=";
                    char *rightp = strstr(line_buf, b_temp);
                    rightp += strlen(b_temp);

                    boundary = rightp;
                }
                else
                {
                    ctype = other;
                    return -1;
                }
            }
        }

        if (!strcmp(line_buf, "\r\n"))
            break;
    } //while

    if (ret == _ERROR || len < 0)
        return -1;

    //内容
    body = new char[len];

    read(req_fd, body, len);

    return len;
} //GetRequestBody

/**********
处理json，返回枚举类型request_type
request_body    去除头部后的正文部分
data_begin      若类型正确，data_begin指向请求类型后的数据起始地址；若类型错误，该值不可信
**********/
request_type RequestType(char *request_body, char *&data_begin, int &ContentLen)
{
    char temp[] = "\"type\":\"";
    char *leftp = strstr(request_body, temp);
    if (leftp == NULL)
        return Error;

    leftp += strlen(temp);
    char *rightp = strstr(leftp, "\"");
    if (rightp == NULL)
        return Error;

    char type[16];
    *rightp = '\0';
    memcpy(type, leftp, rightp - leftp + 1);
    *rightp = '\"';

    ContentLen -= rightp + 2 - data_begin;
    if (0 == memcmp(type, "signup", sizeof("signup")))
    {
        data_begin = rightp + 2;
        return SignUp;
    }
    if (0 == memcmp(type, "signin", sizeof("signin")))
    {
        data_begin = rightp + 2;
        return SignIn;
    }
    if (0 == memcmp(type, "upload_req", sizeof("upload_req")))
    {
        data_begin = rightp + 2;
        return Upload_req;
    }
    if (0 == memcmp(type, "upload_data", sizeof("upload_data")))
    {
        data_begin = rightp + 2;
        return Upload_data;
    }
    if (0 == memcmp(type, "getdir", sizeof("getdir")))
    {
        data_begin = rightp + 2;
        return GetDir;
    }
    if (0 == memcmp(type, "delete", sizeof("delete")))
    {
        data_begin = rightp + 2;
        return Delete;
    }
    if (0 == memcmp(type, "move", sizeof("move")))
    {
        data_begin = rightp + 2;
        return Move;
    }
    if (0 == memcmp(type, "copy", sizeof("copy")))
    {
        data_begin = rightp + 2;
        return Copy;
    }

    return Error;
} //RequestType

/**********
处理用户注册登录的数据，若返回值不是_SUCCESS则username passwd的值不可信
data_begin      请求类型后的数据起始地址
username        用户int名，需要在调用本函数前申请好长度为USERLEN+1的空间
passwd          密码，需要在调用本函数前申请好长度为PSWDLEN+1的空间
**********/
int GetUserPasswd(char *data_begin, char *username, char *passwd)
{
    char user_temp[] = "\"username\":\"";
    char pswd_temp[] = "\"passwd\":\"";
    //用户名username
    char *leftp = strstr(data_begin, user_temp);
    if (leftp == NULL)
        return _ERROR;
    leftp += strlen(user_temp);
    char *rightp = strstr(leftp, "\"");
    if (rightp == NULL)
        return _ERROR;

    //超过长度限制
    if (rightp - leftp > USER_LEN)
    {
        return _WRONGDATA;
    }
    *rightp = '\0';
    memcpy(username, leftp, rightp - leftp + 1);
    *rightp = '\"';

    //密码passwd
    leftp = strstr(data_begin, pswd_temp);
    if (leftp == NULL)
        return _ERROR;
    leftp += strlen(pswd_temp);
    rightp = strstr(leftp, "\"");
    if (rightp == NULL)
        return _ERROR;

    //超过长度限制
    if (rightp - leftp > PSWD_LEN)
    {
        return _WRONGDATA;
    }
    *rightp = '\0';
    memcpy(passwd, leftp, rightp - leftp + 1);
    *rightp = '\"';

    return _SUCCESS;
} //GetUserPasswd

/**********
处理待传输文件的详细信息，返回路径id，若返回_ERROR，则file_info结果不可信
data_begin      指向请求类型后的数据起始地址
file_info       文件信息
**********/
int GetFileformat(char *&data_begin, file_format &file_info)
{
    char *leftp, *rightp;
    char file_temp[] = "\"file\":{";
    //file字段
    leftp = strstr(data_begin, file_temp);
    if (leftp == NULL)
        return _ERROR;
    data_begin = leftp + strlen(file_temp);

    //文件名filename
    char name_temp[] = "\"fileName\":\"";
    leftp = strstr(data_begin, name_temp);
    if (leftp == NULL)
        return _ERROR;
    leftp += strlen(name_temp);
    rightp = strstr(leftp, "\"");
    if (rightp == NULL)
        return _ERROR;

    *rightp = '\0';
    file_info.filename = leftp;
    *rightp = '\"';

    //文件大小filesize
    char size_temp[] = "\"fileSize\":";
    char size[8];
    leftp = strstr(data_begin, size_temp);
    if (leftp == NULL)
        return _ERROR;
    leftp += strlen(size_temp);
    rightp = strstr(leftp, ",");
    if (rightp == NULL)
        return _ERROR;

    *rightp = '\0';
    memcpy(size, leftp, rightp - leftp + 1);
    *rightp = ',';

    file_info.filesize = atoi(size);

    //文件md5
    char md5_temp[] = "\"fileMd5\":\"";
    leftp = strstr(data_begin, md5_temp);
    if (leftp == NULL)
        return _ERROR;
    leftp += strlen(md5_temp);
    rightp = strstr(leftp, "\"");
    if (rightp == NULL)
        return _ERROR;

    *rightp = '\0';
    file_info.MD5 = leftp;
    *rightp = '\"';

    //文件块数fileChunks
    char num_temp[] = "\"fileChunks\":";
    leftp = strstr(data_begin, num_temp);
    if (leftp == NULL)
        return _ERROR;
    leftp += strlen(num_temp);
    rightp = strstr(leftp, ",");
    if (rightp == NULL)
        return _ERROR;

    *rightp = '\0';
    memcpy(size, leftp, rightp - leftp + 1);
    *rightp = ',';

    file_info.block_num = atoi(size);

    /*****
    //文件路径
    char path_temp[] = "\"filePath\":\"";
    leftp = strstr(rightp, path_temp);
    if (leftp == NULL)
        return _ERROR;
    leftp += strlen(path_temp);
    rightp = strstr(leftp, "\"");
    if (rightp == NULL)
        return _ERROR;

    *rightp = '\0';
    file_info.filepath = leftp;
    *rightp = '\"';

    //中间应有逗号
    rightp = strstr(leftp, ",");
    if (rightp == NULL)
        return _ERROR;
    *****/

    //文件路径
    char path_temp[] = "\"filePath\":";
    leftp = strstr(data_begin, path_temp);
    if (leftp == NULL)
        return _ERROR;
    leftp += strlen(path_temp);
    rightp = strstr(leftp, ",");
    if (rightp == NULL)
        return _ERROR;

    *rightp = '\0';
    memcpy(size, leftp, rightp - leftp + 1);
    *rightp = ',';

    int DirNum = atoi(size);

    //用户名
    char username_temp[] = "\"username\":\"";
    leftp = strstr(data_begin, username_temp);
    if (leftp == NULL)
        return _ERROR;
    leftp += strlen(username_temp);
    rightp = strstr(leftp, "\"");
    if (rightp == NULL)
        return _ERROR;

    *rightp = '\0';
    file_info.username = leftp;
    *rightp = '\"';

    //文件夹
    char dir_temp[] = "\"RelativePath\":\"";
    leftp = strstr(data_begin, dir_temp);
    //没有RelativePath一项，为文件
    if (leftp == NULL)
    {
        file_info.isfolder = false;
        file_info.filepath = "";
        data_begin = rightp + 3;
        return DirNum;
    }

    leftp += strlen(dir_temp);
    rightp = strstr(leftp, "\"");
    if (rightp == NULL)
        return _ERROR;

    *rightp = '\0';
    file_info.filepath = leftp;
    *rightp = '\"';

    file_info.isfolder = false;

    data_begin = rightp + 3;

    return DirNum;

} //GetFileformat

/**********
将目录编号转换为路径字符串
DirNum          目录在数据库中的编号
num             目录结构表的大小
Cur_Dir         用户当前目录结构
**********/
string TransPath(int DirNum, int num, file_format *Cur_Dir)
{
    string Path = "";
    if (DirNum == 0)
    {
        Path = "/";
        return Path;
    }
    int i;
    for (i = 1; i < num; i++)
    {
        if (Cur_Dir[i].fileid == DirNum)
        {
            if (!Cur_Dir[i].isfolder)
                return Path;

            Path = Cur_Dir[i].filepath;
            return Path;
        }
    }

    return Path;
} //TransPath

/**********
处理上传到服务器的文件数据，返回收到的块编号，若<0则错误
data_begin      请求类型后的数据起始地址
boundary        boundary
DataLen         收到的数据的长度
file_data       文件数据的起始地址，无需申请空间，需要释放
file_info       文件信息
**********/
int GetFileData(char *&data_begin, string boundary, int &DataLen, char *&file_data, file_format &file_info)
{
    char line_buf[LINEBUF];
    int No;
    char size[8];
    char *leftp, *rightp;

    //cout << boundary << endl;
    boundary.copy(line_buf, boundary.length());
    strcpy(line_buf + boundary.length(), "\r\nContent-Disposition: form-data; name=\"file_info\"\r\n");
    //cout << "---------\n" << data_begin << endl;
    GetFileformat(data_begin, file_info);

    cout << "md5: " << file_info.MD5 << endl;
    cout << "block_num:" << file_info.block_num << endl;

    boundary.copy(line_buf, boundary.length());
    line_buf[boundary.length()] = '\0';
    leftp = strstr(data_begin, line_buf);
    if (!leftp)
        return -1;

    strcpy(line_buf, "Content-Disposition: form-data; name=\"file_data\"; filename=\"");
    leftp = strstr(data_begin, line_buf);
    if (!leftp)
        return -1;
    leftp += strlen(line_buf);

    //chunkNo
    rightp = strstr(leftp, "?");
    if (!rightp)
        return -1;

    *rightp = '\0';
    memcpy(size, leftp, rightp - leftp + 1);
    *rightp = '?';

    No = atoi(size);

    cout << "ChunkNo:" << No << endl;
    //获取块长度
    leftp = rightp + 1;
    rightp = strstr(leftp, "\"");
    if (!rightp)
        return -1;

    *rightp = '\0';
    memcpy(size, leftp, rightp - leftp + 1);
    *rightp = '\"';

    DataLen = atoi(size);

    cout << "DataLen:" << DataLen << endl;
    //fileData
    strcpy(line_buf, "\r\n\r\n");
    leftp = strstr(rightp, line_buf);
    if (!leftp)
        return -1;

    leftp += strlen(line_buf);

    file_data = new char[DataLen];
    memcpy(file_data, leftp, DataLen);

    return No;
} //GetFileData

/**********
获取用户的目录，返回string为json字符串
file      数组的头指针
**********/
string GetFileTree(int num, file_format *file)
{
    string ret;
    ret = "[";
    char temp[32];

    int i;
    for (i = 1; i < num; i++)
    {
        ret += "{\"id\":\"";
        sprintf(temp, "%d", file[i].fileid);
        ret += temp;

        ret += "\",\"filename\":\"";
        ret += file[i].filename;

        ret += "\",\"filesize\":\"";
        if (file[i].filesize == -1)
            sprintf(temp, "-");
        else
            sprintf(temp, "%d", file[i].filesize);
        ret += temp;

        ret += "\",\"createdate\":\"";
        ret += file[i].create_date.tmp0;

        ret += "\"";
        //用户根目录下
        if (file[i].parentid != file[0].fileid)
        {
            ret += ",\"parentId\":\"";
            sprintf(temp, "%d", file[i].parentid);
            ret += temp;

            ret += "\"";
        }
        ret += "}";
        if (i != num - 1)
            ret += ",";
    }

    ret += "]";
    cout << "json:" << ret << endl;
    return ret;
} //GetFileTree

/**********
获取用户名，若返回值不是_SUCCESS则username的值不可信
data_begin      请求类型后的数据起始地址
username        用户名
**********/
int GetUsername(char *data_begin, string &username)
{
    char user_temp[] = "\"username\":\"";
    //用户名username
    char *leftp = strstr(data_begin, user_temp);
    if (leftp == NULL)
        return _ERROR;
    leftp += strlen(user_temp);
    char *rightp = strstr(leftp, "\"");
    if (rightp == NULL)
        return _ERROR;

    *rightp = '\0';
    username = leftp;
    *rightp = '\"';

    return _SUCCESS;
} //GetUsername

/**********
处理用户删除文件的数据，若返回值不是_SUCCESS则fileid的值不可信
data_begin      请求类型后的数据起始地址
fileid          文件id int型
**********/
int Getfileid(char *data_begin, int &fileid)
{
    char fileid_temp[] = "\"fileid\":\"";
    char temp[8];
    char *leftp, *rightp;
    //用户文件id
    leftp = strstr(data_begin, fileid_temp);
    if (leftp == NULL)
        return _ERROR;
    leftp += strlen(fileid_temp);
    rightp = strstr(leftp, "\"");
    if (rightp == NULL)
        return _ERROR;

    *rightp = '\0';
    memcpy(temp, leftp, rightp - leftp + 1);
    *rightp = '\"';

    fileid = atoi(temp);

    return _SUCCESS;
}

/**********
处理用户移动/复制文件/夹的数据，若返回值不是_SUCCESS则from_id，to_id的值不可信
data_begin      请求类型后的数据起始地址
from_id         待移动/删除的文件id
to_id           将移动/删除至的文件夹id
**********/
int Getfromtoid(char *data_begin, int &from_id, int &to_id)
{
    char temp[8];
    char *leftp, *rightp;

    //起始位置 文件id
    char fromid_temp[] = "\"from_id\":\"";
    leftp = strstr(data_begin, fromid_temp);
    if (leftp == NULL)
        return _ERROR;
    leftp += strlen(fromid_temp);
    rightp = strstr(leftp, "\"");
    if (rightp == NULL)
        return _ERROR;

    *rightp = '\0';
    memcpy(temp, leftp, rightp - leftp + 1);
    *rightp = '\"';

    from_id = atoi(temp);

    //目标位置 文件夹id
    char toid_temp[] = "\"to_id\":\"";
    leftp = strstr(rightp, toid_temp);
    if (leftp == NULL)
        return _ERROR;
    leftp += strlen(toid_temp);
    rightp = strstr(leftp, "\"");
    if (rightp == NULL)
        return _ERROR;

    *rightp = '\0';
    memcpy(temp, leftp, rightp - leftp + 1);
    *rightp = '\"';

    to_id = atoi(temp);

    return _SUCCESS;
}