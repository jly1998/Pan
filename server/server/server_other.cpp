#include "server.h"
using namespace std;

/**********
发送响应报文
socket  目的socket
code    状态码
Status  数据库操作的返回值
body    响应报文的内容，若无内容则body为NULL
**********/
int SendResponse(int socket, int code, int Status, char *body, char *ip)
{
    char response[BUFSIZE];
    // 发送请求headers到客户端
    switch (code)
    {
    case 200:
        switch (Status)
        {
        case _SUCCESS:
            sprintf(response, "HTTP/1.1 200 OK\r\n");
            break;
        case _WRONGDATA:
            sprintf(response, "HTTP/1.1 211 wrongdata\r\n");
            break;
        case _USERWRONG:
            sprintf(response, "HTTP/1.1 212 userwrong\r\n");
            break;
        case NOBLOCK:
            sprintf(response, "HTTP/1.1 200 NOBLOCK\r\n");
            break;
        default:
            sprintf(response, "HTTP/1.1 400 bad request\r\n");
            break;
        }
        break;
    case 400:
        sprintf(response, "HTTP/1.1 400 bad request\r\n");
        break;
    default:
        sprintf(response, "HTTP/1.1 400 bad request\r\n");
        break;
    }
    sprintf(response, "%sAccess-Control-Allow-Origin: %s\r\n", response,ip);
    sprintf(response, "%sContent-Type: application/x-www-form-urlencoded\r\n", response);
    sprintf(response, "%sContent-Length: %d\r\n", response, body ? strlen(body) : 0);
    sprintf(response, "%sServer: Pan Server\r\n\r\n", response);
    if (body)
        sprintf(response, "%s%s\r\n", response, body);

    cout << response;
    write(socket, response, strlen(response));

    return _SUCCESS;
} //SendResponse

/**********
设置回复报文的正文内容
Total       总包数
Uploaded    服务器中已有包数
No          下个包的序号，若已传输结束则置0
**********/
string SetResponseBody(int Total, int Uploaded, int No)
{
    string body;
    char temp[8];

    body = "{\"Total\":";
    sprintf(temp, "%d", Total);
    body += temp;
    body += ",\"Uploaded\":";
    sprintf(temp, "%d", Uploaded);
    body += temp;
    body += ",\"No\":";
    sprintf(temp, "%d", No);
    body += temp;
    body += "}";

    return body;
} //SetResponseBody

/**********
read方式获取一整行
fd  文件描述符
linebuf 结果
**********/
int MyGetline(int fd, string &linebuf)
{
    int len = 0;
    char temp[LINEBUF];

    for (len = 0; len < LINEBUF - 1; len++)
    {
        read(fd, temp + len, 1);
        if (temp[len] == '\0')
            return _ERROR;

        if (temp[len] == '\n')
        {
            temp[len + 1] = '\0';
            linebuf = temp;
            return _SUCCESS;
        }
    } //for
    temp[LINEBUF - 1] = '\0';
    linebuf = temp;
    return _SUCCESS;
} //MyGetline

/**********
存储当前块
ChunkNo         Chunk号
DataLen         收到的数据的长度
file_data       文件数据的起始地址，无需申请空间，需要释放
file_path       文件路径
**********/
int SaveFile(const int ChunkNo, const int DataLen, const char *file_data, const string file_path, const string file_name)
{
    int fd;
    char temp[8];

    sprintf(temp, ".%d", ChunkNo);

    char *path = new char[strlen(DataRoot) + file_path.length() + file_name.length() + strlen(temp) + 2];

    sprintf(path, "%s%s/%s%s", DataRoot, file_path.data(), file_name.data(), temp);

    fd = open(path, O_WRONLY | O_CREAT ,0666);
    if (fd < 0)
    {
        perror("Create file error");
        return _ERROR;
    }

    write(fd, file_data, DataLen);

    delete[] path;
    close(fd);
} //SaveFile

/**********
建立新目录
path            待建目录的根目录
dirName         待建目录名
**********/
int CreateDir(const char *path, const char *dirName)
{

    //如果数据目录不存在，错误
    if (access(path, F_OK) != 0)
    {
        return _WRONGDATA;
    }

    char *path_temp = new char[strlen(path) + strlen(dirName) + 2];
    if (!path_temp)
    {
        perror("malloc failed");
        return _ERROR;
    }

    strcpy(path_temp, path);
    strcat(path_temp, "/");
    strcat(path_temp, dirName);
    
    //待建目录不存在的话就建个新的
    if (access(path_temp, F_OK) != 0)
    {
        if (mkdir(path_temp, 0700) != 0)
        {
            perror("mkdir failed");
            return _ERROR;
        }
    }
    return _SUCCESS;
} //CreateDir
