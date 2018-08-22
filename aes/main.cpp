#include <iostream>
#include <assert.h>
#include <string.h>
#include <openssl/aes.h>
#include <openssl/md5.h>
#include <openssl/hmac.h>
#include <openssl/buffer.h>

using namespace std;


// 将单个字符转成数字
int HexChar2Int(const char &c)
{
    assert((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f'));
    if(c >= '0' && c <= '9') return (c-'0');
    if(c >= 'a' && c <= 'f') return (c-'a'+10);
    return -1;
}

// 将16进制字符串转成字节数组
// 返回结果需要手动释放
unsigned char* HexString2UcharArray(const char *str_hex)
{
    assert(str_hex != NULL);

    int len = strlen(str_hex);
    assert((len % 2 == 0) && (len != 0));

    unsigned char *result = (unsigned char*)malloc(len/2);
    assert(result != NULL);

    for(int i = 0, j = 0; i < len; i += 2, j++)
    {
        int high = HexChar2Int(str_hex[i]);
        int low = HexChar2Int(str_hex[i+1]);
        unsigned char ret = ((high & 0x0f) << 4) | (low & 0x0f);
        result[j] = ret;
    }

    return result;
}

// 计算字符串的md5值，英文字母用小写
// 返回结果需要手动释放
char* generate_md5(const char* str_input, int len)
{
    assert(str_input != NULL);

    unsigned char *md5_ret = (unsigned char*)malloc(MD5_DIGEST_LENGTH);
    assert(md5_ret != NULL);

    MD5((const unsigned char*)str_input, len, md5_ret);

    char *result = (char*)malloc(MD5_DIGEST_LENGTH*2+1);
    assert(result != NULL);

    for(int i = 0; i < MD5_DIGEST_LENGTH; i++)
    {
        sprintf(result+2*i, "%02x", md5_ret[i]);
    }
    result[MD5_DIGEST_LENGTH*2] = '\0';
    free(md5_ret);

    return result;
}

// 对字节数组进行base64编码
// 返回结果需要手动释放
char* encode_base64(const unsigned char *str_input, int len)
{
    BIO *bmem = BIO_new(BIO_s_mem());
    assert(bmem != NULL);

    BIO *b64 = BIO_new(BIO_f_base64());
    assert(b64 != NULL);

    b64 = BIO_push(b64, bmem);
    BIO_write(b64, str_input, len);
    BIO_flush(b64);

    BUF_MEM *bptr;
    BIO_get_mem_ptr(b64, &bptr);
    assert(bptr != NULL);

    char *result = (char*)malloc(bptr->length+1);
    assert(result != NULL);
    memcpy(result, bptr->data, bptr->length);
    result[bptr->length] = '\0';

    BIO_free_all(b64);

    return result;
}

// 加密字符串，加密算法为"AES/CBC/PKCS5Padding"，密钥长度为128
// 返回结果需要手动释放
char* encrypt_string(const char *str2encrypt, const char *str_key)
{
    assert(str2encrypt != NULL && str_key != NULL);

    int nLen = strlen(str2encrypt);
    int nBlock = nLen / AES_BLOCK_SIZE + 1;
    int nTotal = nBlock * AES_BLOCK_SIZE;

    // KCS5Padding：填充的原则是，如果长度少于16个字节，需要补满16个字节，补(16-len)个(16-len)
    // 例如：huguPozhen这个节符串是9个字节，16-9=7，补满后如：huguozhen+7个十进制的7
    // 如果字符串长度正好是16字节，则需要再补16个字节的十进制的16
    char *str_padded = (char*)malloc(nTotal);
    assert(str_padded != NULL);
    int nNumber;
    if(nLen % 16 > 0)
        nNumber = nTotal - nLen;
    else
        nNumber = 16;
    memset(str_padded, nNumber, nTotal);
    memcpy(str_padded, str2encrypt, nLen);

    AES_KEY aes;
    const int nKeyLenght = 128;
    unsigned char iv[AES_BLOCK_SIZE];
    memset(iv, 0x00, AES_BLOCK_SIZE);
    unsigned char *key = HexString2UcharArray(str_key);

    assert(AES_set_encrypt_key(key, nKeyLenght, &aes) >= 0);
    free(key);

    unsigned char *encrypted = (unsigned char*)malloc(nLen+1);
    assert(encrypted != NULL);

    AES_cbc_encrypt((unsigned char*)str_padded, encrypted, nBlock*16, &aes, (unsigned char*)iv, AES_ENCRYPT);
    free(str_padded);

    char *result = encode_base64(encrypted, nLen+1);
    free(encrypted);
    return result;
}

// 加密数据
// 返回结果需要手动释放
char* obtain_encrypted_string(const char *str_json, const char *md5_rule)
{
    assert(str_json != NULL && md5_rule != NULL);

    char *md5_1 = generate_md5(md5_rule, strlen(md5_rule));
    char *str_key = generate_md5(md5_1, strlen(md5_1));
    free(md5_1);

    char *result = encrypt_string(str_json, str_key);
    free(str_key);
    return result;
}


int main()
{
    const char *md5Rule = "2f0ab7bdfdc40e3f5b027cd6-d775-4c19-933f-1469b073016b807b97c5aab547b6ac5d0fbb24894e60";
    const char *pToEncrypt = "{\"uniqueId\":\"02c84ff1-18a0-4b24-ba17-39227a6b641b\",\"key\":\"807b97c5aab547b6ac5d0fbb24894e60\",\"os_version\":\"1.5\"}";
    char *result = obtain_encrypted_string(pToEncrypt, md5Rule);
    cout<<result<<endl;

    std::string enc_data = std::string(result);
    free(result);

    for(string::iterator it = enc_data.begin(); it != enc_data.end(); )
    {
        if(*it == '\n')
            it = enc_data.erase(it);
        else
            it++;
    }

    cout<<enc_data<<endl;

/*
    const char *pstring = "2f0ab7bdfdc40e3fab2d4a6c-50e1-489a-97d8-873aa2743f19807b97c5aab547b6ac5d0fbb24894e60";
    char *str_md5 = generate_md5(pstring, strlen(pstring));
    cout<<str_md5<<endl;        //a8fa7657571fb458fdf5c1330bd29582
    char *str_md5_md5 = generate_md5(str_md5, strlen(str_md5));
    cout<<str_md5_md5<<endl;    //377509594e0d03804e144402d5768fb3
    free(str_md5_md5);
    free(str_md5);

    AES_KEY aes;
    //char key[33] = "377509594e0d03804e144402d5768fb3";
    //unsigned char key[] = {0x37,0x75,0x09,0x59,0x4e,0x0d,0x03,0x80,0x4e,0x14,0x44,0x02,0xd5,0x76,0x8f,0xb3};
    unsigned char* key = HexString2UcharArray("377509594e0d03804e144402d5768fb3");
    unsigned char iv[AES_BLOCK_SIZE];
    memset(iv, 0x00, AES_BLOCK_SIZE);
    const char *pToEncrypt = "{\"uniqueId\":\"42ef3448-ae0c-4b2a-bd26-7322affe7634\",\"key\":\"807b97c5aab547b6ac5d0fbb24894e60\",\"os_version\":\"1.5\"}";
    int nLen = strlen(pToEncrypt);
    cout<<nLen<<endl;

    int nBei = nLen / AES_BLOCK_SIZE + 1;
    int nTotal = nBei * AES_BLOCK_SIZE;
    char *enc_s = (char*)malloc(nTotal);
    int nNumber;
    if(nLen % 16 > 0)
        nNumber = nTotal - nLen;
    else
        nNumber = 16;
    memset(enc_s, nNumber, nTotal);
    memcpy(enc_s, pToEncrypt, nLen);

    if(AES_set_encrypt_key((unsigned char*)key, 128, &aes) < 0) {
        cout<<"Unable to set encryption key in AES"<<endl;
        return -1;
    }

    unsigned char *encrypt_string = (unsigned char*)malloc(nLen+1);
    AES_cbc_encrypt((unsigned char*)enc_s, encrypt_string, nBei*16, &aes, (unsigned char*)iv, AES_ENCRYPT);
    cout<<encode_base64(encrypt_string, nLen+1)<<endl;

    return 0;
*/
}

