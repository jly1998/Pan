#include "mysql_select.h"


/**********************************************************
func:   Ϊÿһ���½��û�����һ��Ĭ���ļ�Ŀ¼
        ��ʱ��δ���ļ��������Ĳ���ֻ����file_format����������ļ���
input:  string user
output: int
        �����ɹ�����_SUCCESS
        ����ʧ�ܷ���_ERROR
***********************************************************/
int MyDB::init_catalog(string user)
{

    cout<<"���ڿ�ʼ����Ĭ��Ŀ¼"<<endl;
    //��ȡ��ǰʱ��
    NowDate date = getTime();
    char temp[1024];
    string inst;
    int flag;
    char tmp_path[tempsize]; //����ļ�·��
    string filename;
    //user�ĸ�Ŀ¼
    sprintf(tmp_path, "/%s", user.data());

    //��file_format�н�����Ŀ¼
    sprintf(temp, "INSERT file_format(filename,username,create_date,parentid,isfolder,filepath) VALUES (\"%s\",\"%s\",\"%s\",0,%d,\"%s\")", user.data(), user.data(), date.tmp0, ISCATALOG,tmp_path);
    inst = temp;
    //ִ�в���ָ��
    flag = exeSQL(inst);
    if (flag == _ERROR)
    {
        return _ERROR;
    }

    //�ҵ���Ŀ¼���ļ���
    sprintf(temp, "SELECT fileid FROM file_format WHERE filepath=\"%s\";", tmp_path);
    inst = temp;
    exeSQL(inst);
    row = mysql_fetch_row(result);
    int root = atoi(row[0]);

    //�½��ļ��С��ҵ���Դ��
    filename = PathResource;
    sprintf(temp, "INSERT file_format(filename,username,create_date,parentid,isfolder,filepath) VALUES (\"%s\",\"%s\",\"%s\",%d,%d,\"%s/%s\")", filename.data(), user.data(), date.tmp0, root,ISCATALOG, tmp_path, filename.data());
    inst = temp;
    //ִ�в���ָ��
    flag = exeSQL(inst);
    if (flag == _ERROR)
    {
        return _ERROR;
    }

    //�½��ļ��С��ҵ�Ӧ�á�
    filename = PathApp;
    sprintf(temp, "INSERT file_format(filename,username,create_date,parentid,isfolder,filepath) VALUES (\"%s\",\"%s\",\"%s\",%d,%d,\"%s/%s\")", filename.data(), user.data(), date.tmp0, root,ISCATALOG, tmp_path, filename.data());
    inst = temp;
    //ִ�в���ָ��
    flag = exeSQL(inst);
    if (flag == _ERROR)
    {
        return _ERROR;
    }

    return _SUCCESS;
}

/**********************************************************
func:   �û��ϴ�һ���ļ������Ȳ�ѯfile_store��
        ������У������û�����������file_store�в�����Ӧ���¼
input:  string path �ļ�·��
        string MD5 �ļ�MD5ֵ
        int filesize �ļ���С
output: bool
        ���з���true�����򷵻�false
***********************************************************/
bool MyDB::ifexist_store(string path, string MD5, int filesize)
{
    char temp[tempsize];
    string sql;
    sprintf(temp, "SELECT memid FROM file_store where MD5=\"%s\";", MD5.data());
    sql = temp;
    exeSQL(sql); //��ʱresult�з��ز�ѯ�Ľ����
    //�����ѯΪ�գ�����Ϊ0
    int num_rows = mysql_num_rows(result); //��ȡ��������ܹ�������
    //�ļ������ڣ���file_stores�в�����Ӧ��Ϣ
    if (num_rows == 0)
    {
        sprintf(temp, "INSERT file_store(path,MD5,filesize,usernum) VALUES(\"%s\",\"%s\",%d,%d)", path.data(), MD5.data(), filesize, 1);
        sql = temp;
        exeSQL(sql); //��file_store�в���һ���¼�¼
        return false;
    }
    //�ļ����ڣ�����file_store�е�usernum
    else
    {
        sprintf(temp, "UPDATE file_store SET usernum = usernum + 1 where MD5=\"%s\"", MD5.data());
        sql = temp;
        exeSQL(sql); //��file_store�и����¼�¼
        return true;
    }
    //�������Ҫ��file_format�����һ����¼
}

/**********************************************************
func:   ͬ�û�ͬ�ļ������Ƿ���������
input:  struct file_format
        ��Чֵ  filepath
output: bool
        ����������false������������true
***********************************************************/
bool MyDB::ifsame_filename(file_format file)
{
    char temp[tempsize];
    string sql;
    // sprintf(temp, "SELECT * FROM file_format WHERE filename=\"%s\" and username=\"%s\" and parentid=%d;", file.filename.data(), file.username.data(), file.parentid);
    sprintf(temp, "SELECT * FROM file_format WHERE filepath=\"%s\" ;", file.filepath.data());
    sql = temp;
    exeSQL(sql);
    //�����ѯΪ�գ�����Ϊ0
    int num_rows = mysql_num_rows(result); //��ȡ��������ܹ�����
    if (num_rows == 0)                     //��ѯ���Ϊ0��˵��������(��δ�����Լ�)
        return true;
    else
        return false;
}

/**********************************************************
func:   �ļ������������������⣬׷��������ʾ����
input:  string &filepath
output: void
***********************************************************/
void MyDB::newfilename_deal(string &filepath)
{
    string init_filepath=filepath;
    file_format file;
    file.filepath=filepath;

    char temp[tempsize];

    for (int i=1;;i++)
    {
        //������
        if (ifsame_filename(file) == true)
        {
            cout<<"filepath:"<<filepath<<endl;
            break;
        }
        else
        {
            //���ļ���׷�������ٴ��ж��Ƿ�����
            sprintf(temp, "%s%d", init_filepath.data(), i);
            file.filepath = temp;
            filepath=file.filepath;
        }
    }

    return;
}

/**********************************************************
func:   ����·�����õ�һ���ļ��� ��ţ�
input:  string path ������ʽ������"/fenghui/jisuanji"
output: int parentid
***********************************************************/
int MyDB::path_to_id(string path)
{
    char temp[tempsize];
    string sql;
    int fileid;

    sprintf(temp, "SELECT fileid FROM file_format WHERE filepath=\"%s\";", path.data());
    // sprintf(temp, "SELECT fileid FROM file_format WHERE filepath=\"/%s\";", path.data());
    sql = temp;
    exeSQL(sql); //һ��·��ֻ�ܶ�Ӧһ���ļ���
    row = mysql_fetch_row(result);
    if(row==NULL)
        return _ERROR;
    fileid = atoi(row[0]);
    cout << "path_to_id:" << fileid << endl;
    return fileid;
}

/**********************************************************
func:   ���ݱ�Ŵ����ݿ��в�ѯ��һ��file_format����
input:  int fileid
output: file_format
***********************************************************/
file_format MyDB::id_to_format(int fileid)
{
    file_format file;
    file.fileid=fileid;
    char temp[tempsize];
    string sql;
    sprintf(temp,"SELECT * FROM file_format WHERE fileid=%d;",fileid);
    sql=temp;
    exeSQL(sql);
    //�����ݿ��ѯ��������ת����file_format�ṹ���ʽ
    row = mysql_fetch_row(result);
    if (row == NULL)
        return file;
    file.filename = (!row[1]) ? "" : row[1];
    file.username = (!row[2]) ? "" : row[2];
    strcpy(file.create_date.tmp0, row[3]);
    file.MD5 = (!row[4]) ? "" : row[4];
    file.parentid = (!row[5]) ? -1 : atoi(row[5]);
    file.isfolder = (!row[6]) ? -1 : atoi(row[6]);
    file.filesize = (!row[7]) ? -1 : atoi(row[7]);
    file.filepath = (!row[8]) ? "" : row[8];

    return file;    
}

