#include "mysql_select.h"

/**********************************************************
func:   ��ѯs�Ƿ��Ѿ���table�У�sΪ��ѡ��
input:  string table 
        string s
output: bool
        �Ѿ����ڷ���false�����򷵻�true
***********************************************************/
bool MyDB::if_insert(string table, string s)
{
    //��ѯuser�е�����
    string sql = "SELECT * FROM ";
    sql += table;

    //mysql_query()ִ�гɹ�����0,ִ��ʧ�ܷ��ط�0ֵ��
    if (mysql_query(mysql, sql.c_str()))
    {
        cout << "Query Error: " << mysql_error(mysql);
        return false;
    }
    else // ��ѯ�ɹ�
    {
        result = mysql_store_result(mysql); //��ȡ�����
        if (result)                         // �����˽����
        {
            int num_fields = mysql_num_fields(result); //��ȡ��������ܹ����ֶ�����������
            int num_rows = mysql_num_rows(result);     //��ȡ��������ܹ�������
            for (int i = 0; i < num_rows; i++)         //��ѯÿһ��
            {
                //��ȡ��һ������
                row = mysql_fetch_row(result);
                if (row == NULL)
                    break;
                //�½����û����ѽ���������false
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
    //����һȦ�û���δ����������true
    return true;
}

/**********************************************************
func:   �ж��û���½�����Ƿ�Ϸ�
input:  string user �û���
        string code
output: int 
        �Ϸ����� _SUCCESS
        ���벻ƥ�䷵�� _WRONGDATA
        �û��������ڷ��� _USERWRONG
***********************************************************/
int MyDB::if_signin(string user, string code)
{
    //��ѯuser�е�����
    string sql = "SELECT * FROM user;";

    //mysql_query()ִ�гɹ�����0,ִ��ʧ�ܷ��ط�0ֵ��
    if (mysql_query(mysql, sql.c_str()))
    {
        cout << "Query Error: " << mysql_error(mysql);
        return false;
    }
    else // ��ѯ�ɹ�
    {
        result = mysql_store_result(mysql); //��ȡ�����
        if (result)                         // �����˽����
        {
            int num_fields = mysql_num_fields(result); //��ȡ��������ܹ����ֶ�����������
            int num_rows = mysql_num_rows(result);     //��ȡ��������ܹ�������
            for (int i = 0; i < num_rows; i++)         //��ѯÿһ��
            {
                //��ȡ��һ������
                row = mysql_fetch_row(result);
                if (row == NULL)
                    break;
                if (row[1] == user) //�û�����
                {
                    char finalpwd[33];
                    MD5_code(code,finalpwd);
                    // if (row[2] == finalcode) //����ƥ��
                    if (strcmp(row[2],finalpwd)==0) //����ƥ��
                        return _SUCCESS;
                    else
                        return _WRONGDATA; //���벻ƥ��
                }
            }
        }
    }
    //�û�������
    return _USERWRONG;
}

//���������ת��
string changeCharToString(char *p)
{
    string str="";
    for(int i=0;i<16;i++)
        str+=p[i];
    str+="";
    return str;
}

/******************************************************
func:   ���������MD5����
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
 
	// //Md5���ܺ��32λ���
	// printf("����ǰ:%s\n���ܺ�16λ:", encrypt);
	// for (i = 4; i<12; i++)
	// {
	// 	printf("%02x", decrypt[i]);  
	// }

	//Md5���ܺ��32λ���
	//printf("\n����ǰ:%s\n���ܺ�32λ:", encrypt);
	for (i = 0; i<16; i++)
	{
		//printf("%02x", decrypt[i]);  
        sprintf(finalpwd+i*2,"%02x",decrypt[i]);
	}
    finalpwd[32]='\0';
    
    
	return;
}



/**********************************************************
func:   �ж������Ƿ�Ϸ�
        �涨���볤�ȱ�����8λ���ϣ�ͬʱ������ĸ�������Լ������ַ������ð����Ƿ��ַ�
input:  string code
output: bool 
        ����Ϸ����� true
        ���벻�Ϸ����� false
***********************************************************/
bool iflegal_code(string code)
{
    int length = code.length();
    int i;
    int balpha = 0;  //��д��ĸ
    int salpha = 0;  //Сд��ĸ
    int num = 0;     //����
    int special = 0; //�����ַ�
    int illegal = 0; //�Ƿ��ַ����ո�֮ǰ���ո��ascii��Ϊ32
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
    if (codecap < 3) //������ �ַ�������������
        return false;
    else
        return true;
}

/**********************************************************
func:   �ж��û�ע���Ƿ���Գɹ�
        �����ʽ�����Ȳ�С��12������ͬʱ������ĸ/����/�����ַ������Ҳ��ܺ��зǷ��ַ�(<=32)
input:  string user
        string code
output: int 
        �ɹ�����true��ʧ�ܷ��ش���ԭ��
        ʧ�ܵ�ԭ��_USEREXIST���û����Ѿ�����
                   _WRONGDATA���������ò����涨
***********************************************************/
int MyDB::if_signup(string user, string code)
{
    if (if_insert("user", user) == false) //���Ȳ�ѯ�û����Ƿ��Ѿ�����
        return _USERWRONG;
    if (iflegal_code(code) == false) //���Ų�ѯ�����Ƿ�Ϸ�
        return _WRONGDATA;

    return _SUCCESS;
}

/**********************************************************
func:   �ж��û�ע���Ƿ���Գɹ�
        �����ʽ�����Ȳ�С��8������ͬʱ������ĸ/����/�����ַ������Ҳ��ܺ��зǷ��ַ�(<=32)
        ��װif_signup�����ͻ�����
input:  string user
        string code
output: int 
        �ɹ�����true��ʧ�ܷ��ش���ԭ��
        ʧ�ܵ�ԭ��_USEREXIST���û����Ѿ�����
                   _WRONGDATA���������ò����涨
***********************************************************/
int MyDB::signup(string user, string code)
{
    char temp[tempsize];
    string sql;
    NowDate date = getTime();
    int flag = if_signup(user, code);
    if (flag == _SUCCESS)
    {
        //��user���в���һ���û���¼
        char finalpwd[33];
        MD5_code(code,finalpwd);
        sprintf(temp, "INSERT user(username,pwd,regedit_date) VALUES(\"%s\",\"%s\",\"%s\");", user.data(), finalpwd, date.tmp0);
        sql = temp;
        exeSQL(sql);
        //��file_format�н�������Ŀ¼�ṹ
        init_catalog(user);
    }

    return flag;
}