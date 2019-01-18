/* ebc crypto algorithm library:
     File Name        : cbcCryptolib
     Version          : 0.01
     Date             : 2019-1-5
     Software         : Wang Yueping(version 0.01)
     Language         : C++
     Development tool : Qt Creactor 4.8.0
     Description      : e-business chain crypto algorithm library header file

     (C) Copyright 2018,2019  Right Chain Technolegy Corp., LTD.
*/

#ifndef EBCCRYPTOLIB_H
#define EBCCRYPTOLIB_H

#include <stdint.h>

typedef struct {
    uint64_t val[9];
}bn512;

typedef struct {
    bn512 h, r, hc, hv;
}HASH_SIGN512;

class ebcCryptoLib
{
public:
    ebcCryptoLib();

    // 32-bits and n-bytes random
    long int random32bits(void);
    void randomNbytes(unsigned char *buf, int len);

    ////////////////////////////////////////////////////////////////////////////////

    // blake2_160/224/256/384/512 hash function
    int blake2s_160(const unsigned char *data, const long int data_len, unsigned char *digest);
    int blake2s_224(const unsigned char *data, const long int data_len, unsigned char *digest);
    int blake2s_256(const unsigned char *data, const long int data_len, unsigned char *digest);

    int blake2b_384(const unsigned char *data, const long int data_len, unsigned char  *digest);
    int blake2b_512(const unsigned char *data, const long int data_len, unsigned char  *digest);

    ////////////////////////////////////////////////////////////////////////////////

    void hashGenerateKeyPair(unsigned char *private_key, unsigned char *public_key, unsigned char *address_id);
    void hashGeneratePublicKey(const unsigned char *private_key, unsigned char *public_key);
    void hashGenerateAddress(const unsigned char *public_key, unsigned char *address_id);
    int hashPrivateKeyVerify(const unsigned char *private_key);
    int hashPublicKeyVerify(const unsigned char *public_key);
    int hashAddressVerify(const unsigned char *address_id);

    ////////////////////////////////////////////////////////////////////////////////

    void hashSignature512(unsigned char *private_key, unsigned char *public_key, unsigned char *msg, HASH_SIGN512 *sign);
    int hashSignatureVerify512(unsigned char *public_key, unsigned char *msg, HASH_SIGN512 *sign);

private:

protected:
};

#endif // EBCCRYPTOLIB_H
