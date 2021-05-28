#include <HGO>
#include "application.h"
#include <openssl/bio.h>

#include <openssl/evp.h>
#include <openssl/rsa.h>
#include <openssl/pem.h>

using namespace HGO::APP;
using namespace HGO::CRYPTO;

int main(int argc, char ** argv)
{
    Wallet node(argc, argv);

    return node.exec();
}