/**********************************************************
func:   �ж��ļ�2�ǲ����ļ�1�����ļ�
input:  string id1
        string id2
output: bool
        ����Ƿ���true
        ���Ƿ���false
***********************************************************/
bool MyDB::i_am_your_father(string id1,string id2)
{
    //ֻ��Ҫ�ж�id1�Ƿ���id2��ǰ׺
    //�鳭�Ӵ��޽����˵���������ļ���
    if(id2.find(id1) == string::npos)
    {
        return false;
    }
    else
    {
        return true;
    }
    

}


/**********************************************************
func:   ����·�����õ�һ��file_format�ṹ�壻
input:  string path �ļ�·�������û���
        char *MD5 �ļ�MD5ֵ
        int isfolder �Ƿ����ļ���
output: file_format
        ��Чֵ   username
                filename
                filepath(���û�������·���������ݿ��ʽ)
                parentid
                MD5
                isfolder
                create_date
***********************************************************/
file_format MyDB::path_to_format(string path, string MD5, int isfolder)
{
    int layer = 0;
    int i;
    file_format file;
    //int flag;   //����ֵ

    char split[2] = "/";
    char *p;
    char temp[tempsize]; //��char*���ʹ洢stirng����
    sprintf(temp, path.data());
    p = strtok(temp, split); //��ȡ���ĵ�һ���ַ�����username��
    /****************����~GET��username******************/
    file.username = p;
    cout << "username:" << file.username << endl;
    while (p != NULL)
    {
        layer++;
        cout << p << endl;
        p = strtok(NULL, split);
    }
    //��i��¼Ŀ¼�������õ���i-1Ϊ�ļ��ĸ�Ŀ¼parent
    sprintf(temp, path.data());
    string parent_path;
    p = strtok(temp, split);
    for (i = 0; i < layer; i++)
    {
        if (i == layer - 1)
        {
            /****************����~GET��filename******************/
            file.filename = p;
        }
        else
        {
            parent_path.append("/");
            string temp_p = p;
            parent_path.append(temp_p);
            p = strtok(NULL, split);
        }
    }
    cout << "parent:" << parent_path << endl;
    /****************����~GET��filepath******************/
    file.filepath = parent_path;
    file.filepath.append("/");
    file.filepath.append(file.filename);
    /****************����~GET��parentid******************/
    file.parentid = path_to_id(parent_path);
    /****************����~GET��MD5******************/
    if (isfolder == ISFILE)
        file.MD5 = MD5;
    /****************����~GET��isfolder******************/
    file.isfolder = isfolder;
    /****************����~GET��create_date******************/
    NowDate date = getTime();
    memcpy(&(file.create_date), &(date), sizeof(NowDate));

    cout << "path_to_format is over" << endl;
    return file;
}

/**********************************************************
func:   �û��ϴ�һ���ļ������ļ����������ļ�����
        ��������file_bloak���Ƿ���δ�ϴ���ɵĿ飬���жϸ��ļ����ϴ��Ƿ��Ƕϵ�������
        ���ǣ�ɾ��֮ǰ��file_format�н������ļ����¼
            ����file_store�е�usernum--
input:  string path �ļ�·�������ļ��������û���
        string �ļ�MD5ֵ
output: int ��־λ
            FINISHED    �ļ��Ѿ����ϴ����/���ļ�
            CONTINUE     �ļ���δ���ϴ���ɣ�ɾ����Ӧ�ı��¼
***********************************************************/
int MyDB::if_finished(string path,string MD5)
{
    char temp[tempsize];
    string sql;
    //�鿴���ļ��Ƿ�����δ���ϴ��Ŀ�
    sprintf(temp,"SELECT * FROM file_block WHERE MD5=\"%s\";",MD5.data());
    sql=temp;
    exeSQL(sql);
    int num_rows = mysql_num_rows(result);     //��ȡ��������ܹ�������
    if(num_rows==0)//�ļ��Ѿ����ϴ����
    {
        return FINISHED;
    }
   // cout<<"ɾ���ļ���"<<endl;
    //�ļ�û�б��ϴ���ɣ��������ļ���¼��Ӧ��������file_format�У�ɾ��
    sprintf(temp,"DELETE FROM file_format WHERE filepath=\"%s\";",path.data());
    sql=temp;
    exeSQL(sql);

    //ͬʱfile_store�е�usernum--
    sprintf(temp,"UPDATE file_store SET usernum=usernum-1 WHERE MD5=\"%s\";",MD5.data());
    sql=temp;
    exeSQL(sql);
    
    return CONTINUE;
}


/**********************************************************
func:   �û��ϴ�һ���ļ�������file_format,file_store
        ���ļ��������ж��Ƿ��ϴ��꣬���ǣ������������ǣ��ϵ�����
input:  string path �ļ�·�������ļ��������û���
        string MD5 �ļ�MD5ֵ
        int isfolder ��־λ���Ƿ����ļ���
        int filesize �ļ���С
        int block_num ����
output: int ��־λ
        _SUCCESS    ��file_format�в�����Ӧ��¼
                    ��file_store�в�����Ӧ��¼
        <0 ����error��file_format��Ч
            SAMENAME �ļ�����
            _ERROR ���ļ��ĸ�Ŀ¼��δ����
***********************************************************/
int MyDB::upload_file(string path, string MD5, int isfolder, int filesize,int block_num)
{
    file_format file = path_to_format(path, MD5, isfolder);
    int flag;
    char temp[tempsize];
    string sql;

    //�½����ļ��������ļ�����
    if (ifsame_filename(file) == false)
    {
        //�ļ�����
        if(isfolder==ISFILE)
        {
            //�ļ��Ѿ����ϴ��ɹ�
            int flag=if_finished(path,MD5);
            if(flag==FINISHED)
                return SAMENAME;
        }
        //�ļ�������
        else
        {
            //ֱ�ӷ�������
            return SAMENAME;
        }
        
    }

    //������ ����file_format�в���һ�����¼
    //�жϸ�Ŀ¼�Ƿ����
    if(file.parentid==-1)//���ļ��ĸ�Ŀ¼��δ����
    {
        cout<<"����¥�󣺸��ļ��ĸ�Ŀ¼��δ����"<<endl;
        return _ERROR;
    }
    //�ļ��ĸ�Ŀ¼�Ѿ�����
    sprintf(temp, "INSERT file_format(filename,username,create_date,MD5,parentid,isfolder,nowsize,filepath) VALUES(\"%s\",\"%s\",\"%s\",\"%s\",%d,%d,%d,\"%s\")", file.filename.data(), file.username.data(), file.create_date.tmp0, file.MD5.data(), file.parentid, file.isfolder,filesize, file.filepath.data());
    sql = temp;
    exeSQL(sql); //��ʱresult�з��ز���Ľ����
    //���ж�MD5ֵ�Ƿ��Ѿ�����
    //������ļ��У���Ҫ�ݹ��ж�ÿ���ļ���ִ�в�����¼����
    if (file.isfolder == ISCATALOG)
    {
        //��ȡ��Ŀ¼�µ�ÿ���ļ�״̬���ı���Ӧ�ļ�¼
    }
    //������ļ���ֱ���жϸ��ļ�
    else
    {
        //��ѯfile_store�����и���usernum��δ������ӱ��¼
        flag=ifexist_store(file.filepath, file.MD5, filesize);
        //δ���У���file_block�в�����¼
        cout<<"ifexist_store return "<<flag<<endl;
        if (flag == false)
        {
            init_block(file.MD5,block_num);
        }
    }
    return _SUCCESS;
}

