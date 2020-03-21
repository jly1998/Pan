#include "server.h"
//#include "mysql_select.h"
using namespace std;

MyDB db;

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        cout << "-----������������-----" << endl;
        cout << "�밴[��ִ���ļ��� �˿ں�]��ʽ����" << endl;
        return 1;
    }
    pid_t pid = fork();
    if (pid < 0)
    {
        perror("fork error");
        return 1;
    }
    else if (pid == 0) //�ӽ���
    {
        //�������Ŀ¼�����ڣ���һ��
        if (access(DataRoot, F_OK) != 0)
        {
            if (mkdir(DataRoot, 0700) != 0)
            {
                perror("mkdir failed");
                exit(-1);
            }
        }

        Disk_server(argv[1]);
    }
    else //�����̽���
    {
        return 0;
    }

    return 0;
} //main

/**********
���̿ͻ���������
_port   �˿ںţ��ַ���
**********/
int Disk_server(char *_port)
{
    //����socket
    int listen_sock = S_CreateListenSock(_port);
    int Maxfd = listen_sock;
    cout << "------------------------------" << endl;

    fd_set rfds, wfds;
    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 0;
    while (1)
    {
        FD_ZERO(&rfds);

        FD_SET(listen_sock, &rfds);

        int flag = select(listen_sock + 1, &rfds, NULL, NULL, &timeout);
        if (flag < 0)
        {
            perror("select error");
            continue;
        }
        if (flag > 0)
        {
            if (FD_ISSET(listen_sock, &rfds))
            {
                printf("found new request from client\n");

                pid_t pid = fork();
                if (pid < 0)
                {
                    perror("fork error");
                    return 1;
                }
                else if (pid == 0) //�ӽ���
                {
                    S_AcConncs(listen_sock, Maxfd);
                    return 0;
                } //sub-process
            }     //rfd is set
        }         //select����>0
    }             //while
    close(listen_sock);
    return 0;
} //S_Listen

/**********
��������socket������������
_port   �˿ںţ��ַ���
**********/
int S_CreateListenSock(char *_port)
{
    int flag = 1;
    //�����׽���
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0)
    {
        perror("create socket failed");
        exit(-1);
    }
    //	printf("creat socket success\n");

    //���÷�����
    SetNonBlock(sock);
    //"listen socket"

    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(int)) < 0)
    {
        printf("set socket option error");
        exit(-1);
    }

    //��
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;                //����ΪIPͨ��
    server_addr.sin_addr.s_addr = htons(INADDR_ANY); //������IP��ַ--�������ӵ����б��ص�ַ��
    server_addr.sin_port = htons(atoi(_port));       //�������˿ں�

    int len = sizeof(server_addr);
    if (bind(sock, (struct sockaddr *)&server_addr, (socklen_t)len) < 0)
    {
        perror("bind error!");
        exit(-1);
    }
    //	printf("bind success\n");

    //����
    if (listen(sock, _BACKLOG_) < 0)
    {
        perror("listen");
        exit(-1);
    }
    printf("Listen status, waiting for connection request...\n");
    return sock;
} //S_CreateListenSock

