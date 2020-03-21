/**********
处理用户删除文件的数据，若返回值不是_SUCCESS则fileid的值不可信
data_begin      请求类型后的数据起始地址
fileid          文件id int型
**********/
int Getfileid(char *data_begin, int &fileid)
{
    char fileid_temp[] = "\"fileid\":";
    char temp[8];
    char *leftp,*rightp;
    //用户文件id
    leftp = strstr(data_begin, fileid_temp);
    if (leftp == NULL)
        return _ERROR;
    leftp += strlen(fileid_temp);
    rightp = strstr(leftp, ")");
    if (rightp == NULL)
        return _ERROR;

    *rightp = '\0';
    memcpy(temp, leftp, rightp - leftp + 1);
    *rightp = ')';

    fileid=atoi(temp);

    return _SUCCESS;
}

/**********
处理用户移动/复制文件/夹的数据，若返回值不是_SUCCESS则from_id，to_id的值不可信
data_begin      请求类型后的数据起始地址
from_id         待移动/删除的文件id
to_id           将移动/删除至的文件夹id
**********/
int Getfromtoid(char *data_begin, int &from_id,int &to_id)
{
    char temp[8];
    char *leftp,*rightp;

    //起始位置 文件id
    char fromid_temp[] = "\"from_id\":";
    leftp = strstr(data_begin, fromid_temp);
    if (leftp == NULL)
        return _ERROR;
    leftp += strlen(fromid_temp);
    rightp = strstr(leftp, ",");
    if (rightp == NULL)
        return _ERROR;

    *rightp = '\0';
    memcpy(temp, leftp, rightp - leftp + 1);
    *rightp = ',';

    from_id=atoi(temp);

    //目标位置 文件夹id
    char toid_temp[] = "\"to_id\":";
    leftp = strstr(rightp, toid_temp);
    if (leftp == NULL)
        return _ERROR;
    leftp += strlen(toid_temp);
    rightp = strstr(leftp, ")");
    if (rightp == NULL)
        return _ERROR;

    *rightp = '\0';
    memcpy(temp, leftp, rightp - leftp + 1);
    *rightp = ')';

    to_id=atoi(temp);

    return _SUCCESS;
}