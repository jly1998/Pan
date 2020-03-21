#include "mysql_select.h"

/**********************************************************
func:   查询s是否已经在table中，s为候选码
input:  string table 
        string s
output: bool
        已经存在返回false，否则返回true
***********************************************************/
bool MyDB::if_insert(string table, string s)
{
    //查询user中的数据
    string sql = "SELECT * FROM ";
    sql += table;

    //mysql_query()执行成功返回0,执行失败返回非0值。
    if (mysql_query(mysql, sql.c_str()))
    {
        cout << "Query Error: " << mysql_error(mysql);
        return false;
    }
    else // 查询成功
    {
        result = mysql_store_result(mysql); //获取结果集
        if (result)                         // 返回了结果集
        {
            int num_fields = mysql_num_fields(result); //获取结果集中总共的字段数，即列数
            int num_rows = mysql_num_rows(result);     //获取结果集中总共的行数
            for (int i = 0; i < num_rows; i++)         //查询每一行
            {
                //获取下一行数据
                row = mysql_fetch_row(result);
                if (row == NULL)
                    break;
                //新建的用户名已建立，返回false
                if (table == "user")
                {
                    if (row[1] == s)
                    {
                        return false;
                    }
                }
            }
        }
    }
    //找了一圈用户尚未建立，返回true
    return true;
}

/**********************************************************
func:   判断用户登陆操作是否合法
input:  string user 用户名
        string code
output: int 
        合法返回 _SUCCESS
        密码不匹配返回 _WRONGDATA
        用户名不存在返回 _USERWRONG
***********************************************************/
int MyDB::if_signin(string user, string code)
{
    //查询user中的数据
    string sql = "SELECT * FROM user;";

    //mysql_query()执行成功返回0,执行失败返回非0值。
    if (mysql_query(mysql, sql.c_str()))
    {
        cout << "Query Error: " << mysql_error(mysql);
        return false;
    }
    else // 查询成功
    {
        result = mysql_store_result(mysql); //获取结果集
        if (result)                         // 返回了结果集
        {
            int num_fields = mysql_num_fields(result); //获取结果集中总共的字段数，即列数
            int num_rows = mysql_num_rows(result);     //获取结果集中总共的行数
            for (int i = 0; i < num_rows; i++)         //查询每一行
            {
                //获取下一行数据
                row = mysql_fetch_row(result);
                if (row == NULL)
                    break;
                if (row[1] == user) //用户存在
                {
                    char finalpwd[33];
                    MD5_code(code,finalpwd);
                    // if (row[2] == finalcode) //密码匹配
                    if (strcmp(row[2],finalpwd)==0) //密码匹配
                        return _SUCCESS;
                    else
                        return _WRONGDATA; //密码不匹配
                }
            }
        }
    }
    //用户不存在
    return _USERWRONG;
}

//令人无语的转换
string changeCharToString(char *p)
{
    string str="";
    for(int i=0;i<16;i++)
        str+=p[i];
    str+="";
    return str;
}

/******************************************************
func:   将密码进行MD5加密
input:  string pwd
output: string finalpwd
*******************************************************/
void MD5_code(string pwd,char*finalpwd)
{
    int i;
	unsigned char encrypt[tempsize];
    //21232f297a57a5a743894a0e4a801fc3  
	unsigned char decrypt[16];

    memcpy((char*)encrypt,pwd.data(),pwd.length());
 
	MD5_CTX md5;
 
	MD5Init(&md5);
	MD5Update(&md5, encrypt, strlen((char *)encrypt));
	MD5Final(&md5, decrypt);

    //finalpwd=temp;
 
	// //Md5加密后的32位结果
	// printf("加密前:%s\n加密后16位:", encrypt);
	// for (i = 4; i<12; i++)
	// {
	// 	printf("%02x", decrypt[i]);  
	// }

	//Md5加密后的32位结果
	//printf("\n加密前:%s\n加密后32位:", encrypt);
	for (i = 0; i<16; i++)
	{
		//printf("%02x", decrypt[i]);  
        sprintf(finalpwd+i*2,"%02x",decrypt[i]);
	}
    finalpwd[32]='\0';
    
    
	return;
}



/**********************************************************
func:   判断密码是否合法
        规定密码长度必须在8位以上，同时包括字母，数字以及特殊字符，不得包含非法字符
input:  string code
output: bool 
        密码合法返回 true
        密码不合法返回 false
***********************************************************/
bool iflegal_code(string code)
{
    int length = code.length();
    int i;
    int balpha = 0;  //大写字母
    int salpha = 0;  //小写字母
    int num = 0;     //数字
    int special = 0; //特殊字符
    int illegal = 0; //非法字符，空格之前，空格的ascii码为32
    int codecap = 0;

    if (length < codelength)
    {
        return false;
    }
    for (i = 0; i < length; i++)
    {
        if (code[i] >= 'A' && code[i] <= 'Z')
            balpha = 1;
        else if (code[i] >= 'a' && code[i] <= 'z')
            salpha = 1;
        else if (code[i] >= '0' && code[i] <= '9')
            num = 1;
        else
            special = 1;
    }
    codecap = balpha + salpha + num + special;
    if (codecap < 3) //密码中 字符类型少于三种
        return false;
    else
        return true;
}

/**********************************************************
func:   判断用户注册是否可以成功
        密码格式：长度不小于12，必须同时含有字母/数字/特殊字符，并且不能含有非法字符(<=32)
input:  string user
        string code
output: int 
        成功返回true，失败返回错误原因
        失败的原因：_USEREXIST：用户名已经存在
                   _WRONGDATA：密码设置不符规定
***********************************************************/
int MyDB::if_signup(string user, string code)
{
    if (if_insert("user", user) == false) //首先查询用户名是否已经存在
        return _USERWRONG;
    if (iflegal_code(code) == false) //接着查询密码是否合法
        return _WRONGDATA;

    return _SUCCESS;
}

/**********************************************************
func:   判断用户注册是否可以成功
        密码格式：长度不小于8，必须同时含有字母/数字/特殊字符，并且不能含有非法字符(<=32)
        封装if_signup，给客户端用
input:  string user
        string code
output: int 
        成功返回true，失败返回错误原因
        失败的原因：_USEREXIST：用户名已经存在
                   _WRONGDATA：密码设置不符规定
***********************************************************/
int MyDB::signup(string user, string code)
{
    char temp[tempsize];
    string sql;
    NowDate date = getTime();
    int flag = if_signup(user, code);
    if (flag == _SUCCESS)
    {
        //在user表中插入一条用户记录
        char finalpwd[33];
        MD5_code(code,finalpwd);
        sprintf(temp, "INSERT user(username,pwd,regedit_date) VALUES(\"%s\",\"%s\",\"%s\");", user.data(), finalpwd, date.tmp0);
        sql = temp;
        exeSQL(sql);
        //在file_format中建立两个目录结构
        init_catalog(user);
    }

    return flag;
}