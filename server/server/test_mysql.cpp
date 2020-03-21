#include "mysql_select.h"
using namespace std;

//构造函数
MyDB::MyDB()
{
    mysql = mysql_init(NULL); //初始化数据库连接变量
    if (mysql == NULL)
    {
        cout << "Error:" << mysql_error(mysql);
        exit(1);
    }
}

//析构函数，断开与mysql的连接
MyDB::~MyDB()
{
    if (mysql != NULL) //关闭数据连接
    {
        mysql_close(mysql);
    }
}

//关闭与MySQL的链接
void MyDB::closeSQL()
{
    mysql_close(mysql);

    return;
}

//初始化MyDB
bool MyDB::initDB(string host, string user, string passwd, string db_name)
{
    // 函数mysql_real_connect建立一个数据库连接
    // 成功返回MYSQL*连接句柄，失败返回NULL
    mysql = mysql_real_connect(mysql, host.c_str(), user.c_str(), passwd.c_str(), db_name.c_str(), 0, NULL, 0);
    if (mysql == NULL)
    {
        cout << "Error: " << mysql_error(mysql);
        exit(1);
    }
    return true;
}

//输出select的结果
bool MyDB::outselect()
{

    int num_fields = mysql_num_fields(result); //获取结果集中总共的字段数，即列数
    int num_rows = mysql_num_rows(result);     //获取结果集中总共的行数
    //cout<<num_fields<<" "<<num_rows<<endl;
    for (int i = 0; i < num_rows; i++)         //输出每一行
    {
        //获取下一行数据
        row = mysql_fetch_row(result);
        if (row == NULL)
            break;

        for (int j = 0; j < num_fields; j++) //输出每一字段
        {
            if (row[j] == NULL)
            {
                cout <<setiosflags(ios::left)<< setw(6) << "NULL";
            }
            else
            {
                int length=(strlen(row[j])<6?6:12);
                cout << setiosflags(ios::left)<<setw(length) << row[j];
            }
        }
        cout << endl;
    }
    cout << endl;

    return true;
}

//执行MySQL语句
bool MyDB::exeSQL(string sql)
{
    //mysql_query()执行成功返回0,执行失败返回非0值。
    if (mysql_query(mysql, sql.c_str()))
    {
        cout << "Query Error: " << mysql_error(mysql);
        return false;
    }
    else // 查询成功select
    {
        result = mysql_store_result(mysql); //获取结果集
        if (result)                         // 返回了结果集
        {
            return true;
        }
        else // result==NULL
        {
            if (mysql_field_count(mysql) == 0) //代表执行的是update,insert,delete类的非查询语句
            {
                // (it was not a SELECT)
                int num_rows = mysql_affected_rows(mysql); //返回update,insert,delete影响的行数
            }
            else // error
            {
                cout << "Get result error: " << mysql_error(mysql);
                return false;
            }
        }
    }

    return true;
}

NowDate getTime()
{
    time_t timep;
    time(&timep);
    NowDate date;

    strftime(date.tmp0, sizeof(date.tmp0), "%Y-%m-%d", localtime(&timep));
    strftime(date.tmp1, sizeof(date.tmp1), "%H:%M:%S", localtime(&timep));

    return date;
}

//获得毫秒数量级
long getCurrentTimeMsec()
{
    int i = 0;
    long msec = 0;
    char str[20] = {0};
    struct timeval stuCurrentTime;

    gettimeofday(&stuCurrentTime, NULL);
    sprintf(str, "%ld%03ld", stuCurrentTime.tv_sec, (stuCurrentTime.tv_usec) / 1000);

    for (i = 0; i < strlen(str); i++)
    {
        msec = msec * 10 + (str[i] - '0');
    }
    return msec;
}

/*
int main()
{
    MyDB db;

    NowDate date = getTime();
    int r;
    string user = "fengh";
    string code = "111aaaa!";
    char temp[1024];
    string inst;
    //连接数据库
    db.initDB("localhost", "root", "", "test");
    //将用户信息添加到数据库
    //  db.exeSQL("INSERT user(username,pwd,regedit_date) values('fenghui','123123','2019-12-07');");
    //  db.exeSQL("delete from user where userid = 2;");

    //将所有用户信息读出，并输出。
    db.exeSQL("SELECT * from user;");
    db.outselect();
    //测试新建用户
    r = db.if_signup(user, code);
    if (r == -1)
    {
        cout << "该用户已经存在！" << endl;
    }
    else if (r == -2)
    {
        cout << "密码设置不合法，" << endl;
        cout << "密码格式：长度不小于8，必须同时含有字母/数字/特殊字符，并且不能含有空格" << endl;
    }
    else
    {
        cout << "用户建立成功" << endl;
        cout << "现有用户有：" << endl;
        sprintf(temp, "INSERT user(username,pwd,regedit_date) values(\"%s\",\"%s\",\"%s\");", user.data(), code.data(), date.tmp0);
        inst = temp;
        db.exeSQL(inst);
        db.exeSQL("SELECT * from user;");
    }
    //测试用户登陆
    r = db.if_signin("fenghui", "123123");
    if (r == false)
        cout << "用户名或密码输入错误" << endl;
    else
        cout << "用户登录成功" << endl;
    //测试删除用户
    //r=db.exeSQL("delete from user where username = \'fenghui\';");
    //测试新建文件夹
    //db.init_catalog("fenghui");

    //测试查询为空时的反应
    //经过查询发现result仍然有返回，返回列数，行数为0罢了
    // db.exeSQL("SELECT * from user where username=\"hyf\";");
    // db.outselect();
    
    //测试查询文件是否存在
    db.ifexist_store("11111",1024);
    db.exeSQL("SELECT * from file_store;");
    db.outselect();


    return 0;
}
*/