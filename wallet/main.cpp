#include <HGO>
#include "application.h"

using namespace HGO::APP;

int main(int argc, char ** argv)
{
    Wallet node(argc, argv);

    return node.exec();
}