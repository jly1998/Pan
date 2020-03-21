#include "mysql_select.h"


/**********************************************************
func:   为每一个新建用户建立一个默认文件目录
        此时尚未有文件，所做的操作只是向file_format中添加两个文件夹
input:  string user
output: int
        建立成功返回_SUCCESS
        建立失败返回_ERROR
***********************************************************/
int MyDB::init_catalog(string user)
{

    cout<<"现在开始建立默认目录"<<endl;
    //获取当前时间
    NowDate date = getTime();
    char temp[1024];
    string inst;
    int flag;
    char tmp_path[tempsize]; //存放文件路径
    string filename;
    //user的根目录
    sprintf(tmp_path, "/%s", user.data());

    //在file_format中建立根目录
    sprintf(temp, "INSERT file_format(filename,username,create_date,parentid,isfolder,filepath) VALUES (\"%s\",\"%s\",\"%s\",0,%d,\"%s\")", user.data(), user.data(), date.tmp0, ISCATALOG,tmp_path);
    inst = temp;
    //执行插入指令
    flag = exeSQL(inst);
    if (flag == _ERROR)
    {
        return _ERROR;
    }

    //找到根目录的文件号
    sprintf(temp, "SELECT fileid FROM file_format WHERE filepath=\"%s\";", tmp_path);
    inst = temp;
    exeSQL(inst);
    row = mysql_fetch_row(result);
    int root = atoi(row[0]);

    //新建文件夹“我的资源”
    filename = PathResource;
    sprintf(temp, "INSERT file_format(filename,username,create_date,parentid,isfolder,filepath) VALUES (\"%s\",\"%s\",\"%s\",%d,%d,\"%s/%s\")", filename.data(), user.data(), date.tmp0, root,ISCATALOG, tmp_path, filename.data());
    inst = temp;
    //执行插入指令
    flag = exeSQL(inst);
    if (flag == _ERROR)
    {
        return _ERROR;
    }

    //新建文件夹“我的应用”
    filename = PathApp;
    sprintf(temp, "INSERT file_format(filename,username,create_date,parentid,isfolder,filepath) VALUES (\"%s\",\"%s\",\"%s\",%d,%d,\"%s/%s\")", filename.data(), user.data(), date.tmp0, root,ISCATALOG, tmp_path, filename.data());
    inst = temp;
    //执行插入指令
    flag = exeSQL(inst);
    if (flag == _ERROR)
    {
        return _ERROR;
    }

    return _SUCCESS;
}

/**********************************************************
func:   用户上传一份文件，首先查询file_store；
        如果命中，更新用户数；否则，在file_store中插入相应表记录
input:  string path 文件路径
        string MD5 文件MD5值
        int filesize 文件大小
output: bool
        命中返回true；否则返回false
***********************************************************/
bool MyDB::ifexist_store(string path, string MD5, int filesize)
{
    char temp[tempsize];
    string sql;
    sprintf(temp, "SELECT memid FROM file_store where MD5=\"%s\";", MD5.data());
    sql = temp;
    exeSQL(sql); //此时result中返回查询的结果集
    //如果查询为空，行数为0
    int num_rows = mysql_num_rows(result); //获取结果集中总共的行数
    //文件不存在，在file_stores中插入相应信息
    if (num_rows == 0)
    {
        sprintf(temp, "INSERT file_store(path,MD5,filesize,usernum) VALUES(\"%s\",\"%s\",%d,%d)", path.data(), MD5.data(), filesize, 1);
        sql = temp;
        exeSQL(sql); //向file_store中插入一条新记录
        return false;
    }
    //文件存在，更新file_store中的usernum
    else
    {
        sprintf(temp, "UPDATE file_store SET usernum = usernum + 1 where MD5=\"%s\"", MD5.data());
        sql = temp;
        exeSQL(sql); //向file_store中个更新记录
        return true;
    }
    //命中与否都要在file_format中添加一条记录
}

/**********************************************************
func:   同用户同文件夹下是否有重名；
input:  struct file_format
        有效值  filepath
output: bool
        有重名返回false，无重名返回true
***********************************************************/
bool MyDB::ifsame_filename(file_format file)
{
    char temp[tempsize];
    string sql;
    // sprintf(temp, "SELECT * FROM file_format WHERE filename=\"%s\" and username=\"%s\" and parentid=%d;", file.filename.data(), file.username.data(), file.parentid);
    sprintf(temp, "SELECT * FROM file_format WHERE filepath=\"%s\" ;", file.filepath.data());
    sql = temp;
    exeSQL(sql);
    //如果查询为空，行数为0
    int num_rows = mysql_num_rows(result); //获取结果集中总共的行
    if (num_rows == 0)                     //查询结果为0，说明无重名(尚未插入自己)
        return true;
    else
        return false;
}