/**********************************************************
func:   �û��ϴ�һ���ļ�������file_format��
        �൱�ڷ�װ��upload_file
input:  file_format ���Կͻ��˵Ľṹ�壬�������Լ��
        username 
        filepath
        isfolder 
        MD5         *
        filesize    *
        block_num   *   ֻ���ļ���Ҫ
output: ��upload_file�ķ���ֵ��ͬ
        int ��־λ
        _SUCCESS    ��file_format�в�����Ӧ��¼
                    ��file_store�в�����Ӧ��¼
        <0 ����error��file_format��Ч
            SAMENAME �ļ�����
***********************************************************/
int MyDB::sealed_upload(file_format file)
{
    char temp[tempsize];
    string path;
    sprintf(temp, "/%s%s", file.username.data(), file.filepath.data());
    path = temp;
    int flag = upload_file(path, file.MD5, file.isfolder, file.filesize,file.block_num);

    return flag;
}

/**********************************************************
func:   ɾ��һ���ļ�����ɾ�������ڴ�
        �������ݱ�ɾ��file_format�еļ�¼������file_store
input:  string path ·�������û�������Ŀ¼
output: void
***********************************************************/
void MyDB::delete_file(string path)
{
    //��������õ�MD5��is_folder��Ϊ��Ч��ֻ��Ϊ�˵õ����ӹ������filepath
    file_format file = path_to_format(path, "0000", 1);
    char temp[tempsize];
    string sql;
    string MD5;
    int fileid;
    sprintf(temp, "SELECT fileid FROM file_format where filepath=\"%s\";", file.filepath.data());
    sql = temp;
    exeSQL(sql); //��ʱresult�з��ز�ѯ�Ľ����
    //��ȡ��һ������
    row = mysql_fetch_row(result);
    //����һ��·��ӳ�䵽һ��fileid��
    int num_rows = mysql_num_rows(result); //��ȡ��������ܹ�������
    if (num_rows == 0)
        return;
    fileid = atoi(row[0]);

    sprintf(temp, "SELECT MD5 FROM file_format where fileid=%d;", fileid);
    sql = temp;
    exeSQL(sql); //��ʱresult�з��ز�ѯ�Ľ����
    //��ȡ��һ������
    row = mysql_fetch_row(result);
    //����һ���ļ�ӳ�䵽һ��MD5����
    MD5 = row[0];
    //ɾ��file_format�еı��¼
    sprintf(temp, "DELETE FROM file_format where fileid=%d;", fileid);
    sql = temp;
    exeSQL(sql);
    //file_store���û�ռ����--
    sprintf(temp, "UPDATE file_store SET usernum = usernum-1 where MD5=\"%s\" and usernum > 0;", MD5.data());
    sql = temp;
    exeSQL(sql);

    return;
}

/**********************************************************
func:   �û�ɾ���ļ��У�����file_format��
input:  string path ·�������û�������Ŀ¼
output: int ��־λ
        _SUCCESS    ��file_format�и�����Ӧ��¼
        _ERROR      �ļ������ڻ���������
***********************************************************/
int MyDB::delete_catalog(string path)
{
    char temp[tempsize];
    string sql;
    int fileid;

    //�ҵ��ļ���fileid
    sprintf(temp,"SELECT fileid FROM file_format WHERE filepath=\"%s\";",path.data());
    sql=temp;
    exeSQL(sql);
    //��ffile.fileid���
    if(row==NULL)//·�������ڣ�
        return _ERROR;
    row = mysql_fetch_row(result);
    fileid=atoi(row[0]);

    //����file_format��ɾ�������ļ��б��¼
    //�ļ��в����ܳ�����file_store��
    sprintf(temp,"DELETE FROM file_format WHERE fileid=%d;",fileid);
    //cout<<"delete catalog:"<<fileid<<endl;
    sql=temp;
    exeSQL(sql);

    //�ҵ����ļ����µ����ļ�
    sprintf(temp,"SELECT * FROM file_format WHERE parentid=%d;",fileid);
    sql=temp;
    exeSQL(sql);

    //�ݹ��ѯ����ʹ�ù����ռ䣬Ӧ�����þֲ������洢ÿ��result�Ľ��
    int num_fields = mysql_num_fields(result); //��ȡ��������ܹ����ֶ�����������
    int num_rows = mysql_num_rows(result);     //��ȡ��������ܹ�������

    //���ļ���Ϊ��
    if(num_rows==0)
        return _SUCCESS;
    string temp_result[num_rows][num_fields];
     //ÿһ�� ����result��temp_result
    for (int i = 0; i < num_rows; i++)        
    {
        //��ȡ��һ������
        row = mysql_fetch_row(result);
        if (row == NULL)
            break;
        for (int j = 0; j < num_fields; j++) 
        {
            if(row[j] == NULL)
                temp_result[i][j]="";
            else
                temp_result[i][j]=row[j];
        }
    }
    
    for(int i=0;i<num_rows;i++)
    {
        string path_c;
        //"/�û���/��Ŀ¼/�ļ���"
        sprintf(temp,"%s/%s",path.data(),temp_result[i][1].data());
        path_c=temp;
        //cout<<"��һ��ɾ���ļ�"<<path_c<<endl;
        if(atoi(temp_result[i][6].data())==ISCATALOG)//�ļ��У��ݹ����
        {
           // cout<<"ɾ����һ���ļ���"<<endl;
            delete_catalog(path_c);
        }
        else
        {
            //ֱ�ӵ���delete_file����
            delete_file(path_c);
        }
    }

    return _SUCCESS;
}

/**********************************************************
func:   ɾ��һ���ļ�����ɾ�������ڴ�
        �������ݱ�ɾ��file_format�еļ�¼������file_store
        �൱�ڷ�װ��delete_file��delete_catalog
input:  file_format file 
                ���Կͻ��˵Ľṹ���е���Ч����ֻ��fileid
                ��Ҫ�ڲ��Լ�ת��Ϊ�ṹ��
        ��Чֵ��int fileid
output: void
***********************************************************/
void MyDB::sealed_delete(file_format file)
{
    file_format real_file;
    real_file=id_to_format(file.fileid);
    // char temp[tempsize];
    // string path;
    // sprintf(temp, "/%s%s", real_file.username.data(), real_file.filepath.data());
    // path = temp;

    //ɾ���ļ�
    if(real_file.isfolder==ISFILE)
    {
        //cout<<"real_file.filepath:"<<real_file.filepath<<endl;
        delete_file(real_file.filepath);
    }
    //ɾ���ļ���
    else
    {
        delete_catalog(real_file.filepath);
    }

    return;
}

