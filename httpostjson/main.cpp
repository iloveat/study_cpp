#include <iostream>
#include <string>
#include <string.h>
#include <curl/curl.h>
#include <exception>
#include "json/json.h"

using namespace std;


#define POST_TYPE_HTTP  (0)
#define POST_TYPE_HTTPS (1)


const int RECV_BUFFER_SIZE = 1024;
char recv_buff[RECV_BUFFER_SIZE];

size_t recvData( void *buffer, size_t sz, size_t nmemb, void *userp )
{
    int  wr_index = 0;
    int segsize = sz * nmemb;
    if ( wr_index + segsize > RECV_BUFFER_SIZE )
    {
        *(int *)userp = 1;
        return 0;
    }

    memcpy( (void *)&recv_buff[wr_index], buffer, (size_t)segsize );
    wr_index += segsize;
    recv_buff[wr_index] = 0;

    return segsize;
}


/*
int postJsonHttpRequest(const string &strUrl, const string &strJson, const int &type, const string &crtFile, int timeout=2)
{
    try
    {
        CURL *pCurl = NULL;
        curl_global_init(CURL_GLOBAL_ALL);
        pCurl = curl_easy_init();

        if (NULL != pCurl)
        {
            // 设置http发送的内容类型为JSON
            curl_slist *plist = curl_slist_append(NULL, "Content-Type:application/json;charset=UTF-8");
            // 设置超时时间
            curl_easy_setopt(pCurl, CURLOPT_TIMEOUT, timeout);
            curl_easy_setopt(pCurl, CURLOPT_HTTPHEADER, plist);
            curl_easy_setopt(pCurl, CURLOPT_URL, strUrl.c_str());
            // 设置要POST的JSON数据
            curl_easy_setopt(pCurl, CURLOPT_POSTFIELDS, strJson.c_str());
            // 设置接受数据的回调函数
            curl_easy_setopt(pCurl, CURLOPT_WRITEFUNCTION, recvData);
            // 如果是https请求则要添加证书
            if (type == POST_TYPE_HTTPS)
            {
                curl_easy_setopt(pCurl, CURLOPT_SSL_VERIFYHOST, 2);
                curl_easy_setopt(pCurl, CURLOPT_SSL_VERIFYPEER, true);
                curl_easy_setopt(pCurl, CURLOPT_CAINFO, crtFile.c_str());
            }
            // 发送请求
            CURLcode res = curl_easy_perform(pCurl);
            if (res != CURLE_OK)
            {
                printf("curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
                curl_easy_cleanup(pCurl);
                return -3;
            }
            // always cleanup
            curl_easy_cleanup(pCurl);
        }
        else
        {
            return -2;
        }

        curl_global_cleanup();
    }
    catch (std::exception &ex)
    {
        return -1;
    }
    return 1;
}

int getRetResult(const char *buff, int len)
{
    int result = 0;
    if(buff == NULL || len <= 0)
        return result;

    string pattern = "\"ret\"";
    string str_buff = string(buff);
    int len_patt = pattern.length();

    for(int i = 0; i < len-len_patt; i++)
    {
        if(pattern == str_buff.substr(i, len_patt))
        {
            string rest = str_buff.substr(i+len_patt);
            int colon_pos = rest.find_first_of(':');
            int comma_pos = rest.find_first_of(',');
            if(comma_pos==-1)
                comma_pos = rest.find_first_of('}');
            result = atoi(rest.substr(colon_pos+1, comma_pos-colon_pos-1).c_str());
            break;
        }
    }

    return result;
}

string getUseridResult(const char *buff, int len)
{
    string result = "";
    if(buff == NULL || len <= 0)
        return result;

    string pattern = "\"userid\"";
    string str_buff = string(buff);
    int len_patt = pattern.length();

    for(int i = 0; i < len-len_patt; i++)
    {
        if(pattern == str_buff.substr(i, len_patt))
        {
            string rest = str_buff.substr(i+len_patt);
            int colon_pos = rest.find_first_of(':');
            int comma_pos = rest.find_first_of(',');
            if(comma_pos==-1)
                comma_pos = rest.find_first_of('}');
            string tmp = rest.substr(colon_pos+1, comma_pos-colon_pos-1);
            int first_quote = tmp.find_first_of('\"');
            int last_quote = tmp.find_last_of('\"');
            result = tmp.substr(first_quote, last_quote);
            break;
        }
    }

    return result;
}

void apikey_http_post()
{
    const string strUrl = "http://smartdevice.ai.tuling123.com/projectapi/multibiz";
    const string strJson = "{\"key\":\"1f0a0c1aab1b49c2974ee16ac240769e\",\"sdk_type\":\"visionSDK\",\"service_identify\":\"validityVerification\"}";
    int res = postJsonHttpRequest(strUrl, strJson, POST_TYPE_HTTP, "");
    if(res < 0)
    {
        cout<<"connect network failed, error code: "<<res<<endl;
    }
    else
    {
        cout<<recv_buff<<endl;
        cout<<getRetResult(recv_buff, strlen(recv_buff))<<endl;
    }
}

void apikey_https_post()
{
    const string strUrl = "https://smartdevice.ai.tuling123.com/projectapi/multibiz";
    const string strJson = "{\"key\":\"1f0a0c1aab1b49c2974ee16ac240769e\",\"sdk_type\":\"visionSDK\",\"service_identify\":\"validityVerification\"}";
    int res = postJsonHttpRequest(strUrl, strJson, POST_TYPE_HTTPS, "mykey.pem");
    if(res < 0)
    {
        cout<<"connect network failed, error code: "<<res<<endl;
    }
    else
    {
        cout<<recv_buff<<endl;
        cout<<getRetResult(recv_buff, strlen(recv_buff))<<endl;
    }
}

void userid_https_post()
{
    string strUrl = "https://www.tuling123.com/openapi/getuserid.do";
    //string strJson = "{\"key\":\"1f0a0c1aab1b49c2974ee16ac240769e\",\"uniqueId\":\"00000000-54b3-e7c7-0000-000046bffd97\",\"os_version\": \"1.5\"}";
    string strJson = "{\"timestamp\": \"8bbe5ba2-5700-4611-8985-28b3d252cd83\",\"key\": \"1f0a0c1aab1b49c2974ee16ac240769e\", \"data\": \"bCQ3KmuvMbVj\\/L6lM3y+cIn38qmLbs+Md2Wn2EA0y7qThITv5Rqc2DG5KMAn2KnoE0z2l0q1VPndoF3Q8pACu1JByZJX5bk4JSFxC\\/B4PT5YpTvrXFDt0i1mpSp9hw0r\"}";
    int res = postJsonHttpRequest(strUrl, strJson, POST_TYPE_HTTPS, ".myusr.crt");
    if(res < 0)
    {
        cout<<"connect network failed, error code: "<<res<<endl;
    }
    else
    {
        cout<<recv_buff<<endl;
        cout<<getRetResult(recv_buff, strlen(recv_buff))<<endl;
    }
}

void userid_https_post()
{
    string strUrl = "https://www.tuling123.com/openapi/getuserid.do";
    //string strJson = "{\"key\":\"1f0a0c1aab1b49c2974ee16ac240769e\",\"uniqueId\":\"00000000-54b3-e7c7-0000-000046bffd97\",\"os_version\": \"1.5\"}";
    string strJson = "{\"timestamp\": \"8bbe5ba2-5700-4611-8985-28b3d252cd83\",\"key\": \"1f0a0c1aab1b49c2974ee16ac240769e\", \"data\": \"bCQ3KmuvMbVj\\/L6lM3y+cIn38qmLbs+Md2Wn2EA0y7qThITv5Rqc2DG5KMAn2KnoE0z2l0q1VPndoF3Q8pACu1JByZJX5bk4JSFxC\\/B4PT5YpTvrXFDt0i1mpSp9hw0r\"}";
    int res = postJsonHttpRequest(strUrl, strJson, POST_TYPE_HTTPS, ".myusr.crt");
    if(res < 0)
    {
        cout<<"connect network failed, error code: "<<res<<endl;
    }
    else
    {
        cout<<recv_buff<<endl;
        cout<<getRetResult(recv_buff, strlen(recv_buff))<<endl;
    }
}

void apikey_https_post()
{
    const string strUrl = "https://smartdevice.ai.tuling123.com/projectapi/multibiz";
    const string strJson = "{\"key\":\"1f0a0c1aab1b49c2974ee16ac240769e\",\"sdk_type\":\"visionSDK\",\"service_identify\":\"validityVerification\"}";
    int res = postJsonHttpRequest(strUrl, strJson, POST_TYPE_HTTPS, "mykey.pem");
    if(res < 0)
    {
        cout<<"connect network failed, error code: "<<res<<endl;
    }
    else
    {
        cout<<recv_buff<<endl;
        cout<<getRetResult(recv_buff, strlen(recv_buff))<<endl;
    }
}
*/