/**********
��������
listen_sock     ����socket
Maxfd           ��ǰ���socket
**********/
int S_AcConncs(int listen_sock, int &Maxfd)
{
    //���Ӳ�����Ϊ������
    struct sockaddr_in connc;
    int connc_len, ret;
    int CurSock = accept(listen_sock, (struct sockaddr *)&connc, (socklen_t *)&connc_len);

    SetNonBlock(CurSock);
    if (CurSock == -1)
    {
        perror("accept error");
        return _ERROR;
    }
    if (CurSock > Maxfd)
        Maxfd = CurSock;
    close(listen_sock);
    //��Ϣ
    struct sockaddr_in peer;
    socklen_t len = sizeof(peer);
    if (!getpeername(CurSock, (struct sockaddr *)&peer, &len))
    {
        printf("Accept connection from %s:%d\n", inet_ntoa(peer.sin_addr), ntohs(peer.sin_port));
    }

    fd_set rfds, wfds;
    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 50000;

    char reqfile[32];
    sprintf(reqfile, "%d%s", getpid(), ReqFileName);
    int req_fd = open(reqfile, O_WRONLY | O_CREAT, 0666);
    if (req_fd < 0)
    {
        perror("create file error");
        return _ERROR;
    }

    int select_flag;
    while (1)
    {
        FD_ZERO(&rfds);
        FD_SET(CurSock, &rfds);
        select_flag = select(Maxfd + 1, &rfds, NULL, NULL, &timeout);
        if (select_flag < 0)
        {
            perror("select error");
            return _ERROR;
        }

        //�ɶ�
        if (FD_ISSET(CurSock, &rfds))
        {
            char request_buf[BUFSIZE] = {0};
            size_t rset;
            rset = read(CurSock, request_buf, BUFSIZE);
            cout << "socket read" << rset << endl;
            if (rset == 0)
            {
                cout << "Connection disconnected!" << endl;
                return _ERROR;
            }
            if (rset < 0 && !(errno == EINTR || errno == EWOULDBLOCK || errno == EAGAIN))
            {
                perror("read failed");
                return _ERROR;
            }

            //����������
            write(req_fd, request_buf, rset);

        } //�ɶ�
        else
        {
            cout << "socket not readable" << endl;
            break;
        }

    } //while

    close(req_fd);
    //����request
    string ResponseBody;
    char ip[64];
    ret = DealRequest(ResponseBody, ip);
    cout << ip << endl;
    while (1)
    {
        FD_ZERO(&wfds);
        FD_SET(CurSock, &wfds);
        select_flag = select(Maxfd + 1, NULL, &wfds, NULL, &timeout);
        if (select_flag < 0)
        {
            perror("select error");
            return _ERROR;
        }
        //��д
        if (select_flag > 0 && FD_ISSET(CurSock, &wfds))
        {
            cout << "ret" << ret << endl;
            //    cout << "operate_ret" << operate_ret << endl;
            //          ��������ֵ�ǶԵ�    �ɹ�����                        ��������
            int code = (ret == _ERROR) ? 400 : 200;

            //char *content = NULL;
            char content[LINEBUF];
            ResponseBody.copy(content, LINEBUF - 1);
            content[ResponseBody.length()] = '\0';
            //char test[] = "{\"Total\":1,\"Uploaded\":0,\"No\":1}";
            SendResponse(CurSock, code, ret, ret == _SUCCESS || ret == NOBLOCK ? content : NULL, ip);
            break;
        } //��д
    }     //while

    //remove(reqfile);
    close(CurSock);
    return _SUCCESS;
} //S_AcConncs

