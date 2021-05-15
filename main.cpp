
#include <iostream>
#include "HGO"

using namespace HGO::CHAIN;
using namespace HGO::EXCEPTION;

int main(int argc, char ** argv)
{

    Blockchain chain;

    Block Genesis("Genesis Block");
    Block blk1("Mes premieres data");
    
    chain << Genesis << blk1;
    try {
        Blockchain chain2(chain.getChain());
        std::cout << chain << "\n\n"<<chain2;
    } catch (BlockchainException e) {
        std::cout<<"HGO Error => " << e.what()<<"\n";
    }

  
    
    return 0;
}