/**********************************************************
func:   文件名的重命名处理问题，追加数字以示区分
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
        //无重名
        if (ifsame_filename(file) == true)
        {
            cout<<"filepath:"<<filepath<<endl;
            break;
        }
        else
        {
            //给文件名追加数字再次判断是否重名
            sprintf(temp, "%s%d", init_filepath.data(), i);
            file.filepath = temp;
            filepath=file.filepath;
        }
    }

    return;
}

/**********************************************************
func:   根据路径名得到一个文件夹 编号；
input:  string path 基本格式类似于"/fenghui/jisuanji"
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
    exeSQL(sql); //一个路径只能对应一个文件号
    row = mysql_fetch_row(result);
    if(row==NULL)
        return _ERROR;
    fileid = atoi(row[0]);
    cout << "path_to_id:" << fileid << endl;
    return fileid;
}

/**********************************************************
func:   根据编号从数据库中查询到一个file_format返回
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
    //将数据库查询到的内容转换成file_format结构体格式
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
func:   判断文件2是不是文件1的子文件
input:  string id1
        string id2
output: bool
        如果是返回true
        不是返回false
***********************************************************/
bool MyDB::i_am_your_father(string id1,string id2)
{
    //只需要判断id1是否是id2的前缀
    //查抄子串无结果，说明不是子文件夹
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
func:   根据路径名得到一个file_format结构体；
input:  string path 文件路径，含用户名
        char *MD5 文件MD5值
        int isfolder 是否是文件夹
output: file_format
        有效值   username
                filename
                filepath(带用户名，带路径名，数据库格式)
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
    //int flag;   //返回值

    char split[2] = "/";
    char *p;
    char temp[tempsize]; //用char*类型存储stirng类型
    sprintf(temp, path.data());
    p = strtok(temp, split); //获取到的第一个字符串是username；
    /****************叮咚~GET！username******************/
    file.username = p;
    cout << "username:" << file.username << endl;
    while (p != NULL)
    {
        layer++;
        cout << p << endl;
        p = strtok(NULL, split);
    }
    //用i记录目录层数，得到的i-1为文件的父目录parent
    sprintf(temp, path.data());
    string parent_path;
    p = strtok(temp, split);
    for (i = 0; i < layer; i++)
    {
        if (i == layer - 1)
        {
            /****************叮咚~GET！filename******************/
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
    /****************叮咚~GET！filepath******************/
    file.filepath = parent_path;
    file.filepath.append("/");
    file.filepath.append(file.filename);
    /****************叮咚~GET！parentid******************/
    file.parentid = path_to_id(parent_path);
    /****************叮咚~GET！MD5******************/
    if (isfolder == ISFILE)
        file.MD5 = MD5;
    /****************叮咚~GET！isfolder******************/
    file.isfolder = isfolder;
    /****************叮咚~GET！create_date******************/
    NowDate date = getTime();
    memcpy(&(file.create_date), &(date), sizeof(NowDate));

    cout << "path_to_format is over" << endl;
    return file;
}

/**********************************************************
func:   用户上传一份文件，与文件夹中已有文件重名
        根据其在file_bloak中是否还有未上传完成的块，来判断该文件的上传是否是断点续传，
        若是，删除之前在file_format中建立的文件表记录
            并把file_store中的usernum--
input:  string path 文件路径（含文件名，含用户名
        string 文件MD5值
output: int 标志位
            FINISHED    文件已经被上传完成/新文件
            CONTINUE     文件尚未被上传完成，删除相应的表记录
***********************************************************/
int MyDB::if_finished(string path,string MD5)
{
    char temp[tempsize];
    string sql;
    //查看该文件是否还有尚未被上传的块
    sprintf(temp,"SELECT * FROM file_block WHERE MD5=\"%s\";",MD5.data());
    sql=temp;
    exeSQL(sql);
    int num_rows = mysql_num_rows(result);     //获取结果集中总共的行数
    if(num_rows==0)//文件已经被上传完成
    {
        return FINISHED;
    }
   // cout<<"删除文件咯"<<endl;
    //文件没有被上传完成，那这条文件记录不应当出现在file_format中，删掉
    sprintf(temp,"DELETE FROM file_format WHERE filepath=\"%s\";",path.data());
    sql=temp;
    exeSQL(sql);

    //同时file_store中的usernum--
    sprintf(temp,"UPDATE file_store SET usernum=usernum-1 WHERE MD5=\"%s\";",MD5.data());
    sql=temp;
    exeSQL(sql);
    
    return CONTINUE;
}


/**********************************************************
func:   用户上传一份文件，更新file_format,file_store
        当文件重名，判断是否被上传完，若是，重名；若不是，断点续传
input:  string path 文件路径（含文件名，含用户名
        string MD5 文件MD5值
        int isfolder 标志位，是否是文件夹
        int filesize 文件大小
        int block_num 块数
output: int 标志位
        _SUCCESS    在file_format中插入相应记录
                    在file_store中插入相应记录
        <0 出现error，file_format无效
            SAMENAME 文件重名
            _ERROR 该文件的父目录尚未建立
***********************************************************/
int MyDB::upload_file(string path, string MD5, int isfolder, int filesize,int block_num)
{
    file_format file = path_to_format(path, MD5, isfolder);
    int flag;
    char temp[tempsize];
    string sql;

    //新建的文件与已有文件重名
    if (ifsame_filename(file) == false)
    {
        //文件重名
        if(isfolder==ISFILE)
        {
            //文件已经被上传成功
            int flag=if_finished(path,MD5);
            if(flag==FINISHED)
                return SAMENAME;
        }
        //文件夹重名
        else
        {
            //直接返回重名
            return SAMENAME;
        }
        
    }

    //无重名 先在file_format中插入一条表记录
    //判断父目录是否存在
    if(file.parentid==-1)//该文件的父目录尚未建立
    {
        cout<<"空中楼阁：该文件的父目录尚未建立"<<endl;
        return _ERROR;
    }
    //文件的父目录已经建立
    sprintf(temp, "INSERT file_format(filename,username,create_date,MD5,parentid,isfolder,nowsize,filepath) VALUES(\"%s\",\"%s\",\"%s\",\"%s\",%d,%d,%d,\"%s\")", file.filename.data(), file.username.data(), file.create_date.tmp0, file.MD5.data(), file.parentid, file.isfolder,filesize, file.filepath.data());
    sql = temp;
    exeSQL(sql); //此时result中返回插入的结果集
    //再判断MD5值是否已经存在
    //如果是文件夹，需要递归判断每个文件并执行插入表记录工作
    if (file.isfolder == ISCATALOG)
    {
        //获取子目录下的每个文件状态并改变相应的记录
    }
    //如果是文件，直接判断该文件
    else
    {
        //查询file_store表，命中更新usernum，未命中添加表记录
        flag=ifexist_store(file.filepath, file.MD5, filesize);
        //未命中，在file_block中插入块记录
        cout<<"ifexist_store return "<<flag<<endl;
        if (flag == false)
        {
            init_block(file.MD5,block_num);
        }
    }
    return _SUCCESS;
}

/**********************************************************
func:   用户上传一份文件，更新file_format；
        相当于封装了upload_file
input:  file_format 来自客户端的结构体，按照相关约定
        username 
        filepath
        isfolder 
        MD5         *
        filesize    *
        block_num   *   只有文件需要
output: 与upload_file的返回值相同
        int 标志位
        _SUCCESS    在file_format中插入相应记录
                    在file_store中插入相应记录
        <0 出现error，file_format无效
            SAMENAME 文件重名
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
func:   删除一份文件，不删除物理内存
        更改两份表，删除file_format中的记录，更新file_store
input:  string path 路径包含用户名（根目录
output: void
***********************************************************/
void MyDB::delete_file(string path)
{
    //在这里调用的MD5与is_folder均为无效，只是为了得到“加工”后的filepath
    file_format file = path_to_format(path, "0000", 1);
    char temp[tempsize];
    string sql;
    string MD5;
    int fileid;
    sprintf(temp, "SELECT fileid FROM file_format where filepath=\"%s\";", file.filepath.data());
    sql = temp;
    exeSQL(sql); //此时result中返回查询的结果集
    //获取下一行数据
    row = mysql_fetch_row(result);
    //正常一个路径映射到一个fileid上
    int num_rows = mysql_num_rows(result); //获取结果集中总共的行数
    if (num_rows == 0)
        return;
    fileid = atoi(row[0]);

    sprintf(temp, "SELECT MD5 FROM file_format where fileid=%d;", fileid);
    sql = temp;
    exeSQL(sql); //此时result中返回查询的结果集
    //获取下一行数据
    row = mysql_fetch_row(result);
    //正常一个文件映射到一个MD5码上
    MD5 = row[0];
    //删除file_format中的表记录
    sprintf(temp, "DELETE FROM file_format where fileid=%d;", fileid);
    sql = temp;
    exeSQL(sql);
    //file_store中用户占有数--
    sprintf(temp, "UPDATE file_store SET usernum = usernum-1 where MD5=\"%s\" and usernum > 0;", MD5.data());
    sql = temp;
    exeSQL(sql);

    return;
}

/**********************************************************
func:   用户删除文件夹，更新file_format；
input:  string path 路径包含用户名（根目录
output: int 标志位
        _SUCCESS    在file_format中更新相应记录
        _ERROR      文件不存在或其他错误
***********************************************************/
int MyDB::delete_catalog(string path)
{
    char temp[tempsize];
    string sql;
    int fileid;

    //找到文件夹fileid
    sprintf(temp,"SELECT fileid FROM file_format WHERE filepath=\"%s\";",path.data());
    sql=temp;
    exeSQL(sql);
    //将ffile.fileid填充
    if(row==NULL)//路径不存在？
        return _ERROR;
    row = mysql_fetch_row(result);
    fileid=atoi(row[0]);

    //更新file_format，删除本条文件夹表记录
    //文件夹不可能出现在file_store中
    sprintf(temp,"DELETE FROM file_format WHERE fileid=%d;",fileid);
    //cout<<"delete catalog:"<<fileid<<endl;
    sql=temp;
    exeSQL(sql);

    //找到该文件夹下的子文件
    sprintf(temp,"SELECT * FROM file_format WHERE parentid=%d;",fileid);
    sql=temp;
    exeSQL(sql);

    //递归查询不能使用公共空间，应当采用局部变量存储每次result的结果
    int num_fields = mysql_num_fields(result); //获取结果集中总共的字段数，即列数
    int num_rows = mysql_num_rows(result);     //获取结果集中总共的行数

    //该文件夹为空
    if(num_rows==0)
        return _SUCCESS;
    string temp_result[num_rows][num_fields];
     //每一行 搬运result至temp_result
    for (int i = 0; i < num_rows; i++)        
    {
        //获取下一行数据
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
        //"/用户名/父目录/文件名"
        sprintf(temp,"%s/%s",path.data(),temp_result[i][1].data());
        path_c=temp;
        //cout<<"下一个删除文件"<<path_c<<endl;
        if(atoi(temp_result[i][6].data())==ISCATALOG)//文件夹，递归操作
        {
           // cout<<"删除下一个文件夹"<<endl;
            delete_catalog(path_c);
        }
        else
        {
            //直接调用delete_file函数
            delete_file(path_c);
        }
    }

    return _SUCCESS;
}

/**********************************************************
func:   删除一份文件，不删除物理内存
        更改两份表，删除file_format中的记录，更新file_store
        相当于封装了delete_file和delete_catalog
input:  file_format file 
                来自客户端的结构体中的有效数据只有fileid
                需要内部自己转换为结构体
        有效值：int fileid
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

    //删除文件
    if(real_file.isfolder==ISFILE)
    {
        //cout<<"real_file.filepath:"<<real_file.filepath<<endl;
        delete_file(real_file.filepath);
    }
    //删除文件夹
    else
    {
        delete_catalog(real_file.filepath);
    }

    return;
}

/**********************************************************
func:   用户移动文件，更新file_format；
        若影响到file_store（文件存储目录以第一份上传为准），应同步更新
input:  string from_path 移动前的路径名，含文件名，含用户名
        string to_path 移动后的路径名，含文件名，含用户名
        string username 用户名
output: int 标志位
        _SUCCESS    在file_format中更新相应记录
        <0 出现error，file_format无效
            SAMENAME 文件重名
***********************************************************/
int MyDB::move_file(string from_path,string to_path,string username)
{
    char temp[tempsize];
    string sql;
    string temp_fpath;//补上用户名之后的from_path
    string temp_tpath;//补上用户名之后的to_path
    file_format tfile;
    file_format ffile;
    bool flag;
    //sprintf(temp,"/%s%s",username.data(),from_path.data());
    temp_fpath=from_path;
    //sprintf(temp,"/%s%s",username.data(),to_path.data());
    temp_tpath=to_path;
    //这里的MD5与is_folder均无效，是为了得到to_path转换出来的file_format结构
    //有效值有filename，username，parentid
    //得到符合数据库结构要求的filepath
    //为了给ifsame_filename传参
    ffile=path_to_format(temp_fpath,"0000",1);
    tfile=path_to_format(temp_tpath,"0000",1);
    flag=ifsame_filename(tfile);
     
    // cout<<endl;
    // cout<<"temp_tpath.data()"<<temp_tpath.data()<<endl;
    // cout<<"ffile.filepath.data():"<<ffile.filepath.data()<<endl;
    // cout<<"tfile.filepath.data():"<<tfile.filepath.data()<<endl;

    if(flag==true)//无重名，可直接更改file_format的filepath
    {
        //更新file_format的filepath和parentid
        sprintf(temp,"UPDATE file_format SET parentid=%d WHERE filepath=\"%s\";",tfile.parentid,ffile.filepath.data());
        sql=temp;
        exeSQL(sql);

        sprintf(temp,"UPDATE file_format SET filepath=\"%s\" WHERE filepath=\"%s\";",tfile.filepath.data(),ffile.filepath.data());
        sql=temp;
        exeSQL(sql);
        
        //更新file_store
        sprintf(temp,"UPDATE file_store SET path=\"%s\" WHERE path=\"%s\";",tfile.filepath.data(),ffile.filepath.data());
        sql=temp;
        exeSQL(sql);
    
        return _SUCCESS;
    }
    else//重名，直接返回，移动失败
    {
        return SAMENAME;
    }
}

/**********************************************************
func:   用户移动文件夹，更新file_format；
        若影响到file_store（文件存储目录以第一份上传为准），应同步更新
input:  string from_path 移动前的路径名，含文件名，含用户名
        string to_path 移动后的路径名，含文件名，含用户名
        string username 用户名
output: int 标志位
        _SUCCESS    在file_format中更新相应记录
        <0 出现error，file_format无效
            SAMENAME 文件重名
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
    //这里的MD5无效，isfolde或许有效，为了得到to_path转换出来的file_format结构：filename,parentid,filepath,username
    ffile=path_to_format(temp_fpath,"0000",1);
    tfile=path_to_format(temp_tpath,"0000",1);

    //首先判断是否重名，是否有同名文件夹/文件
    int flag=ifsame_filename(tfile);
    if(flag==false)//有重名
        return SAMENAME;
    
    //无重名
    //找到文件夹fileid
    sprintf(temp,"SELECT fileid FROM file_format WHERE filepath=\"%s\";",ffile.filepath.data());
    sql=temp;
    exeSQL(sql);
    //将ffile.fileid填充
    if(row==NULL)//路径不存在？
        return _ERROR;
    row = mysql_fetch_row(result);
    ffile.fileid=atoi(row[0]);
    tfile.fileid=atoi(row[0]);

    //更新file_format的filepath和parentid
    //文件夹不可能出现在file_store中
    sprintf(temp,"UPDATE file_format SET parentid=%d WHERE filepath=\"%s\";",tfile.parentid,ffile.filepath.data());
    sql=temp;
    exeSQL(sql);

    sprintf(temp,"UPDATE file_format SET filepath=\"%s\" WHERE filepath=\"%s\";",tfile.filepath.data(),ffile.filepath.data());
    sql=temp;
    exeSQL(sql);

    //找到该文件夹下的子文件
    sprintf(temp,"SELECT * FROM file_format WHERE parentid=%d;",ffile.fileid);
    sql=temp;
    exeSQL(sql);

    //递归查询不能使用公共空间，应当采用局部变量存储每次result的结果
    int num_fields = mysql_num_fields(result); //获取结果集中总共的字段数，即列数
    int num_rows = mysql_num_rows(result);     //获取结果集中总共的行数
    //该文件夹为空
    if(num_rows==0)
        return _SUCCESS;
    string temp_result[num_rows][num_fields];
     //每一行 搬运result至temp_result
    for (int i = 0; i < num_rows; i++)        
    {
        //获取下一行数据
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
        //"父目录/文件名"
        sprintf(temp,"%s/%s",from_path.data(),temp_result[i][1].data());
        from_path_c=temp;
        sprintf(temp,"%s/%s",to_path.data(),temp_result[i][1].data());
        to_path_c=temp;

        if(temp_result[i][6]=="1")//文件夹，递归操作
        {
            move_catalog(from_path_c,to_path_c,username);
        }
        else
        {
            //直接调用move_file函数
            move_file(from_path_c,to_path_c,username);
        }
    }

    // for (int i = 0; i < num_rows; i++)         //每一行
    // {
    //     for (int j = 0; j < num_fields; j++) //输出每一字段
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
func:   移动文件/夹
        相当于封装了move_file和move_catalog
input:  int from_id 起始文件id
        int to_id 终止路径的文件夹id
                需要内部自己转换为结构体
output: 文件/夹移动结果
***********************************************************/
int MyDB::sealed_move(int from_id,int to_id)
{
    file_format from_file;
    file_format to_file;
    //根据id得到相应的结构体
    //有效值有 filepath(含用户名的起始文件路径)
    //        username(用户名)
    //        filename(文件名)
    from_file=id_to_format(from_id);
    //有效值，移动后的路径的父目录
    to_file=id_to_format(to_id);

    char temp_path[tempsize];
    string path;
    sprintf(temp_path,"%s/%s",to_file.filepath.data(),from_file.filename.data());
    path=temp_path;


    if(to_file.isfolder==ISFILE)
    {
        cout<<"移动的目标路径是文件是不被允许的哦！"<<endl;
        return WRONGTOFILE;
    }

    //移动文件
    if(from_file.isfolder==ISFILE)
    {
        //cout<<"real_file.filepath:"<<real_file.filepath<<endl;
        return move_file(from_file.filepath,path,from_file.username);
    }
    //移动文件夹
    else
    {
        if(i_am_your_father(from_file.filepath,path)==true)
        {
            cout<<"致命错误：现在要将文件夹移动到他子文件夹里面了哦！"<<endl;
            return MOVETOSON;
        }
         return move_catalog(from_file.filepath,path,from_file.username);
    }

}

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
int MyDB::copy_file(string username,string from_path,string to_path)
{
    char temp[tempsize];
    string sql;
    string temp_fpath;//补上用户名之后的from_path
    string temp_tpath;//补上用户名之后的to_path
    file_format tfile;
    file_format ffile;
    bool flag;
    //sprintf(temp,"/%s%s",username.data(),from_path.data());
    temp_fpath=from_path;
    //sprintf(temp,"/%s%s",username.data(),to_path.data());
    temp_tpath=to_path;
    
    //找到文件的MD5值
    sprintf(temp,"SELECT MD5 FROM file_format WHERE filepath=\"%s\";",temp_fpath.data());
    sql=temp;
    exeSQL(sql);
    row = mysql_fetch_row(result);
    if(row==NULL)
    {
        return _ERROR;
    }
    string temp_MD5=row[0];

    //得到符合数据库结构要求的filepath
    //为了给ifsame_filename传参
    ffile=path_to_format(temp_fpath,temp_MD5,0);
    tfile=path_to_format(temp_tpath,temp_MD5,0);
    flag=ifsame_filename(tfile);
    if(flag==false)//重名
    {
        return SAMENAME;
    }
    else//无重名
    {
        //向file_format中插入一条新记录
        sprintf(temp,"INSERT file_format(filename,username,create_date,MD5,parentid,isfolder,filepath) VALUES(\"%s\",\"%s\",\"%s\",\"%s\",%d,%d,\"%s\");",tfile.filename.data(),tfile.username.data(),tfile.create_date.tmp0,tfile.MD5.data(),tfile.parentid,tfile.isfolder,tfile.filepath.data());
        sql=temp;
        exeSQL(sql);
        
        //在file_store中存储的相应文件的用户占有数++
        sprintf(temp,"UPDATE file_store SET usernum=usernum+1 WHERE MD5=\"%s\";",temp_MD5.data());
        sql=temp;
        exeSQL(sql);

        return _SUCCESS;
    } 
}

/**********************************************************
func:   复制文件夹，递归查找在file_format中添加若干记录
input:  string username
        string from_path 移动前的路径名，含文件名，含用户名
        string to_path 移动后的路径名，含文件名，含用户名
output: int
        重名返回SAMENAME
        失败返回_ERROR
        不重名且复制成功返回_SUCCESS
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
    //这里的MD5无效，isfolde或许有效，为了得到to_path转换出来的file_format结构：filename,parentid,filepath,username
    ffile=path_to_format(temp_fpath,"0000",1);
    tfile=path_to_format(temp_tpath,"0000",1);

    //首先判断是否重名，是否有同名文件夹/文件
    int flag=ifsame_filename(tfile);
    if(flag==false)//有重名
        return SAMENAME;
    
    //无重名
    //找到文件夹fileid
    sprintf(temp,"SELECT fileid FROM file_format WHERE filepath=\"%s\";",ffile.filepath.data());
    sql=temp;
    exeSQL(sql);
    //将ffile.fileid填充
    if(row==NULL)//路径不存在？
        return _ERROR;
    row = mysql_fetch_row(result);
    ffile.fileid=atoi(row[0]);
    tfile.fileid=atoi(row[0]);

    //在file_format中插入一条新记录
    //文件夹不可能出现在file_store中
    sprintf(temp,"INSERT file_format(filename,username,create_date,parentid,isfolder,filepath) VALUES(\"%s\",\"%s\",\"%s\",%d,%d,\"%s\");",tfile.filename.data(),tfile.username.data(),tfile.create_date.tmp0,tfile.parentid,tfile.isfolder,tfile.filepath.data());
    sql=temp;
    exeSQL(sql);

    //找到该文件夹下的子文件
    sprintf(temp,"SELECT * FROM file_format WHERE parentid=%d;",ffile.fileid);
    sql=temp;
    exeSQL(sql);

    //递归查询不能使用公共空间，应当采用局部变量存储每次result的结果
    int num_fields = mysql_num_fields(result); //获取结果集中总共的字段数，即列数
    int num_rows = mysql_num_rows(result);     //获取结果集中总共的行数
    //该文件夹为空
    if(num_rows==0)
        return _SUCCESS;
    string temp_result[num_rows][num_fields];
     //每一行 搬运result至temp_result
    for (int i = 0; i < num_rows; i++)        
    {
        //获取下一行数据
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
        //"/用户名/父目录/文件名"
        sprintf(temp,"%s/%s",from_path.data(),temp_result[i][1].data());
        from_path_c=temp;
        sprintf(temp,"%s/%s",to_path.data(),temp_result[i][1].data());
        to_path_c=temp;

        if(temp_result[i][6]=="1")//文件夹，递归操作
        {
            copy_catalog(username,from_path_c,to_path_c);
        }
        else
        {
            //直接调用copy_file函，复制文件
            copy_file(username,from_path_c,to_path_c);
        }
    }

    return _SUCCESS;   

}

/**********************************************************
func:   复制文件/夹
        相当于封装了move_file和move_catalog
        同文件夹下复制会追加数字区分文件名
input:  int from_id 起始文件id
        int to_id 终止路径的文件夹id
                需要内部自己转换为结构体
output: 文件/夹复制结果
***********************************************************/
int MyDB::sealed_copy(int from_id,int to_id)
{
    file_format from_file;
    file_format to_file;
    //根据id得到相应的结构体
    //有效值有 filepath(含用户名的起始文件路径)
    //        username(用户名)
    //        filename(文件名)
    from_file=id_to_format(from_id);
    //有效值，移动后的路径的父目录
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
        cout<<"复制的目标路径是文件是不被允许的哦！"<<endl;
        return WRONGTOFILE;
    }
    //复制文件
    if(from_file.isfolder==ISFILE)
    {
        //同文件夹下复制
        if(from_file.filepath==path)
        {
            //在文件后面后缀数字避免重名
            newfilename_deal(path);
        }
        //cout<<"real_file.filepath:"<<real_file.filepath<<endl;
         return copy_file(from_file.username,from_file.filepath,path);
    }
    //复制文件夹
    else
    {
        //同文件夹下复制
        if(from_file.filepath==path)
        {
            //在文件后面后缀数字避免重名
            newfilename_deal(path);
        }
        return copy_catalog(from_file.username,from_file.filepath,path);
    }

}


/**********************************************************
func:   用户传来一个路径，通过地址去查询数据库，返回给用户该层表结构
input:  file_format 来自客户端的结构体，按照相关约定
        有效值   filepath
                username
output: int 返回查询到的结果的行数，
        如果是文件，返回-1
        如果是文件夹，返回目录下的文件数（包括文件夹
                    用于动态申请file_format数组的大小
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
    //这里的MD5值与isfolder均无意义，目的是为了得到符合数据库结构的filepath
    file_format temp_file = path_to_format(path, "0000", 1);
    //查询该目录结构的id号
    int fileid = path_to_id(temp_file.filepath.data());
    sprintf(temp, "SELECT * FROM file_format WHERE parentid = %d;", fileid);
    sql = temp;
    exeSQL(sql); //此时查询结果已按照file_format的形式插入到结果集中

    int num_rows = mysql_num_rows(result); //获取结果集中总共的行数

    return num_rows;
}

/**********************************************************
func:   去掉路径前的用户名，传送给客户端
input:  string path
output: string 没有用户名的路径
***********************************************************/
string nouser_path(char *path)
{
    char split[2] = "/";
    char *p;
    char temp[tempsize]; //用char*类型存储stirng类型
    int layer = 0;
    string opath="/";
    strcpy(temp, path);
    p = strtok(temp, split); //获取到的第一个字符串是username；
    while (p != NULL)
    {
        layer++;
        p = strtok(NULL, split);
    }
    //用i记录目录层数，得到的i-1为文件的父目录parent
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
func:   此函数与find_content(const file_format file)配套使用,
        此时result中已存储了查询结果
input:  file_format* 数组，即该目录下的文件结构，由该函数写入  
output: int 返回查询到的结果的行数，用于动态申请file_format数组的大小
***********************************************************/
void MyDB::store_format(file_format *file)
{
    int num_fields = mysql_num_fields(result); //获取结果集中总共的字段数，即列数
    int num_rows = mysql_num_rows(result);     //获取结果集中总共的行数
    for (int i = 0; i < num_rows; i++)         //将每一行黏贴到content中
    {
        //获取下一行数据
        row = mysql_fetch_row(result);
        if (row ==NULL)
            break;
        //把结果赋给file_format[i]
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
    //     cout<<"file[i]：fileid:"<<file[i].fileid<<endl;
    //     cout<<"file[i]：filename:"<<file[i].filename<<endl;
    //     cout<<"file[i]：username:"<<file[i].username<<endl;
    //     cout<<"file[i]：create_date:"<<file[i].create_date.tmp0<<endl;
    //     cout<<"file[i]：MD5:"<<file[i].MD5<<endl;
    //     cout<<"file[i]：parentid:"<<file[i].parentid<<endl;
    //     cout<<"file[i]：isfolder:"<<file[i].isfolder<<endl;
    //     cout<<"file[i]：filesize:"<<file[i].filesize<<endl;
    //     cout<<"file[i]：filepath:"<<file[i].filepath<<endl;
    //     cout<<endl;
    // }
    return;
}

/**********************************************************
func:   查询重命名，是否可以成功，给客户端用
        同时更新file_format和file_store
input:  file_format 
        有效值  username
                filepath 路径来自于客户端，没有username
        string newname
output: int 返回操作结果
            _SUCCESS 成功重命名并更改file_format中相应的表记录
            SAMENAME 发生重名问题，重命名失败
***********************************************************/
int MyDB::ifrename(file_format file, string newname)
{
    file_format temp_file;
    string temp_path;
    string sql;
    char temp[tempsize];
    bool flag;
    //拼接得到需要的格式
    sprintf(temp, "/%s%s", file.username.data(), file.filepath.data());
    temp_path = temp;
    //这里的MD5和isfolder同样无意义，目的是得到parentid，filepath，username
    temp_file = path_to_format(temp_path, "0000", 1);
    temp_file.filename = newname; //替换文件名即可
    flag = ifsame_filename(temp_file);
    if (flag == false) //有重名
    {
        return SAMENAME;
    }
    else //无重名则要更新表记录file_format中的filename和filepath
    {
        sprintf(temp, "SELECT fileid FROM file_format WHERE filepath= \"%s\"", temp_file.filepath.data());
        sql = temp;
        exeSQL(sql);                   //查询得到fileid
        row = mysql_fetch_row(result); //一个路径对应唯一文件id
        int fileid = atoi(row[0]);

        sprintf(temp, "SELECT filepath FROM file_format WHERE fileid= %d", temp_file.parentid);
        sql = temp;
        exeSQL(sql);                   //查询得到父目录路径
        row = mysql_fetch_row(result); //一个文件id对应唯一路径，带用户名
        temp_path=row[0];//得到父目录路径
        temp_path.append("//");
        temp_path.append(newname);
        
        //更新filepath
        sprintf(temp,"UPDATE file_format SET filepath=\"%s\" WHERE fileid=%d;",temp_path.data(),fileid);
        sql=temp;
        exeSQL(sql);

        //更新filename
        sprintf(temp,"UPDATE file_format SET filename=\"%s\" WHERE fileid=%d;",newname.data(),fileid);
        sql=temp;
        exeSQL(sql);

        //更新file_store里的path
        sprintf(temp,"UPDATE file_store SET path=\"%s\" WHERE path=\"%s\";",temp_path.data(),file.filepath.data());
        sql=temp;
        exeSQL(sql);
    }

    return _SUCCESS;
}

/**********************************************************
func:   根据MD5值，返回给客户文件实际存储路径（带用户名，相当于数据库中存储路径）
        给客户端用
input:  string MD5
        string &filename "newfile"
        string &parentpath "/fenhui/jisuanji"
output: int
        若查询失败，返回_ERROR
        若查询成功，返回_SUCCESS
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
    //查询失败，返回_ERROR
    if(row==NULL)
    {
        return _ERROR;
    }
    //查询成功，分割父目录与文件名
    file_format file;
    string path=row[0];//文件完整路径
    //为了得到parentid以及filename
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
func:   根据用户名，得到该用户的目录结构体数组
        给客户端用
input:  string username
        file_format &*file 结构体数组的引用？
output: int
        查询成功返回查询到的行数
        查询失败返回_ERROR
***********************************************************/
int MyDB::user_filetree(string username,file_format* &file)
{
    char temp[tempsize];
    string sql;
    //查询指定用户的所有文件
    sprintf(temp,"SELECT * FROM file_format WHERE username=\"%s\";",username.data());
    sql=temp;
    exeSQL(sql);

    int num_fields = mysql_num_fields(result); //获取结果集中总共的字段数，即列数
    int num_rows = mysql_num_rows(result);     //获取结果集中总共的行数

    //申请结构体数组空间
    file=new file_format[num_rows];
    if(file==NULL)
        return _ERROR; 
    for (int i = 0; i < num_rows; i++)         //将每一行黏贴到content中
    {
        //获取下一行数据
        row = mysql_fetch_row(result);
        if (row ==NULL)
            break;
        //把结果赋给file_format[i]
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
    //     cout<<"file[i]：fileid:"<<file[i].fileid<<endl;
    //     cout<<"file[i]：filename:"<<file[i].filename<<endl;
    //     cout<<"file[i]：username:"<<file[i].username<<endl;
    //     cout<<"file[i]：create_date:"<<file[i].create_date.tmp0<<endl;
    //     cout<<"file[i]：MD5:"<<file[i].MD5<<endl;
    //     cout<<"file[i]：parentid:"<<file[i].parentid<<endl;
    //     cout<<"file[i]：isfolder:"<<file[i].isfolder<<endl;
    //     cout<<"file[i]：filesize:"<<file[i].filesize<<endl;
    //     cout<<"file[i]：filepath:"<<file[i].filepath<<endl;
    //     cout<<endl;
    // }
    return num_rows;
}



/*************************块操作***************************/

/**********************************************************
func:   初始化一个文件的块，将文件的所有快都插入到file_block中
        块编号从1开始
input:  string MD5 文件的唯一标识
        int block_num 文件的块数
output: int 成功插入返回_SUCCESS
***********************************************************/
int MyDB::init_block(string MD5,int block_num)
{
    char temp[tempsize];
    string sql;
    //int filesize;
    // int block_num=0;//得到文件分好的块数，也是即将插入的表项数
    // //查询文件的大小
    // sprintf(temp,"SELECT filesize FROM file_store WHERE MD5=\"%s\";",MD5.data());
    // sql=temp;
    // exeSQL(sql);
    // //按理说，一个MD5值对应唯一文件，对应一个文件大小，请
    // row = mysql_fetch_row(result);
    // if(row==NULL)
    // {
    //     return _ERROR;
    // }
    // filesize=atoi(row[0]);
    // block_num=filesize/BLOCKSIZE;
    // if(filesize%BLOCKSIZE!=0)//最后一块不一定被写满
    //     block_num++;
    //对file_block执行初始插入操作
    for(int i=1;i<=block_num;i++)
    {
        sprintf(temp,"INSERT file_block(MD5,block_no,ifusing) VALUES(\"%s\",%d,%d);",MD5.data(),i,NOUSING);
        sql=temp;
        exeSQL(sql);//将每个块的记录都插入到表结构中
    }

    return _SUCCESS;
}

/**********************************************************
func:   查看某文件是否还有为未上传块，客户端调用
        每次查看块表，若有块被占用，去查占用时间，超时则该块也在可用列表
input:  string MD5 文件的唯一标识
output: int 
        若文件尚有未上传块，返回值为下一个需要上传的块号，并修改ifusing为ISUSING
        否则 若文件所有块已经被上传结束，返回NOBLOCK
             若文件中仍有块未被上传但是它们都正在被上传，返回WAITBLOAK，等待一段时间后继续发送传包请求，直到文件上传结束或自动“退出上传”
***********************************************************/
int MyDB::next_block(string MD5)
{
    char temp[tempsize];
    string sql;
    sprintf(temp,"SELECT * FROM file_block WHERE MD5=\"%s\";",MD5.data());
    sql=temp;
    exeSQL(sql);//获取所有尚未被上传完成的块
    int num_fields = mysql_num_fields(result); //获取结果集中总共的字段数，即列数
    int num_rows = mysql_num_rows(result);     //获取结果集中总共的行数
    if(num_rows==0)
    {
        return NOBLOCK;
    }
    else
    {
        //需要存一下查询结果，因为之后还会执行MySQL语句
        file_block block[num_rows];
        for(int i=0;i<num_rows;i++)
        {
            row = mysql_fetch_row(result);//获取一行数据  
            block[i].MD5=row[0];
            block[i].block_no=atoi(row[1]);
            block[i].ifusing=atoi(row[2]);
        }
        for(int i=0;i<num_rows;i++)
        {
            if(block[i].ifusing == NOUSING)//未被上传
            {
                //将ifusing位置1
                sprintf(temp,"UPDATE file_block SET ifusing=1 where MD5=\"%s\" AND block_no=%d;",MD5.data(),block[i].block_no);
                sql=temp;
                exeSQL(sql);

                //记录用户开始上传的时间
                long starttime=getCurrentTimeMsec();
                sprintf(temp,"UPDATE file_block SET usingtime=%ld where MD5=\"%s\" AND block_no=%d;",starttime,MD5.data(),block[i].block_no);
                sql=temp;
                exeSQL(sql);
            
                return block[i].block_no;
            }
            else//包正在被其他用户上传
            {
                //查看是否已经超时
                long nowtime=getCurrentTimeMsec();//当前时间
                long starttime;//开始上传时间
                long continuetime;//上一个用户已经上传时间
                //查询开始上传时间
                sprintf(temp,"SELECT usingtime FROM file_block where MD5=\"%s\" AND block_no=%d;",MD5.data(),block[i].block_no);
                sql=temp;
                exeSQL(sql);

                row = mysql_fetch_row(result);
                starttime=atol(row[0]);

                //计算该块已上传时间
                continuetime=nowtime-starttime;
                if(continuetime<ALARM)//尚未超时，继续传送
                    continue;
                else//已经超时，更新上传时间，并且可以作为要求用户上传的下一个块
                {
                    sprintf(temp,"UPDATE file_block SET usingtime=%ld where MD5=\"%s\" AND block_no=%d;",nowtime,MD5.data(),block[i].block_no);
                    sql=temp;
                    exeSQL(sql);

                    return block[i].block_no;
                }
                
            }
        }
        return WAITBLOCK;//虽然仍有块未传完，但是有其他进程负责
    }
}

/**********************************************************
func:   用户上传一个块成功，删除file_block中的块记录，客户端调用
input:  file_block block 结构体
            string MD5 文件的唯一标识，
            int block_no 块号
output: int 
        删除成功返回_SUCCESS
        删除失败返回_ERROR
***********************************************************/
int MyDB::upblock_success(file_block block)
{
    char temp[tempsize];
    string sql;
    sprintf(temp,"DELETE FROM file_block WHERE MD5=\"%s\" AND block_no=%d;",block.MD5.data(),block.block_no);
    sql=temp;
    int flag=exeSQL(sql);//删除file_block中相应表记录

    return flag;
}

/**********************************************************
func:   用户上传一个块失败，更新file_block中的块记录，将用户占用位清空，客户端调用
input:  file_block block 结构体
            string MD5 文件的唯一标识，
            int block_no 块号
output: int 
        设置成功返回_SUCCESS
        设置失败返回_ERROR
***********************************************************/
int MyDB::upblock_failed(file_block block)
{
    char temp[tempsize];
    string sql;
    sprintf(temp,"UPDATE file_block SET ifusing = %d WHERE MD5=\"%s\" AND block_no=%d;",NOUSING,block.MD5.data(),block.block_no);
    sql=temp;
    int flag=exeSQL(sql);//将file_block中相应块记录置为NOUSING

    return flag;
}

/**********************************************************
func:   返回当前文件已经传输的块数，给客户端
input:      string MD5 文件的唯一标识，
            int block_num 文件一共的块数
output: int 
        传输完成的块数
***********************************************************/
int MyDB::finished_block(string MD5,int block_num)
{
    char temp[tempsize];
    string sql;
    sprintf(temp,"SELECT block_no from file_block WHERE MD5 = \"%s\";",MD5.data());
    sql=temp;
    exeSQL(sql);
    int num_rows = mysql_num_rows(result);     //获取结果集中总共的行数

    int finish=block_num-num_rows;

    return finish;
}