/**********
��������
responseBody     �ظ�����
**********/
int DealRequest(string &responseBody, char *ip)
{
    responseBody = "";
    char reqfile[32];
    sprintf(reqfile, "%d%s", getpid(), ReqFileName);
    int req_fd;
    req_fd = open(reqfile, O_RDONLY);
    if (req_fd < 0)
    {
        perror("deal file error");
        return _ERROR;
    }

    int database_ret, ret, DataLen;
    char *FileData;
    /*
    //��������
    cout << "---------------receive---------------" << endl;
    cout << request_buf << endl;
    cout << "------------------------------" << endl;
*/
    //����body
    char *request_body;
    content_type ctype;
    string boundary;
    //char ip[64];
    int ContentLength = GetRequestBody(req_fd, request_body, ctype, boundary, ip);

    if (ContentLength < 0)
        return _ERROR;
    if (ContentLength == 0)
        return _WRONGDATA;
    cout << "---------------request body---------------" << endl;
    cout << "content type:" << ctype << endl;
    cout << "length:" << ContentLength << endl;
    cout << "ip:" << ip << endl;
    // cout << request_body << endl;
    cout << "------------------------------" << endl;

    char *data_begin;

    //�ж���������
    request_type req_type;
    if (ctype == json)
        req_type = RequestType(request_body, data_begin, ContentLength);
    else if (ctype == formdata)
    {
        req_type = Upload_data;
        data_begin = request_body;
    }
    else
    {
        req_type = Error;
    }

    if (req_type == Error)
        return _ERROR;
    //cout << "type:" << req_type << endl;

    file_format file_info;
    char username[USER_LEN + 1], passwd[PSWD_LEN + 1];
    char pathName[LINEBUF];
    int BlockNo, Uploaded;
    int fromid, toid;
    file_block file_block;
    string u;
    switch (req_type)
    {
    case SignUp: //ע��
        ret = GetUserPasswd(data_begin, username, passwd);
        if (ret == _SUCCESS)
        {
            u = username;
            string p = passwd;

            //�������ݿ�
            db.initDB("localhost", USER, DBPASSWD, DBNAME);
            database_ret = db.signup(u, p);

            if (database_ret == _SUCCESS)
            {
                cout << "username:" << username << endl;
                cout << "passwd:  " << passwd << endl;
            }
            db.closeSQL();
            CreateDir(DataRoot, username);
            strcpy(pathName, DataRoot);
            strcat(pathName, "/");
            strcat(pathName, username);
            CreateDir(pathName, PathResource);
            CreateDir(pathName, PathApp);
        }
        break;
    case SignIn: //��¼
        ret = GetUserPasswd(data_begin, username, passwd);
        if (ret == _SUCCESS)
        {
            u = username;
            string p = passwd;

            //�������ݿ�
            db.initDB("localhost", USER, DBPASSWD, DBNAME);
            database_ret = db.if_signin(u, p);

            if (database_ret == _SUCCESS)
            {
                cout << "username:" << username << endl;
                cout << "passwd:  " << passwd << endl;
            }

            db.closeSQL();
        }
        break;
    case Upload_req: //�����ļ�ǰ������
        ret = GetFileformat(data_begin, file_info);
        if (ret >= 0)
        {
            file_format *filetree;
            int filenum;
            string RelaP;
            if (file_info.filepath.length() != 0)
            {
                RelaP = file_info.filepath;
                cout << "relap:" << file_info.filepath << endl;
            }

            //�������ݿ�
            db.initDB("localhost", USER, DBPASSWD, DBNAME);

            filenum = db.user_filetree(file_info.username, filetree);
            cout << "filenum" << filenum << endl;

            //�ļ��п�ֱ��ʹ��relativepath
            if (file_info.filepath.length() != 0)
            {

                file_info.filepath = TransPath(ret, filenum, filetree);
                //ת������
                if (file_info.filepath.length() == 0)
                {
                    db.closeSQL();
                    database_ret = _WRONGDATA;
                }
                CreateRelativePath(file_info.filepath, RelaP, file_info.username);

                file_info.filepath += "/" + RelaP;
            }
            else
            {
                file_info.filepath = TransPath(ret, filenum, filetree);
                //ת������
                if (file_info.filepath.length() == 0)
                {
                    db.closeSQL();
                    database_ret = _WRONGDATA;
                }
                file_info.filepath += "/" + file_info.filename;
            }
            cout << "name:" << file_info.filename << endl;
            cout << "size:" << file_info.filesize << endl;
            cout << "md5: " << file_info.MD5 << endl;
            cout << "block_num:" << file_info.block_num << endl;
            cout << "path:" << file_info.filepath << endl;
            cout << "user:" << file_info.username << endl;
            //database_ret = _SUCCESS;

            if (db.sealed_upload(file_info) == SAMENAME)
            {
                database_ret = _WRONGDATA;
                break;
            }
            BlockNo = db.next_block(file_info.MD5);
            cout << "BlockNo:" << BlockNo << endl;

            if (BlockNo < 0 && BlockNo != NOBLOCK)
            {
                database_ret = _WRONGDATA;
            }
            else
            {
                database_ret = BlockNo == NOBLOCK ? NOBLOCK : _SUCCESS;
                Uploaded = db.finished_block(file_info.MD5, file_info.block_num);
                responseBody = SetResponseBody(file_info.block_num, Uploaded, BlockNo > 0 ? BlockNo : 0);
            }
            db.closeSQL();
        }
        else
            database_ret = _WRONGDATA;
        break;
    case Upload_data: //�ļ����ݿ�

        ret = GetFileData(data_begin, boundary, DataLen, FileData, file_info);

        cout << "GetFileData return " << ret << endl;
        if (ret > 0)
        {

            string file_path, file_name;
            //cout << FileData << endl;
            file_block.MD5 = file_info.MD5;
            file_block.block_no = ret;

            //�������ݿ�
            db.initDB("localhost", USER, DBPASSWD, DBNAME);
            db.getroad(file_info.MD5, file_name, file_path);
            cout << "MD5:" << file_info.MD5 << endl;
            cout << "No :" << ret << endl;
            db.upblock_success(file_block);
            BlockNo = db.next_block(file_info.MD5);

            Uploaded = db.finished_block(file_info.MD5, file_info.block_num);
            responseBody = SetResponseBody(file_info.block_num, Uploaded, BlockNo > 0 ? BlockNo : 0);
            db.closeSQL();

            SaveFile(ret, DataLen, FileData, file_path, file_name);
            database_ret = BlockNo == NOBLOCK ? NOBLOCK : _SUCCESS;
        }
        else
            database_ret = _WRONGDATA;
        break;
    case GetDir: //��ȡĿ¼
        ret = GetUsername(data_begin, u);
        //cout<<"user:"<<u<<endl;
        if (ret == _SUCCESS)
        {
            file_format *filetree;
            int filenum;
            //�������ݿ�
            db.initDB("localhost", USER, DBPASSWD, DBNAME);

            filenum = db.user_filetree(u, filetree);
            cout << "filenum" << filenum << endl;
            db.closeSQL();
            if (filenum == _ERROR)
            {
                database_ret = _ERROR;
                break;
            }

            responseBody = GetFileTree(filenum, filetree);
            database_ret = _SUCCESS;
        }
        else
            database_ret = _WRONGDATA;
        break;
    case Delete: //ɾ��
        ret = Getfileid(data_begin, fromid);
        //cout<<"user:"<<u<<endl;
        if (ret == _SUCCESS)
        {
            file_format info;
            info.fileid = fromid;
            //�������ݿ�
            db.initDB("localhost", USER, DBPASSWD, DBNAME);

            database_ret = db.sealed_delete(info);
        }
        else
            database_ret = _WRONGDATA;
        break;
    case Move: //�ƶ�
        ret = Getfromtoid(data_begin, fromid, toid);
        //cout<<"user:"<<u<<endl;
        if (ret == _SUCCESS)
        {
            //�������ݿ�
            db.initDB("localhost", USER, DBPASSWD, DBNAME);

            database_ret = db.sealed_move(fromid, toid);
        }
        else
            database_ret = _WRONGDATA;
        break;
    case Copy: //����
        ret = Getfromtoid(data_begin, fromid, toid);
        //cout<<"user:"<<u<<endl;
        if (ret == _SUCCESS)
        {
            //�������ݿ�
            db.initDB("localhost", USER, DBPASSWD, DBNAME);

            database_ret = db.sealed_copy(fromid, toid);
        }
        else
            database_ret = _WRONGDATA;
        break;
    default:
        break;
    }

    delete[] request_body;
    close(req_fd);
    return database_ret;
} //DealRequest