/**********************************************************
func:   �û��ƶ��ļ�������file_format��
        ��Ӱ�쵽file_store���ļ��洢Ŀ¼�Ե�һ���ϴ�Ϊ׼����Ӧͬ������
input:  string from_path �ƶ�ǰ��·���������ļ��������û���
        string to_path �ƶ����·���������ļ��������û���
        string username �û���
output: int ��־λ
        _SUCCESS    ��file_format�и�����Ӧ��¼
        <0 ����error��file_format��Ч
            SAMENAME �ļ�����
***********************************************************/
int MyDB::move_file(string from_path,string to_path,string username)
{
    char temp[tempsize];
    string sql;
    string temp_fpath;//�����û���֮���from_path
    string temp_tpath;//�����û���֮���to_path
    file_format tfile;
    file_format ffile;
    bool flag;
    //sprintf(temp,"/%s%s",username.data(),from_path.data());
    temp_fpath=from_path;
    //sprintf(temp,"/%s%s",username.data(),to_path.data());
    temp_tpath=to_path;
    //�����MD5��is_folder����Ч����Ϊ�˵õ�to_pathת��������file_format�ṹ
    //��Чֵ��filename��username��parentid
    //�õ��������ݿ�ṹҪ���filepath
    //Ϊ�˸�ifsame_filename����
    ffile=path_to_format(temp_fpath,"0000",1);
    tfile=path_to_format(temp_tpath,"0000",1);
    flag=ifsame_filename(tfile);
     
    // cout<<endl;
    // cout<<"temp_tpath.data()"<<temp_tpath.data()<<endl;
    // cout<<"ffile.filepath.data():"<<ffile.filepath.data()<<endl;
    // cout<<"tfile.filepath.data():"<<tfile.filepath.data()<<endl;

    if(flag==true)//����������ֱ�Ӹ���file_format��filepath
    {
        //����file_format��filepath��parentid
        sprintf(temp,"UPDATE file_format SET parentid=%d WHERE filepath=\"%s\";",tfile.parentid,ffile.filepath.data());
        sql=temp;
        exeSQL(sql);

        sprintf(temp,"UPDATE file_format SET filepath=\"%s\" WHERE filepath=\"%s\";",tfile.filepath.data(),ffile.filepath.data());
        sql=temp;
        exeSQL(sql);
        
        //����file_store
        sprintf(temp,"UPDATE file_store SET path=\"%s\" WHERE path=\"%s\";",tfile.filepath.data(),ffile.filepath.data());
        sql=temp;
        exeSQL(sql);
    
        return _SUCCESS;
    }
    else//������ֱ�ӷ��أ��ƶ�ʧ��
    {
        return SAMENAME;
    }
}

/**********************************************************
func:   �û��ƶ��ļ��У�����file_format��
        ��Ӱ�쵽file_store���ļ��洢Ŀ¼�Ե�һ���ϴ�Ϊ׼����Ӧͬ������
input:  string from_path �ƶ�ǰ��·���������ļ��������û���
        string to_path �ƶ����·���������ļ��������û���
        string username �û���
output: int ��־λ
        _SUCCESS    ��file_format�и�����Ӧ��¼
        <0 ����error��file_format��Ч
            SAMENAME �ļ�����
***********************************************************/
int MyDB::move_catalog(string from_path,string to_path,string username)
{
    file_format ffile;
    file_format tfile;
    char temp[tempsize];
    string sql;
    string temp_fpath;
    string temp_tpath;
    //sprintf(temp,"/%s%s",username.data(),from_path.data());
    temp_fpath=from_path;
    //sprintf(temp,"/%s%s",username.data(),to_path.data());
    temp_tpath=to_path;
    //�����MD5��Ч��isfolde������Ч��Ϊ�˵õ�to_pathת��������file_format�ṹ��filename,parentid,filepath,username
    ffile=path_to_format(temp_fpath,"0000",1);
    tfile=path_to_format(temp_tpath,"0000",1);

    //�����ж��Ƿ��������Ƿ���ͬ���ļ���/�ļ�
    int flag=ifsame_filename(tfile);
    if(flag==false)//������
        return SAMENAME;
    
    //������
    //�ҵ��ļ���fileid
    sprintf(temp,"SELECT fileid FROM file_format WHERE filepath=\"%s\";",ffile.filepath.data());
    sql=temp;
    exeSQL(sql);
    //��ffile.fileid���
    if(row==NULL)//·�������ڣ�
        return _ERROR;
    row = mysql_fetch_row(result);
    ffile.fileid=atoi(row[0]);
    tfile.fileid=atoi(row[0]);

    //����file_format��filepath��parentid
    //�ļ��в����ܳ�����file_store��
    sprintf(temp,"UPDATE file_format SET parentid=%d WHERE filepath=\"%s\";",tfile.parentid,ffile.filepath.data());
    sql=temp;
    exeSQL(sql);

    sprintf(temp,"UPDATE file_format SET filepath=\"%s\" WHERE filepath=\"%s\";",tfile.filepath.data(),ffile.filepath.data());
    sql=temp;
    exeSQL(sql);

    //�ҵ����ļ����µ����ļ�
    sprintf(temp,"SELECT * FROM file_format WHERE parentid=%d;",ffile.fileid);
    sql=temp;
    exeSQL(sql);

    //�ݹ��ѯ����ʹ�ù����ռ䣬Ӧ�����þֲ������洢ÿ��result�Ľ��
    int num_fields = mysql_num_fields(result); //��ȡ��������ܹ����ֶ�����������
    int num_rows = mysql_num_rows(result);     //��ȡ��������ܹ�������
    //���ļ���Ϊ��
    if(num_rows==0)
        return _SUCCESS;
    string temp_result[num_rows][num_fields];
     //ÿһ�� ����result��temp_result
    for (int i = 0; i < num_rows; i++)        
    {
        //��ȡ��һ������
        row = mysql_fetch_row(result);
        if (row == NULL)
            break;
        for (int j = 0; j < num_fields; j++) 
        {
            if(row[j] == NULL)
                temp_result[i][j]="";
            else
                temp_result[i][j]=row[j];
        }
    }
    
    for(int i=0;i<num_rows;i++)
    {
        string from_path_c;
        string to_path_c;
        //"��Ŀ¼/�ļ���"
        sprintf(temp,"%s/%s",from_path.data(),temp_result[i][1].data());
        from_path_c=temp;
        sprintf(temp,"%s/%s",to_path.data(),temp_result[i][1].data());
        to_path_c=temp;

        if(temp_result[i][6]=="1")//�ļ��У��ݹ����
        {
            move_catalog(from_path_c,to_path_c,username);
        }
        else
        {
            //ֱ�ӵ���move_file����
            move_file(from_path_c,to_path_c,username);
        }
    }

    // for (int i = 0; i < num_rows; i++)         //ÿһ��
    // {
    //     for (int j = 0; j < num_fields; j++) //���ÿһ�ֶ�
    //     {
    //         if(temp_result[i][j].data()==NULL)
    //             cout<<"NULL";
    //         else
    //         {
    //             cout<<temp_result[i][j]<<"\t";
    //         }
    //     }
    //     cout<<endl;
    // }

    return _SUCCESS;
}