// main function 2 post http or https request with json format data
int postJsonHttpRequest(const string &strUrl, const string &strJson, const int &type, const string &crtFile, int timeout=10)
{
    try
    {
        CURL *pCurl = NULL;
        curl_global_init(CURL_GLOBAL_ALL);
        pCurl = curl_easy_init();

        if (NULL != pCurl)
        {
            // 设置http发送的内容类型为JSON
            curl_slist *plist = curl_slist_append(NULL, "Content-Type:application/json;charset=UTF-8");
            // 设置超时时间
            curl_easy_setopt(pCurl, CURLOPT_TIMEOUT, timeout);
            curl_easy_setopt(pCurl, CURLOPT_HTTPHEADER, plist);
            curl_easy_setopt(pCurl, CURLOPT_URL, strUrl.c_str());
            // 设置要POST的JSON数据
            curl_easy_setopt(pCurl, CURLOPT_POSTFIELDS, strJson.c_str());
            // 设置接受数据的回调函数
            curl_easy_setopt(pCurl, CURLOPT_WRITEFUNCTION, recvData);
            // 如果是https请求则要添加证书
            if (type == POST_TYPE_HTTPS)
            {
                curl_easy_setopt(pCurl, CURLOPT_SSL_VERIFYHOST, 2);
                curl_easy_setopt(pCurl, CURLOPT_SSL_VERIFYPEER, true);
                //curl_easy_setopt(pCurl, CURLOPT_SSLVERSION, CURL_SSLVERSION_TLSv1_2);
                curl_easy_setopt(pCurl, CURLOPT_CAINFO, crtFile.c_str());
            }
            // 发送请求
            CURLcode res = curl_easy_perform(pCurl);
            if (res != CURLE_OK)
            {
                printf("curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
                curl_easy_cleanup(pCurl);
                return -3;
            }
            // always cleanup
            curl_easy_cleanup(pCurl);
        }
        else
        {
            return -2;
        }

        curl_global_cleanup();
    }
    catch (std::exception &ex)
    {
        return -1;
    }
    return 1;
}

// self parse string, format: {"ret":0,"time":"2017-04-21 17:36:31"}
// get [ret] value
int getRetResult(const char *buff, int len)
{
    int result = 0;
    if(buff == NULL || len <= 0)
        return result;

    string pattern = "\"ret\"";
    string str_buff = string(buff);
    int len_patt = pattern.length();

    for(int i = 0; i < len-len_patt; i++)
    {
        if(pattern == str_buff.substr(i, len_patt))
        {
            string rest = str_buff.substr(i+len_patt);
            int colon_pos = rest.find_first_of(':');
            int comma_pos = rest.find_first_of(',');
            if(comma_pos==-1)
                comma_pos = rest.find_first_of('}');
            result = atoi(rest.substr(colon_pos+1, comma_pos-colon_pos-1).c_str());
            break;
        }
    }

    return result;
}

// self parse string, format: {"ret":0,"userid":"sfasdccxvxcvcx"}
// get [userid] value
string getUseridResult(const char *buff, int len)
{
    string result = "";
    if(buff == NULL || len <= 0)
        return result;

    string pattern = "\"userid\"";
    string str_buff = string(buff);
    int len_patt = pattern.length();

    for(int i = 0; i < len-len_patt; i++)
    {
        if(pattern == str_buff.substr(i, len_patt))
        {
            string rest = str_buff.substr(i+len_patt);
            int colon_pos = rest.find_first_of(':');
            int comma_pos = rest.find_first_of(',');
            if(comma_pos==-1)
                comma_pos = rest.find_first_of('}');
            string tmp = rest.substr(colon_pos+1, comma_pos-colon_pos-1);
            int first_quote = tmp.find_first_of('\"');
            int last_quote = tmp.find_last_of('\"');
            result = tmp.substr(first_quote, last_quote-first_quote+1);
            break;
        }
    }

    return result;
}


int obtainUserid_https(const char *data)
{
    memset(recv_buff, 0, sizeof(char)*RECV_BUFFER_SIZE);

    const string strUrl = "https://www.tuling123.com/openapi/getuserid.do";
    string strJson = string(data);

    int result = postJsonHttpRequest(strUrl, strJson, POST_TYPE_HTTPS, "myusr.crt");

    return result;
}


int verifyApikey_https(const char *apikey, const char *userid)
{
    memset(recv_buff, 0, sizeof(char)*RECV_BUFFER_SIZE);

	const string strUrl = "https://smartdevice.ai.tuling123.com/projectapi/multibiz";
	string strJson = "{\"key\":\""+string(apikey)+"\",\"userid\":"+string(userid)+",\"sdk_type\":\"VISUAL_VERSION\",\"service_identify\":\"validityVerification\"}";
	int result = postJsonHttpRequest(strUrl, strJson, POST_TYPE_HTTPS, "mykey.pem");

	return result;
}

/*
-1:network error
-2:get userid failed
 0:invalid apikey
 1:succeed
*/
int checkValidation(const char *apikey, const char *data)
{
	// get userid
	int res = obtainUserid_https(data);
    if(res < 0)
    {
        return -1;  // network error
    }
    else
    {
        cout<<recv_buff<<endl;
        if(getRetResult(recv_buff, strlen(recv_buff)) != 0)
        {
            return -2;  // get userid failed
        }
    }

    // extract userid
	string userid = getUseridResult(recv_buff, strlen(recv_buff));  // with quote, like "84277487"

	// verify apikey
	res = verifyApikey_https(apikey, userid.c_str());
    if(res < 0)
    {
        return -1;  // network error
    }
    else
    {
        cout<<recv_buff<<endl;
        if(getRetResult(recv_buff, strlen(recv_buff)) != 0)
        {
            return 0;  // invalid apikey
        }
    }

    return 1;
}




int main(int argc, char *argv[])
{
    string srtJson = "{\"timestamp\":\"90148806-3e4c-41ab-96b2-e7979d8d0153\",\"key\":\"73de5945c2cf18e3f7126c7f88ba7fea\",\"data\":\"d7AnqaXtCzrdzo7xvL3ano2sL1oyYQXI9Cy74MWRoO4DFRLnMYOcIYg1WcwhmJINp97BdCGIZ3e3ZUWZemeZOwpvsCU6GKfxVOVr9FhomH\\/FXpInuFjisFCcEVzE+mPh\"}";
    checkValidation("73de5945c2cf18e3f7126c7f88ba7fea", srtJson.c_str());

    return 0;
}
