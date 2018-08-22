#include <iostream>

using namespace std;


/**将单个字符转成数字*/
int HexChar2Int(const char &c)
{
    if(c >= '0' && c <= '9') return (c-'0');
    if(c >= 'A' && c <= 'F') return (c-'A'+10);
    return -1;
}

/**将整型字符串还原成二进制字符串*/
/**输入字符串的格式为F0AG85T9...,每两个字符按规则组成一个字节*/
string recoverFromString(const char *str_file)
{
    if(str_file == NULL)
    {
        cout<<"string2array(): input string can not be NULL"<<endl;
        return NULL;
    }

    string str_result = "";

    unsigned long len = strlen(str_file);
    assert(len % 2 == 0);

    for(unsigned long i = 0; i < len; i += 2)
    {
        int low = HexChar2Int(str_file[i]);
        int high = HexChar2Int(str_file[i+1]);
        char ret = ((high & 0x0f) << 4) | (low & 0x0f);
        str_result.append(1, ret);
    }

    return str_result;
}