/**********************************************************
func:   �ƶ��ļ�/��
        �൱�ڷ�װ��move_file��move_catalog
input:  int from_id ��ʼ�ļ�id
        int to_id ��ֹ·�����ļ���id
                ��Ҫ�ڲ��Լ�ת��Ϊ�ṹ��
output: �ļ�/���ƶ����
***********************************************************/
int MyDB::sealed_move(int from_id,int to_id)
{
    file_format from_file;
    file_format to_file;
    //����id�õ���Ӧ�Ľṹ��
    //��Чֵ�� filepath(���û�������ʼ�ļ�·��)
    //        username(�û���)
    //        filename(�ļ���)
    from_file=id_to_format(from_id);
    //��Чֵ���ƶ����·���ĸ�Ŀ¼
    to_file=id_to_format(to_id);

    char temp_path[tempsize];
    string path;
    sprintf(temp_path,"%s/%s",to_file.filepath.data(),from_file.filename.data());
    path=temp_path;


    if(to_file.isfolder==ISFILE)
    {
        cout<<"�ƶ���Ŀ��·�����ļ��ǲ��������Ŷ��"<<endl;
        return WRONGTOFILE;
    }

    //�ƶ��ļ�
    if(from_file.isfolder==ISFILE)
    {
        //cout<<"real_file.filepath:"<<real_file.filepath<<endl;
        return move_file(from_file.filepath,path,from_file.username);
    }
    //�ƶ��ļ���
    else
    {
        if(i_am_your_father(from_file.filepath,path)==true)
        {
            cout<<"������������Ҫ���ļ����ƶ��������ļ���������Ŷ��"<<endl;
            return MOVETOSON;
        }
         return move_catalog(from_file.filepath,path,from_file.username);
    }

}

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
int MyDB::copy_file(string username,string from_path,string to_path)
{
    char temp[tempsize];
    string sql;
    string temp_fpath;//�����û���֮���from_path
    string temp_tpath;//�����û���֮���to_path
    file_format tfile;
    file_format ffile;
    bool flag;
    //sprintf(temp,"/%s%s",username.data(),from_path.data());
    temp_fpath=from_path;
    //sprintf(temp,"/%s%s",username.data(),to_path.data());
    temp_tpath=to_path;
    
    //�ҵ��ļ���MD5ֵ
    sprintf(temp,"SELECT MD5 FROM file_format WHERE filepath=\"%s\";",temp_fpath.data());
    sql=temp;
    exeSQL(sql);
    row = mysql_fetch_row(result);
    if(row==NULL)
    {
        return _ERROR;
    }
    string temp_MD5=row[0];

    //�õ��������ݿ�ṹҪ���filepath
    //Ϊ�˸�ifsame_filename����
    ffile=path_to_format(temp_fpath,temp_MD5,0);
    tfile=path_to_format(temp_tpath,temp_MD5,0);
    flag=ifsame_filename(tfile);
    if(flag==false)//����
    {
        return SAMENAME;
    }
    else//������
    {
        //��file_format�в���һ���¼�¼
        sprintf(temp,"INSERT file_format(filename,username,create_date,MD5,parentid,isfolder,filepath) VALUES(\"%s\",\"%s\",\"%s\",\"%s\",%d,%d,\"%s\");",tfile.filename.data(),tfile.username.data(),tfile.create_date.tmp0,tfile.MD5.data(),tfile.parentid,tfile.isfolder,tfile.filepath.data());
        sql=temp;
        exeSQL(sql);
        
        //��file_store�д洢����Ӧ�ļ����û�ռ����++
        sprintf(temp,"UPDATE file_store SET usernum=usernum+1 WHERE MD5=\"%s\";",temp_MD5.data());
        sql=temp;
        exeSQL(sql);

        return _SUCCESS;
    } 
}

/**********************************************************
func:   �����ļ��У��ݹ������file_format��������ɼ�¼
input:  string username
        string from_path �ƶ�ǰ��·���������ļ��������û���
        string to_path �ƶ����·���������ļ��������û���
output: int
        ��������SAMENAME
        ʧ�ܷ���_ERROR
        �������Ҹ��Ƴɹ�����_SUCCESS
***********************************************************/
int MyDB::copy_catalog(string username,string from_path,string to_path)
{
    file_format ffile;
    file_format tfile;
    char temp[tempsize];
    string sql;
    string temp_fpath;
    string temp_tpath;
    //sprintf(temp,"/%s%s",username.data(),from_path.data());
    temp_fpath=from_path;
    //sprintf(temp,"/%s%s",username.data(),to_path.data());
    temp_tpath=to_path;
    //�����MD5��Ч��isfolde������Ч��Ϊ�˵õ�to_pathת��������file_format�ṹ��filename,parentid,filepath,username
    ffile=path_to_format(temp_fpath,"0000",1);
    tfile=path_to_format(temp_tpath,"0000",1);

    //�����ж��Ƿ��������Ƿ���ͬ���ļ���/�ļ�
    int flag=ifsame_filename(tfile);
    if(flag==false)//������
        return SAMENAME;
    
    //������
    //�ҵ��ļ���fileid
    sprintf(temp,"SELECT fileid FROM file_format WHERE filepath=\"%s\";",ffile.filepath.data());
    sql=temp;
    exeSQL(sql);
    //��ffile.fileid���
    if(row==NULL)//·�������ڣ�
        return _ERROR;
    row = mysql_fetch_row(result);
    ffile.fileid=atoi(row[0]);
    tfile.fileid=atoi(row[0]);

    //��file_format�в���һ���¼�¼
    //�ļ��в����ܳ�����file_store��
    sprintf(temp,"INSERT file_format(filename,username,create_date,parentid,isfolder,filepath) VALUES(\"%s\",\"%s\",\"%s\",%d,%d,\"%s\");",tfile.filename.data(),tfile.username.data(),tfile.create_date.tmp0,tfile.parentid,tfile.isfolder,tfile.filepath.data());
    sql=temp;
    exeSQL(sql);

    //�ҵ����ļ����µ����ļ�
    sprintf(temp,"SELECT * FROM file_format WHERE parentid=%d;",ffile.fileid);
    sql=temp;
    exeSQL(sql);

    //�ݹ��ѯ����ʹ�ù����ռ䣬Ӧ�����þֲ������洢ÿ��result�Ľ��
    int num_fields = mysql_num_fields(result); //��ȡ��������ܹ����ֶ�����������
    int num_rows = mysql_num_rows(result);     //��ȡ��������ܹ�������
    //���ļ���Ϊ��
    if(num_rows==0)
        return _SUCCESS;
    string temp_result[num_rows][num_fields];
     //ÿһ�� ����result��temp_result
    for (int i = 0; i < num_rows; i++)        
    {
        //��ȡ��һ������
        row = mysql_fetch_row(result);
        if (row == NULL)
            break;
        for (int j = 0; j < num_fields; j++) 
        {
            if(row[j] == NULL)
                temp_result[i][j]="";
            else
                temp_result[i][j]=row[j];
        }
    }
    
    for(int i=0;i<num_rows;i++)
    {
        string from_path_c;
        string to_path_c;
        //"/�û���/��Ŀ¼/�ļ���"
        sprintf(temp,"%s/%s",from_path.data(),temp_result[i][1].data());
        from_path_c=temp;
        sprintf(temp,"%s/%s",to_path.data(),temp_result[i][1].data());
        to_path_c=temp;

        if(temp_result[i][6]=="1")//�ļ��У��ݹ����
        {
            copy_catalog(username,from_path_c,to_path_c);
        }
        else
        {
            //ֱ�ӵ���copy_file���������ļ�
            copy_file(username,from_path_c,to_path_c);
        }
    }

    return _SUCCESS;   

}

