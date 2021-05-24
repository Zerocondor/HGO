#include <HGO>
#include "application.h"

using namespace HGO::APP;

int main(int argc, char ** argv)
{
    MasterNode node(argc, argv);

    return node.exec();
}