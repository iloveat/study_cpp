#include <iostream>

using namespace std;


/**将整型字符串还原成二进制字符串*/
/**输入字符串的格式为56,72,114,(最后一个逗号不能少)*/
string recoverFromString(const char *str_file)
{
    if(str_file == NULL)
    {
        cout<<"string2array(): input string can not be NULL"<<endl;
        return NULL;
    }

    string str_result = "";

    int k = 0;
    char str_int[20];
    unsigned long len = strlen(str_file);

    for(unsigned long i = 0; i < len; i++)
    {
        if(str_file[i] == ',')
        {
            str_int[k] = '\0';
            int tt = atoi(str_int);
            //cout<<(char)tt;
            str_result.append(1, (char)tt);
            k = 0;
            continue;
        }
        str_int[k] = str_file[i];
        k++;
    }

    return str_result;
}