/**********************************************************
func:   �����ļ�/��
        �൱�ڷ�װ��move_file��move_catalog
        ͬ�ļ����¸��ƻ�׷�����������ļ���
input:  int from_id ��ʼ�ļ�id
        int to_id ��ֹ·�����ļ���id
                ��Ҫ�ڲ��Լ�ת��Ϊ�ṹ��
output: �ļ�/�и��ƽ��
***********************************************************/
int MyDB::sealed_copy(int from_id,int to_id)
{
    file_format from_file;
    file_format to_file;
    //����id�õ���Ӧ�Ľṹ��
    //��Чֵ�� filepath(���û�������ʼ�ļ�·��)
    //        username(�û���)
    //        filename(�ļ���)
    from_file=id_to_format(from_id);
    //��Чֵ���ƶ����·���ĸ�Ŀ¼
    to_file=id_to_format(to_id);

    char temp_path[tempsize];
    string path;
    sprintf(temp_path,"%s/%s",to_file.filepath.data(),from_file.filename.data());
    path=temp_path;

    // cout<<endl<<endl<<endl;
    // cout<<"now you are in sealed_copy"<<endl;
    // cout<<"from_file.filepath:"<<from_file.filepath<<endl;
    // cout<<"path:"<<path<<endl;

    if(to_file.isfolder==ISFILE)
    {
        cout<<"���Ƶ�Ŀ��·�����ļ��ǲ��������Ŷ��"<<endl;
        return WRONGTOFILE;
    }
    //�����ļ�
    if(from_file.isfolder==ISFILE)
    {
        //ͬ�ļ����¸���
        if(from_file.filepath==path)
        {
            //���ļ������׺���ֱ�������
            newfilename_deal(path);
        }
        //cout<<"real_file.filepath:"<<real_file.filepath<<endl;
         return copy_file(from_file.username,from_file.filepath,path);
    }
    //�����ļ���
    else
    {
        //ͬ�ļ����¸���
        if(from_file.filepath==path)
        {
            //���ļ������׺���ֱ�������
            newfilename_deal(path);
        }
        return copy_catalog(from_file.username,from_file.filepath,path);
    }

}


/**********************************************************
func:   �û�����һ��·����ͨ����ַȥ��ѯ���ݿ⣬���ظ��û��ò��ṹ
input:  file_format ���Կͻ��˵Ľṹ�壬�������Լ��
        ��Чֵ   filepath
                username
output: int ���ز�ѯ���Ľ����������
        ������ļ�������-1
        ������ļ��У�����Ŀ¼�µ��ļ����������ļ���
                    ���ڶ�̬����file_format����Ĵ�С
***********************************************************/
int MyDB::find_content(const file_format file)
{
    if (file.isfolder == ISFILE)
        return -1;
    char temp[tempsize];
    string path;
    string sql;
    sprintf(temp, "/%s%s", file.username.data(), file.filepath.data());
    path = temp;
    //�����MD5ֵ��isfolder�������壬Ŀ����Ϊ�˵õ��������ݿ�ṹ��filepath
    file_format temp_file = path_to_format(path, "0000", 1);
    //��ѯ��Ŀ¼�ṹ��id��
    int fileid = path_to_id(temp_file.filepath.data());
    sprintf(temp, "SELECT * FROM file_format WHERE parentid = %d;", fileid);
    sql = temp;
    exeSQL(sql); //��ʱ��ѯ����Ѱ���file_format����ʽ���뵽�������

    int num_rows = mysql_num_rows(result); //��ȡ��������ܹ�������

    return num_rows;
}

/**********************************************************
func:   ȥ��·��ǰ���û��������͸��ͻ���
input:  string path
output: string û���û�����·��
***********************************************************/
string nouser_path(char *path)
{
    char split[2] = "/";
    char *p;
    char temp[tempsize]; //��char*���ʹ洢stirng����
    int layer = 0;
    string opath="/";
    strcpy(temp, path);
    p = strtok(temp, split); //��ȡ���ĵ�һ���ַ�����username��
    while (p != NULL)
    {
        layer++;
        p = strtok(NULL, split);
    }
    //��i��¼Ŀ¼�������õ���i-1Ϊ�ļ��ĸ�Ŀ¼parent
    strcpy(temp, path);
    p = strtok(temp, split);
    for (int i = 1; i < layer; i++)
    {
        string temp_p;
        p = strtok(NULL, split);
        temp_p = p;
        opath.append(temp_p);
        if (i != layer - 1)
        {
            opath.append("/");
        }
    }

    return opath;
}

/**********************************************************
func:   �˺�����find_content(const file_format file)����ʹ��,
        ��ʱresult���Ѵ洢�˲�ѯ���
input:  file_format* ���飬����Ŀ¼�µ��ļ��ṹ���ɸú���д��  
output: int ���ز�ѯ���Ľ�������������ڶ�̬����file_format����Ĵ�С
***********************************************************/
void MyDB::store_format(file_format *file)
{
    int num_fields = mysql_num_fields(result); //��ȡ��������ܹ����ֶ�����������
    int num_rows = mysql_num_rows(result);     //��ȡ��������ܹ�������
    for (int i = 0; i < num_rows; i++)         //��ÿһ�������content��
    {
        //��ȡ��һ������
        row = mysql_fetch_row(result);
        if (row ==NULL)
            break;
        //�ѽ������file_format[i]
        //cout<<"columns:"<<num_fields<<endl;
        file[i].fileid = (!row[0]) ? -1 : atoi(row[0]);
        file[i].filename = row[1];
        file[i].username = row[2];
        strcpy(file[i].create_date.tmp0, row[3]);
        file[i].MD5 = (!row[4])?"":row[4];
        file[i].parentid = (!row[5]) ? -1 : atoi(row[5]);
        file[i].isfolder = (!row[6]) ? -1 : atoi(row[6]);
        file[i].filesize = (!row[7]) ? -1 : atoi(row[7]);
        file[i].filepath = nouser_path(row[8]);
    }
    // for(int i=0;i<num_rows;i++)
    // {
    //     cout<<i<<endl;
    //     cout<<"file[i]��fileid:"<<file[i].fileid<<endl;
    //     cout<<"file[i]��filename:"<<file[i].filename<<endl;
    //     cout<<"file[i]��username:"<<file[i].username<<endl;
    //     cout<<"file[i]��create_date:"<<file[i].create_date.tmp0<<endl;
    //     cout<<"file[i]��MD5:"<<file[i].MD5<<endl;
    //     cout<<"file[i]��parentid:"<<file[i].parentid<<endl;
    //     cout<<"file[i]��isfolder:"<<file[i].isfolder<<endl;
    //     cout<<"file[i]��filesize:"<<file[i].filesize<<endl;
    //     cout<<"file[i]��filepath:"<<file[i].filepath<<endl;
    //     cout<<endl;
    // }
    return;
}

