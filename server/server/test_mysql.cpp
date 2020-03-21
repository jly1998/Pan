#include "mysql_select.h"
using namespace std;

//���캯��
MyDB::MyDB()
{
    mysql = mysql_init(NULL); //��ʼ�����ݿ����ӱ���
    if (mysql == NULL)
    {
        cout << "Error:" << mysql_error(mysql);
        exit(1);
    }
}

//�����������Ͽ���mysql������
MyDB::~MyDB()
{
    if (mysql != NULL) //�ر���������
    {
        mysql_close(mysql);
    }
}

//�ر���MySQL������
void MyDB::closeSQL()
{
    mysql_close(mysql);

    return;
}

//��ʼ��MyDB
bool MyDB::initDB(string host, string user, string passwd, string db_name)
{
    // ����mysql_real_connect����һ�����ݿ�����
    // �ɹ�����MYSQL*���Ӿ����ʧ�ܷ���NULL
    mysql = mysql_real_connect(mysql, host.c_str(), user.c_str(), passwd.c_str(), db_name.c_str(), 0, NULL, 0);
    if (mysql == NULL)
    {
        cout << "Error: " << mysql_error(mysql);
        exit(1);
    }
    return true;
}

//���select�Ľ��
bool MyDB::outselect()
{

    int num_fields = mysql_num_fields(result); //��ȡ��������ܹ����ֶ�����������
    int num_rows = mysql_num_rows(result);     //��ȡ��������ܹ�������
    //cout<<num_fields<<" "<<num_rows<<endl;
    for (int i = 0; i < num_rows; i++)         //���ÿһ��
    {
        //��ȡ��һ������
        row = mysql_fetch_row(result);
        if (row == NULL)
            break;

        for (int j = 0; j < num_fields; j++) //���ÿһ�ֶ�
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

//ִ��MySQL���
bool MyDB::exeSQL(string sql)
{
    //mysql_query()ִ�гɹ�����0,ִ��ʧ�ܷ��ط�0ֵ��
    if (mysql_query(mysql, sql.c_str()))
    {
        cout << "Query Error: " << mysql_error(mysql);
        return false;
    }
    else // ��ѯ�ɹ�select
    {
        result = mysql_store_result(mysql); //��ȡ�����
        if (result)                         // �����˽����
        {
            return true;
        }
        else // result==NULL
        {
            if (mysql_field_count(mysql) == 0) //����ִ�е���update,insert,delete��ķǲ�ѯ���
            {
                // (it was not a SELECT)
                int num_rows = mysql_affected_rows(mysql); //����update,insert,deleteӰ�������
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

//��ú���������
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
    //�������ݿ�
    db.initDB("localhost", "root", "", "test");
    //���û���Ϣ��ӵ����ݿ�
    //  db.exeSQL("INSERT user(username,pwd,regedit_date) values('fenghui','123123','2019-12-07');");
    //  db.exeSQL("delete from user where userid = 2;");

    //�������û���Ϣ�������������
    db.exeSQL("SELECT * from user;");
    db.outselect();
    //�����½��û�
    r = db.if_signup(user, code);
    if (r == -1)
    {
        cout << "���û��Ѿ����ڣ�" << endl;
    }
    else if (r == -2)
    {
        cout << "�������ò��Ϸ���" << endl;
        cout << "�����ʽ�����Ȳ�С��8������ͬʱ������ĸ/����/�����ַ������Ҳ��ܺ��пո�" << endl;
    }
    else
    {
        cout << "�û������ɹ�" << endl;
        cout << "�����û��У�" << endl;
        sprintf(temp, "INSERT user(username,pwd,regedit_date) values(\"%s\",\"%s\",\"%s\");", user.data(), code.data(), date.tmp0);
        inst = temp;
        db.exeSQL(inst);
        db.exeSQL("SELECT * from user;");
    }
    //�����û���½
    r = db.if_signin("fenghui", "123123");
    if (r == false)
        cout << "�û����������������" << endl;
    else
        cout << "�û���¼�ɹ�" << endl;
    //����ɾ���û�
    //r=db.exeSQL("delete from user where username = \'fenghui\';");
    //�����½��ļ���
    //db.init_catalog("fenghui");

    //���Բ�ѯΪ��ʱ�ķ�Ӧ
    //������ѯ����result��Ȼ�з��أ���������������Ϊ0����
    // db.exeSQL("SELECT * from user where username=\"hyf\";");
    // db.outselect();
    
    //���Բ�ѯ�ļ��Ƿ����
    db.ifexist_store("11111",1024);
    db.exeSQL("SELECT * from file_store;");
    db.outselect();


    return 0;
}
*/