#include <string>
#include <iostream>
#include <cstring>
#include <openssl/hmac.h>
#include <openssl/sha.h>
#include <openssl/rsa.h>
#include <openssl/pem.h>

namespace utils { namespace crypto {

inline std::string b2a_hex(char *byte_arr, int n) {
    const static std::string HexCodes = "0123456789abcdef";
    std::string HexString;
    for(int i = 0; i < n; ++i) {
        unsigned char BinValue = byte_arr[i];
        HexString += HexCodes[(BinValue >> 4) & 0x0F];
        HexString += HexCodes[BinValue & 0x0F];
    }
    return HexString;
}

inline std::string hmac_sha256( const char *key, const char *data) {
    unsigned char* digest;
    digest = HMAC(EVP_sha256(), key, strlen(key), (unsigned char*)(data), strlen(data), NULL, NULL);
    return b2a_hex((char *)digest, 32);
}

inline unsigned char* hmac_sha256_byte( const char *key, const char *data) {
    return HMAC(EVP_sha256(), key, strlen(key), (unsigned char*)(data), strlen(data), NULL, NULL);
}

inline std::string hmac_sha384( const char *key, const char *data) {
    unsigned char* digest;
    digest = HMAC(EVP_sha384(), key, strlen(key), (unsigned char*)(data), strlen(data), NULL, NULL);
    return b2a_hex((char *)digest, 48);
}

inline unsigned char* hmac_sha384_byte( const char *key, const char *data) {
    return HMAC(EVP_sha384(), key, strlen(key), (unsigned char*)(data), strlen(data), NULL, NULL);
}

inline std::string hmac_sha512( const char *key, const char *data) {
    unsigned char* digest;
    digest = HMAC(EVP_sha256(), key, strlen(key), (unsigned char*)(data), strlen(data), NULL, NULL);
    return b2a_hex((char *)digest, 64);
}

static const std::string base64_chars ="ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                                       "abcdefghijklmnopqrstuvwxyz"
                                       "0123456789+/";

inline bool is_base64(unsigned char c) {
    return (isalnum(c) || (c == '+') || (c == '/'));
}

std::string base64_encode(unsigned char const* bytes_to_encode, unsigned int in_len) {
    std::string ret;
    int i = 0;
    int j = 0;
    unsigned char char_array_3[3];
    unsigned char char_array_4[4];

    while (in_len--) {
        char_array_3[i++] = *(bytes_to_encode++);
        if (i == 3) {
            char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
            char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
            char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
            char_array_4[3] = char_array_3[2] & 0x3f;

            for(i = 0; (i <4) ; i++)
                ret += base64_chars[char_array_4[i]];
            i = 0;
        }
    }

    if (i)
    {
        for(j = i; j < 3; j++)
            char_array_3[j] = '\0';

        char_array_4[0] = ( char_array_3[0] & 0xfc) >> 2;
        char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
        char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);

        for (j = 0; (j < i + 1); j++)
            ret += base64_chars[char_array_4[j]];

        while((i++ < 3))
            ret += '=';

    }

    return ret;

}

std::string base64_decode(std::string const& encoded_string) {
    int in_len = encoded_string.size();
    int i = 0;
    int j = 0;
    int in_ = 0;
    unsigned char char_array_4[4], char_array_3[3];
    std::string ret;

    while (in_len-- && ( encoded_string[in_] != '=') && is_base64(encoded_string[in_])) {
        char_array_4[i++] = encoded_string[in_]; in_++;
        if (i ==4) {
            for (i = 0; i <4; i++)
                char_array_4[i] = base64_chars.find(char_array_4[i]);

            char_array_3[0] = ( char_array_4[0] << 2       ) + ((char_array_4[1] & 0x30) >> 4);
            char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
            char_array_3[2] = ((char_array_4[2] & 0x3) << 6) +   char_array_4[3];

            for (i = 0; (i < 3); i++)
                ret += char_array_3[i];
            i = 0;
        }
    }

    if (i) {
        for (j = 0; j < i; j++)
            char_array_4[j] = base64_chars.find(char_array_4[j]);

        char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
        char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);

        for (j = 0; (j < i - 1); j++) ret += char_array_3[j];
    }

    return ret;
}



inline std::string rsa256_private_encrypt(const std::string &data_to_sign, const std::string &priKey) {
    std::string strRet;
    RSA *rsa = NULL;
    BIO *keybio = BIO_new_mem_buf((unsigned char *)priKey.c_str(), -1);

    rsa = PEM_read_bio_RSAPrivateKey(keybio, &rsa, NULL, NULL);
    int len = RSA_size(rsa);

    char *encryptedText = (char *)malloc(len + 1);
    memset(encryptedText, 0, len + 1);
    int ret = RSA_private_encrypt(data_to_sign.length(), (const unsigned char*)data_to_sign.c_str(), (unsigned char*)encryptedText, rsa, RSA_PKCS1_PADDING);

    if (ret >= 0) {
        strRet = std::string(encryptedText, ret);
    }

    free(encryptedText);
    BIO_free_all(keybio);
    RSA_free(rsa);
    return strRet;
}



inline std::string rsa256_pub_decrypt(const std::string &cipherText, const std::string &pubKey)
{
    std::string strRet;
    RSA *rsa = RSA_new();
    BIO *keybio;
    keybio = BIO_new_mem_buf((unsigned char *)pubKey.c_str(), -1);

    rsa = PEM_read_bio_RSAPublicKey(keybio, &rsa, NULL, NULL);

    int len = RSA_size(rsa);
    char *decryptedText = (char *)malloc(len + 1);
    memset(decryptedText, 0, len + 1);
    int ret = RSA_public_decrypt(cipherText.length(), (const unsigned char*)cipherText.c_str(), (unsigned char*)decryptedText, rsa, RSA_PKCS1_PADDING);

    if (ret >= 0) {
        strRet = std::string(decryptedText, ret);
    }

    free(decryptedText);
    BIO_free_all(keybio);
    RSA_free(rsa);
    return strRet;
}

}}
