#include "server.h"
using namespace std;

/**********
������Ӧ����
socket  Ŀ��socket
code    ״̬��
Status  ���ݿ�����ķ���ֵ
body    ��Ӧ���ĵ����ݣ�����������bodyΪNULL
**********/
int SendResponse(int socket, int code, int Status, char *body, char *ip)
{
    char response[BUFSIZE];
    // ��������headers���ͻ���
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
���ûظ����ĵ���������
Total       �ܰ���
Uploaded    �����������а���
No          �¸�������ţ����Ѵ����������0
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
read��ʽ��ȡһ����
fd  �ļ�������
linebuf ���
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
�洢��ǰ��
ChunkNo         Chunk��
DataLen         �յ������ݵĳ���
file_data       �ļ����ݵ���ʼ��ַ����������ռ䣬��Ҫ�ͷ�
file_path       �ļ�·��
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
������Ŀ¼
path            ����Ŀ¼�ĸ�Ŀ¼
dirName         ����Ŀ¼��
**********/
int CreateDir(const char *path, const char *dirName)
{

    //�������Ŀ¼�����ڣ�����
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
    
    //����Ŀ¼�����ڵĻ��ͽ����µ�
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