/**********************************************************
func:   ��ѯ���������Ƿ���Գɹ������ͻ�����
        ͬʱ����file_format��file_store
input:  file_format 
        ��Чֵ  username
                filepath ·�������ڿͻ��ˣ�û��username
        string newname
output: int ���ز������
            _SUCCESS �ɹ�������������file_format����Ӧ�ı��¼
            SAMENAME �����������⣬������ʧ��
***********************************************************/
int MyDB::ifrename(file_format file, string newname)
{
    file_format temp_file;
    string temp_path;
    string sql;
    char temp[tempsize];
    bool flag;
    //ƴ�ӵõ���Ҫ�ĸ�ʽ
    sprintf(temp, "/%s%s", file.username.data(), file.filepath.data());
    temp_path = temp;
    //�����MD5��isfolderͬ�������壬Ŀ���ǵõ�parentid��filepath��username
    temp_file = path_to_format(temp_path, "0000", 1);
    temp_file.filename = newname; //�滻�ļ�������
    flag = ifsame_filename(temp_file);
    if (flag == false) //������
    {
        return SAMENAME;
    }
    else //��������Ҫ���±��¼file_format�е�filename��filepath
    {
        sprintf(temp, "SELECT fileid FROM file_format WHERE filepath= \"%s\"", temp_file.filepath.data());
        sql = temp;
        exeSQL(sql);                   //��ѯ�õ�fileid
        row = mysql_fetch_row(result); //һ��·����ӦΨһ�ļ�id
        int fileid = atoi(row[0]);

        sprintf(temp, "SELECT filepath FROM file_format WHERE fileid= %d", temp_file.parentid);
        sql = temp;
        exeSQL(sql);                   //��ѯ�õ���Ŀ¼·��
        row = mysql_fetch_row(result); //һ���ļ�id��ӦΨһ·�������û���
        temp_path=row[0];//�õ���Ŀ¼·��
        temp_path.append("//");
        temp_path.append(newname);
        
        //����filepath
        sprintf(temp,"UPDATE file_format SET filepath=\"%s\" WHERE fileid=%d;",temp_path.data(),fileid);
        sql=temp;
        exeSQL(sql);

        //����filename
        sprintf(temp,"UPDATE file_format SET filename=\"%s\" WHERE fileid=%d;",newname.data(),fileid);
        sql=temp;
        exeSQL(sql);

        //����file_store���path
        sprintf(temp,"UPDATE file_store SET path=\"%s\" WHERE path=\"%s\";",temp_path.data(),file.filepath.data());
        sql=temp;
        exeSQL(sql);
    }

    return _SUCCESS;
}

/**********************************************************
func:   ����MD5ֵ�����ظ��ͻ��ļ�ʵ�ʴ洢·�������û������൱�����ݿ��д洢·����
        ���ͻ�����
input:  string MD5
        string &filename "newfile"
        string &parentpath "/fenhui/jisuanji"
output: int
        ����ѯʧ�ܣ�����_ERROR
        ����ѯ�ɹ�������_SUCCESS
***********************************************************/
int MyDB::getroad(string MD5,string &filename,string &parentpath)
{
    string ret;
    char temp[tempsize];
    string sql;
    sprintf(temp,"SELECT path FROM file_store WHERE MD5=\"%s\";",MD5.data());
    sql=temp;
    exeSQL(sql);
    row = mysql_fetch_row(result);
    //��ѯʧ�ܣ�����_ERROR
    if(row==NULL)
    {
        return _ERROR;
    }
    //��ѯ�ɹ����ָĿ¼���ļ���
    file_format file;
    string path=row[0];//�ļ�����·��
    //Ϊ�˵õ�parentid�Լ�filename
    file=path_to_format(path,"1111",0);
    sprintf(temp,"SELECT filepath FROM file_format WHERE fileid=%d;",file.parentid);
    sql=temp;
    exeSQL(sql);
    row = mysql_fetch_row(result);
    if(row==NULL)
    {
        return _ERROR;
    }
    parentpath=row[0];
    filename=file.filename;


    return _SUCCESS;
}

/**********************************************************
func:   �����û������õ����û���Ŀ¼�ṹ������
        ���ͻ�����
input:  string username
        file_format &*file �ṹ����������ã�
output: int
        ��ѯ�ɹ����ز�ѯ��������
        ��ѯʧ�ܷ���_ERROR
***********************************************************/
int MyDB::user_filetree(string username,file_format* &file)
{
    char temp[tempsize];
    string sql;
    //��ѯָ���û��������ļ�
    sprintf(temp,"SELECT * FROM file_format WHERE username=\"%s\";",username.data());
    sql=temp;
    exeSQL(sql);

    int num_fields = mysql_num_fields(result); //��ȡ��������ܹ����ֶ�����������
    int num_rows = mysql_num_rows(result);     //��ȡ��������ܹ�������

    //����ṹ������ռ�
    file=new file_format[num_rows];
    if(file==NULL)
        return _ERROR; 
    for (int i = 0; i < num_rows; i++)         //��ÿһ�������content��
    {
        //��ȡ��һ������
        row = mysql_fetch_row(result);
        if (row ==NULL)
            break;
        //�ѽ������file_format[i]
        //cout<<"columns:"<<num_fields<<endl;
        file[i].fileid = (!row[0]) ? -1 : atoi(row[0]);
        file[i].filename = (!row[1]) ? "" : row[1];
        file[i].username = (!row[2]) ? "" : row[2];
        strcpy(file[i].create_date.tmp0, row[3]);
        file[i].MD5 = (!row[4]) ? "" : row[4];
        file[i].parentid = (!row[5]) ? -1 : atoi(row[5]);
        file[i].isfolder = (!row[6]) ? -1 : atoi(row[6]);
        file[i].filesize = (!row[7]) ? -1 : atoi(row[7]);
        file[i].filepath = nouser_path(row[8]);
    }
    // for(int i=0;i<num_rows;i++)
    // {
    //     cout<<i<<endl;
    //     cout<<"file[i]��fileid:"<<file[i].fileid<<endl;
    //     cout<<"file[i]��filename:"<<file[i].filename<<endl;
    //     cout<<"file[i]��username:"<<file[i].username<<endl;
    //     cout<<"file[i]��create_date:"<<file[i].create_date.tmp0<<endl;
    //     cout<<"file[i]��MD5:"<<file[i].MD5<<endl;
    //     cout<<"file[i]��parentid:"<<file[i].parentid<<endl;
    //     cout<<"file[i]��isfolder:"<<file[i].isfolder<<endl;
    //     cout<<"file[i]��filesize:"<<file[i].filesize<<endl;
    //     cout<<"file[i]��filepath:"<<file[i].filepath<<endl;
    //     cout<<endl;
    // }
    return num_rows;
}



/*************************�����***************************/

