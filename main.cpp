
#include <iostream>
#include "HGO"

using namespace HGO::CHAIN;
using namespace HGO::EXCEPTION;
using namespace HGO::TOKEN;

int main(int argc, char ** argv)
{

    if(argc > 1 && std::string(argv[1]) == "--init")
    {
        Block genesis("HGO Genesis block");
        Blockchain chain;
        chain << genesis;

        //Install initial Amounts
        Wallet nicoWallet(chain);
        nicoWallet.unlockWallet("test/datas/nico.wal");
        Transaction baseTx;
        baseTx.from = Wallet::ANONYMOUS_ADDRESS;
        baseTx.to = nicoWallet.getAddress();
        baseTx.amount = 200000000L;
        baseTx.token = "HGO";
        baseTx.time = std::time(nullptr);

        chain.requestTransaction(baseTx);
        chain.requestTransaction(baseTx);

        chain.save("blk.chain");
    }

    try {
        Blockchain chain = Blockchain::load("blk.chain");
        Wallet nicoWallet(chain), other(chain);
        nicoWallet.unlockWallet("test/datas/nico.wal");
        other.unlockWallet("test/datas/other.wal");
        nicoWallet.send(other.getAddress(), 225.32L);
        //nicoWallet.send(nicoWallet.getAddress(), 0);
        std::cout<<nicoWallet<< other;
        
        chain.save("blk.chain");

    } catch (BlockchainException e) {
        std::cout<<"HGO Error => " << e.what()<<"\n";
    }

  
    
    return 0;
}
