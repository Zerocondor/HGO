
#include <iostream>
#include "HGO"

using namespace HGO::CHAIN;

int main(int argc, char ** argv)
{

    Blockchain chain;

    Block Genesis("Genesis Block");
    Block blk1("Mes premieres data");
    
    chain << Genesis << blk1;

    Blockchain chain2(chain.getChain());
    std::cout << chain << "\n\n"<<chain2;
    
    return 0;
}