/**********************************************************
func:   ��ʼ��һ���ļ��Ŀ飬���ļ������п춼���뵽file_block��
        ���Ŵ�1��ʼ
input:  string MD5 �ļ���Ψһ��ʶ
        int block_num �ļ��Ŀ���
output: int �ɹ����뷵��_SUCCESS
***********************************************************/
int MyDB::init_block(string MD5,int block_num)
{
    char temp[tempsize];
    string sql;
    //int filesize;
    // int block_num=0;//�õ��ļ��ֺõĿ�����Ҳ�Ǽ�������ı�����
    // //��ѯ�ļ��Ĵ�С
    // sprintf(temp,"SELECT filesize FROM file_store WHERE MD5=\"%s\";",MD5.data());
    // sql=temp;
    // exeSQL(sql);
    // //����˵��һ��MD5ֵ��ӦΨһ�ļ�����Ӧһ���ļ���С����
    // row = mysql_fetch_row(result);
    // if(row==NULL)
    // {
    //     return _ERROR;
    // }
    // filesize=atoi(row[0]);
    // block_num=filesize/BLOCKSIZE;
    // if(filesize%BLOCKSIZE!=0)//���һ�鲻һ����д��
    //     block_num++;
    //��file_blockִ�г�ʼ�������
    for(int i=1;i<=block_num;i++)
    {
        sprintf(temp,"INSERT file_block(MD5,block_no,ifusing) VALUES(\"%s\",%d,%d);",MD5.data(),i,NOUSING);
        sql=temp;
        exeSQL(sql);//��ÿ����ļ�¼�����뵽��ṹ��
    }

    return _SUCCESS;
}

/**********************************************************
func:   �鿴ĳ�ļ��Ƿ���Ϊδ�ϴ��飬�ͻ��˵���
        ÿ�β鿴������п鱻ռ�ã�ȥ��ռ��ʱ�䣬��ʱ��ÿ�Ҳ�ڿ����б�
input:  string MD5 �ļ���Ψһ��ʶ
output: int 
        ���ļ�����δ�ϴ��飬����ֵΪ��һ����Ҫ�ϴ��Ŀ�ţ����޸�ifusingΪISUSING
        ���� ���ļ����п��Ѿ����ϴ�����������NOBLOCK
             ���ļ������п�δ���ϴ��������Ƕ����ڱ��ϴ�������WAITBLOAK���ȴ�һ��ʱ���������ʹ�������ֱ���ļ��ϴ��������Զ����˳��ϴ���
***********************************************************/
int MyDB::next_block(string MD5)
{
    char temp[tempsize];
    string sql;
    sprintf(temp,"SELECT * FROM file_block WHERE MD5=\"%s\";",MD5.data());
    sql=temp;
    exeSQL(sql);//��ȡ������δ���ϴ���ɵĿ�
    int num_fields = mysql_num_fields(result); //��ȡ��������ܹ����ֶ�����������
    int num_rows = mysql_num_rows(result);     //��ȡ��������ܹ�������
    if(num_rows==0)
    {
        return NOBLOCK;
    }
    else
    {
        //��Ҫ��һ�²�ѯ�������Ϊ֮�󻹻�ִ��MySQL���
        file_block block[num_rows];
        for(int i=0;i<num_rows;i++)
        {
            row = mysql_fetch_row(result);//��ȡһ������  
            block[i].MD5=row[0];
            block[i].block_no=atoi(row[1]);
            block[i].ifusing=atoi(row[2]);
        }
        for(int i=0;i<num_rows;i++)
        {
            if(block[i].ifusing == NOUSING)//δ���ϴ�
            {
                //��ifusingλ��1
                sprintf(temp,"UPDATE file_block SET ifusing=1 where MD5=\"%s\" AND block_no=%d;",MD5.data(),block[i].block_no);
                sql=temp;
                exeSQL(sql);

                //��¼�û���ʼ�ϴ���ʱ��
                long starttime=getCurrentTimeMsec();
                sprintf(temp,"UPDATE file_block SET usingtime=%ld where MD5=\"%s\" AND block_no=%d;",starttime,MD5.data(),block[i].block_no);
                sql=temp;
                exeSQL(sql);
            
                return block[i].block_no;
            }
            else//�����ڱ������û��ϴ�
            {
                //�鿴�Ƿ��Ѿ���ʱ
                long nowtime=getCurrentTimeMsec();//��ǰʱ��
                long starttime;//��ʼ�ϴ�ʱ��
                long continuetime;//��һ���û��Ѿ��ϴ�ʱ��
                //��ѯ��ʼ�ϴ�ʱ��
                sprintf(temp,"SELECT usingtime FROM file_block where MD5=\"%s\" AND block_no=%d;",MD5.data(),block[i].block_no);
                sql=temp;
                exeSQL(sql);

                row = mysql_fetch_row(result);
                starttime=atol(row[0]);

                //����ÿ����ϴ�ʱ��
                continuetime=nowtime-starttime;
                if(continuetime<ALARM)//��δ��ʱ����������
                    continue;
                else//�Ѿ���ʱ�������ϴ�ʱ�䣬���ҿ�����ΪҪ���û��ϴ�����һ����
                {
                    sprintf(temp,"UPDATE file_block SET usingtime=%ld where MD5=\"%s\" AND block_no=%d;",nowtime,MD5.data(),block[i].block_no);
                    sql=temp;
                    exeSQL(sql);

                    return block[i].block_no;
                }
                
            }
        }
        return WAITBLOCK;//��Ȼ���п�δ���꣬�������������̸���
    }
}

/**********************************************************
func:   �û��ϴ�һ����ɹ���ɾ��file_block�еĿ��¼���ͻ��˵���
input:  file_block block �ṹ��
            string MD5 �ļ���Ψһ��ʶ��
            int block_no ���
output: int 
        ɾ���ɹ�����_SUCCESS
        ɾ��ʧ�ܷ���_ERROR
***********************************************************/
int MyDB::upblock_success(file_block block)
{
    char temp[tempsize];
    string sql;
    sprintf(temp,"DELETE FROM file_block WHERE MD5=\"%s\" AND block_no=%d;",block.MD5.data(),block.block_no);
    sql=temp;
    int flag=exeSQL(sql);//ɾ��file_block����Ӧ���¼

    return flag;
}

/**********************************************************
func:   �û��ϴ�һ����ʧ�ܣ�����file_block�еĿ��¼�����û�ռ��λ��գ��ͻ��˵���
input:  file_block block �ṹ��
            string MD5 �ļ���Ψһ��ʶ��
            int block_no ���
output: int 
        ���óɹ�����_SUCCESS
        ����ʧ�ܷ���_ERROR
***********************************************************/
int MyDB::upblock_failed(file_block block)
{
    char temp[tempsize];
    string sql;
    sprintf(temp,"UPDATE file_block SET ifusing = %d WHERE MD5=\"%s\" AND block_no=%d;",NOUSING,block.MD5.data(),block.block_no);
    sql=temp;
    int flag=exeSQL(sql);//��file_block����Ӧ���¼��ΪNOUSING

    return flag;
}

/**********************************************************
func:   ���ص�ǰ�ļ��Ѿ�����Ŀ��������ͻ���
input:      string MD5 �ļ���Ψһ��ʶ��
            int block_num �ļ�һ���Ŀ���
output: int 
        ������ɵĿ���
***********************************************************/
int MyDB::finished_block(string MD5,int block_num)
{
    char temp[tempsize];
    string sql;
    sprintf(temp,"SELECT block_no from file_block WHERE MD5 = \"%s\";",MD5.data());
    sql=temp;
    exeSQL(sql);
    int num_rows = mysql_num_rows(result);     //��ȡ��������ܹ�������

    int finish=block_num-num_rows;

    return finish;
}

