#ifndef UNTITLED1_TA_ENCRYPT_H
#define UNTITLED1_TA_ENCRYPT_H
#include <iostream>

namespace thinkingdata{

    using namespace std;

    class ITDEncrypt {
    public:
        virtual ~ITDEncrypt() {

        }
        virtual string symmetricEncryptType() = 0;
        virtual string asymmetricEncryptType() = 0;
        virtual void encryptDataEvent(const char* plain_text,string &encryptData) = 0;
        virtual void encryptSymmetricKey(unsigned char *key,string &rsaData) = 0;
    };

    class TDRSAEncrypt : public ITDEncrypt {
    public:
        ~TDRSAEncrypt();
        string symmetricEncryptType();
        string asymmetricEncryptType();
        void encryptDataEvent(const char* plain_text,string &encryptData);
        void encryptSymmetricKey(unsigned char *key,string &rsaData);
        TDRSAEncrypt(int version, const string& publicKey);
        TDRSAEncrypt();
        bool enableEncrypt= false;
        void updateSecretKey(int version, const string& publicKey);
        void checkUploadDataEncrypt(char* jsonChar,string &out);
    private:
        int version;
        string publicKey;
        static const int KEY_LEN = 16;
        void generate_key(unsigned char* key, int len);
        bool aes_ecb_encrypt(const unsigned char* key, const unsigned char* in, int in_len, unsigned char* out, int& out_len);
        string base64_encode(const unsigned char* in, int in_len);
        bool rsa_public_encrypt(const char* public_key, const unsigned char* in, size_t in_len, unsigned char* out, size_t& out_len);
    };

}


#endif //UNTITLED1_TA_ENCRYPT_H
