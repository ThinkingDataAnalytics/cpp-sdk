#include "ta_encrypt.h"
#include <openssl/aes.h>
#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/err.h>
#include <openssl/rsa.h>
#include <openssl/rand.h>
#include <openssl/buffer.h>
#include "ta_json_object.h"
#include "ta_cpp_helper.h"

namespace thinkingdata {

    TDRSAEncrypt::TDRSAEncrypt() {

    }

    TDRSAEncrypt::TDRSAEncrypt(int version, const string& publicKey) :version(version){
        this->publicKey = "-----BEGIN PUBLIC KEY-----\n"+publicKey+"\n-----END PUBLIC KEY-----\n";
    }

    TDRSAEncrypt::~TDRSAEncrypt(){}


    string TDRSAEncrypt::asymmetricEncryptType() {
        return "RSA";
    }

    string TDRSAEncrypt::symmetricEncryptType() {
        return "AES";
    }

    void TDRSAEncrypt::updateSecretKey(int version, const string &publicKey) {
        this->version = version;
        this->publicKey = "-----BEGIN PUBLIC KEY-----\n"+publicKey+"\n-----END PUBLIC KEY-----\n";
    }

    void TDRSAEncrypt::encryptDataEvent(const char* plain_text,string &encryptData) {
        if(!enableEncrypt){
            //Encryption is not enabled and the original data is returned directly
            encryptData = plain_text;
            return;
        }
        unsigned char key[KEY_LEN];
        generate_key(key, KEY_LEN);
        int plain_len = static_cast<int>(strlen(plain_text));
        int cipher_len = plain_len + EVP_MAX_BLOCK_LENGTH;
        unsigned char* cipher_text = new unsigned char[cipher_len];
        //AES Encryption
        if (!aes_ecb_encrypt(key, reinterpret_cast<const unsigned char*>(plain_text), plain_len, cipher_text, cipher_len)) {
            delete[] cipher_text;
            encryptData = plain_text;
            ta_cpp_helper::handleTECallback(1004,"aes encrypt error");
            return;
        }
        string strAes = base64_encode(cipher_text, cipher_len);
        delete[] cipher_text;
        string strRsa;
        encryptSymmetricKey(key,strRsa);
        if(strRsa.empty()){
            encryptData = plain_text;
            ta_cpp_helper::handleTECallback(1004,"rsa encrypt error");
            return;
        }
        //output encrypted result
        TDJSONObject json;
        json.SetNumber("pkv",this->version);
        json.SetString("ekey",strRsa);
        json.SetString("payload",strAes);
        encryptData = TDJSONObject::ToJson(json);
    }

    void TDRSAEncrypt::encryptSymmetricKey(unsigned char *key,string &rsaData) {

        const char* public_key_str = this->publicKey.c_str();
        unsigned char ciphertext[256];
        size_t ciphertext_len = 0;
        if (!rsa_public_encrypt(public_key_str, key, KEY_LEN, ciphertext, ciphertext_len)) {
            rsaData = "";
            return;
        }
        rsaData = base64_encode(ciphertext, static_cast<int>(ciphertext_len));
    }

    void TDRSAEncrypt::generate_key(unsigned char *key, int len) {
        RAND_bytes(key, len);
    }

    void TDRSAEncrypt::checkUploadDataEncrypt(char *jsonChar, string &out) {
        string uploadData = string(jsonChar);
        if(!enableEncrypt){
            //Encryption is not enabled and the original data is returned directly
            out = uploadData;
            return;
        }
        //Enable encryption to determine whether the data is encrypted
        if(uploadData.find("ekey") != string::npos && uploadData.find("pkv") != string::npos && uploadData.find("payload") != string::npos){
            //If encrypted, do not continue to encrypt
            out = uploadData;
            return;
        }else{
            //Encrypt local data and report
            encryptDataEvent(jsonChar,out);
        }
    }


    bool
    TDRSAEncrypt::aes_ecb_encrypt(const unsigned char *key, const unsigned char *in, int in_len, unsigned char *out,
                                  int &out_len) {
        EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
        if (!ctx) {
            EVP_CIPHER_CTX_free(ctx);
            return false;
        }

        if (EVP_EncryptInit_ex(ctx, EVP_aes_128_ecb(), nullptr, key, nullptr) != 1) {
            EVP_CIPHER_CTX_free(ctx);
            return false;
        }

        if (EVP_EncryptUpdate(ctx, out, &out_len, in, in_len) != 1) {
            EVP_CIPHER_CTX_free(ctx);
            return false;
        }

        int tmp_len = 0;
        if (EVP_EncryptFinal_ex(ctx, out + out_len, &tmp_len) != 1) {
            EVP_CIPHER_CTX_free(ctx);
            return false;
        }

        out_len += tmp_len;
        EVP_CIPHER_CTX_free(ctx);
        return true;
    }


    string TDRSAEncrypt::base64_encode(const unsigned char *in, int in_len) {
        BIO* bio, * b64;
        BUF_MEM* bufferPtr;

        b64 = BIO_new(BIO_f_base64());
        bio = BIO_new(BIO_s_mem());
        bio = BIO_push(b64, bio);

        BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL);
        BIO_write(bio, in, in_len);
        BIO_flush(bio);
        BIO_get_mem_ptr(bio, &bufferPtr);

        std::string result(bufferPtr->data, bufferPtr->length);

        BIO_free_all(bio);

        return result;
    }

    bool
    TDRSAEncrypt::rsa_public_encrypt(const char *public_key, const unsigned char *in, size_t in_len, unsigned char *out,
                                     size_t &out_len) {
        BIO* bio = BIO_new_mem_buf(public_key, -1);
        if (!bio) {
            return false;
        }

        EVP_PKEY* pkey = PEM_read_bio_PUBKEY(bio, nullptr, nullptr, nullptr);
        if (!pkey) {
            BIO_free(bio);
            return false;
        }

        EVP_PKEY_CTX* ctx = EVP_PKEY_CTX_new(pkey, nullptr);
        if (!ctx) {
            EVP_PKEY_free(pkey);
            BIO_free(bio);
            return false;
        }

        // Set encryption engine
        if (EVP_PKEY_encrypt_init(ctx) <= 0) {
            EVP_PKEY_free(pkey);
            EVP_PKEY_CTX_free(ctx);
            BIO_free(bio);
            return false;
        }

        // Set encryption padding method
        if (EVP_PKEY_CTX_set_rsa_padding(ctx, RSA_PKCS1_PADDING) <= 0) {
            EVP_PKEY_free(pkey);
            EVP_PKEY_CTX_free(ctx);
            BIO_free(bio);
            return false;
        }

        // encrypt
        if (EVP_PKEY_encrypt(ctx, out, &out_len, in, in_len) <= 0) {
            EVP_PKEY_free(pkey);
            EVP_PKEY_CTX_free(ctx);
            BIO_free(bio);
            return false;
        }

        EVP_PKEY_free(pkey);
        EVP_PKEY_CTX_free(ctx);
        BIO_free(bio);
        return true;
    }


}
