
#include <iostream>
#include "HGO"

using namespace HGO::CHAIN;
using namespace HGO::EXCEPTION;

int main(int argc, char ** argv)
{

    try {

        Blockchain chain = Blockchain::load("blk.chain");
        std::string data;
        while(std::getline(std::cin, data))
        {
            if(data == "qq")
                break;

            Block nBlock(data);
            chain.addBlock(nBlock);
        }
        std::cout<<chain;
        std::cout<< "Saving chain to blk.chain";
        chain.save("blk.chain");

    } catch (BlockchainException e) {
        std::cout<<"HGO Error => " << e.what()<<"\n";
    }

  
    
    return 0;
}
