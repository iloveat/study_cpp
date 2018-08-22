#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "parse_string.h"  //字符串解析函数
#include "page.jpg.h"

using namespace std;


#define MAXLEN 1


const char HexCHAR[] = {'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};


/**将字符串str_file所指的内容转存为二进制文件*/
/**字符串str_file的格式为F0AG85T9...,每两个字符按规则组成一个字节*/
bool string2binaryfile(const char *str_file, const char *out_file_name="out.bin")
{
    if(str_file == NULL)
    {
        cout<<"string2binaryfile(): input string can not be NULL"<<endl;
        return false;
    }

    if(out_file_name == NULL)
        out_file_name = "out.bin";

    ofstream outbin(out_file_name, ios::binary);
    if(!outbin)
    {
        cout<<"string2binaryfile(): create output file failed"<<endl;
        outbin.close();
        return false;
    }

    unsigned long len = strlen(str_file);
    assert(len % 2 == 0);

    for(unsigned long i = 0; i < len; i += 2)
    {
        int low = HexChar2Int(str_file[i]);
        int high = HexChar2Int(str_file[i+1]);
        char ret = ((high & 0x0f) << 4) | (low & 0x0f);
        outbin << ret;
    }

    outbin.close();
    return true;
}


/**将二进制文件逐字节转换成整数并导出到C++头文件中*/
bool binaryfile2header(const char *binary_file_name, const char *header_file_name="out.h")
{
    if(binary_file_name == NULL)
    {
        cout<<"binaryfile2header(): file name can not be NULL"<<endl;
        return false;
    }
    if(header_file_name == NULL)
        header_file_name = "out.h";

    ofstream outheader(header_file_name);
    if(!outheader)
    {
        cout<<"binaryfile2header(): create output file failed"<<endl;
        outheader.close();
        return false;
    }
    outheader<<"const char *str_bin = ";
    outheader<<"\"";

    char buf[MAXLEN];
    FILE *inbin = fopen(binary_file_name, "rb");
    if(inbin == NULL)
    {
        cout<<"open binary fail failed"<<endl;
        return false;
    }
    int rc;
    while((rc = fread(buf, sizeof(char), MAXLEN, inbin)) != 0)
    {
        int idxl = (buf[0]) & 0x0f;
        int idxh = (buf[0] >> 4) & 0x0f;
        cout << HexCHAR[idxl] << HexCHAR[idxh];
        outheader << HexCHAR[idxl] << HexCHAR[idxh];
    }
    fclose(inbin);

    outheader<<"\";"<<endl;
    outheader.close();

    return true;
}



int main(int argc, char **argv)
{

    if(argc < 2)
    {
        cout<<"at least one parameter [binary file name] needed"<<endl;
        return -1;
    }

    for(int i = 1; i < argc; i++)
    {
        char headerpath[500];
        memset(headerpath, '\0', 500*sizeof(char));
        sprintf(headerpath, "%s.h", argv[i]);
        binaryfile2header(argv[i], headerpath);
    }

    //恢复被编码的图像文件
    //string2binaryfile(str_bin, "page.jpg");

    /**恢复被编码的图像文件，测试*/
    /*
    string res = recoverFromString(str_bin);
    ofstream otf("recovery.jpg");
    otf<<res;
    otf.close();
    */

    return 0;
}