/**********
��socket����Ϊ��������
iSock   �׽���������
**********/
int SetNonBlock(int iSock)
{
    int iFlags;

    iFlags = fcntl(iSock, F_GETFL, 0);
    iFlags |= O_NONBLOCK;
    iFlags |= O_NDELAY;
    int ret = fcntl(iSock, F_SETFL, iFlags);
    return ret;
}

/**********
Ϊ�ϴ��ļ��н�����Ŀ¼
path            ����Ŀ¼�ĸ�Ŀ¼
RelativePath    ĳ�ļ������·��
user            �û���
**********/
int CreateRelativePath(string path, string RelativePath, string user)
{
    char *path_ch = new char[LINEBUF];
    char *path_head = new char[LINEBUF];
    char *path_final = new char[LINEBUF];
    sprintf(path_head, "%s/%s", DataRoot, user.data());
    sprintf(path_ch, "%s", path.data());

    char *RelativeP = new char[RelativePath.length() + 1];
    RelativePath.copy(RelativeP, RelativePath.length());
    RelativeP[RelativePath.length()] = '\0';

    char *pnext, *plast = RelativeP;

    while (1)
    {
        //��һ��/
        pnext = strstr(plast, "/");

        *pnext = '\0';
        sprintf(path_final, "%s%s", path_head, path_ch);
        cout << path_final << " " << plast << endl;
        CreateDir(path_final, plast);

        strcat(path_ch, "/");
        strcat(path_ch, plast);
        cout << path_ch << endl;

        file_format info;
        info.filepath = path_ch;
        info.username = user;
        info.isfolder = true;
        int ret = db.sealed_upload(info);

        if (ret != _SUCCESS)
            return _ERROR;

        *pnext = '/';

        plast = pnext + 1;

        //�Ѿ������һ��/�ˣ�˵����������ļ���
        if (strstr(plast, "/") == NULL)
            break;
    } //while

    delete[] path_ch;
    delete[] RelativeP;
    return _SUCCESS;
} //CreateRelativePath