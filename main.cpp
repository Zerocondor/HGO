#include <iostream>
#include <map>
#include "HGO"

using namespace HGO::CHAIN;
using namespace HGO::EXCEPTION;
using namespace HGO::TOKEN;

using ARGS = std::map<std::string, std::string>;
ARGS parseArgs(int argc, char ** argv)
{
    ARGS lst;
    if(argc > 1)
    {
        for( int i = 1; i < argc; ++i)
        {
            std::string command(argv[i]);
            if(command.substr(0,2) == "--")
            {
                std::string::size_type posEqual = command.find_first_of('=');
                std::string commandName;
                std::string value;
                if(posEqual == std::string::npos)
                {
                    commandName = command.substr(2);
                    value = "";
                    
                } else {
                    commandName = std::string(command.begin() + 2, command.begin() + posEqual);
                    value = std::string(command.begin() + posEqual + 1, command.end());
                }
                lst[commandName] = value;   
            }
        }
    }
    return std::move(lst);
}

bool hasOption(const ARGS & args, const std::string & search)
{
    ARGS::const_iterator found = args.find(search);
    if(found == args.cend())
        return false;

        return true;

}

void printHeader()
{
    std::cout<<"****************************************\n"
    <<"\t\t\033[31mHGO Blockchain\033[0m\n"
    <<"****************************************\n";
}

bool testHandler(const EVENTS::ChainEvent & evt)
{
    std::cout<<"Test des events\n\t";
    evt.print();
    std::cout<<"\n";
    std::cout.flush();
    return true;
}


int main(int argc, char ** argv)
{

    ARGS commands = parseArgs(argc, argv);
    printHeader();

    EVENTS::NewBlockEvent nb;
    EVENTS::NewTransactionEvent nt;
    
    EVENTS::ChainEventManager mgr;
    mgr.registerCallback(testHandler);
    mgr.dispatchEvent(nb);
    mgr.dispatchEvent(nt);
    std::cin.get();


    return 0;
    //Has init
    if(hasOption(commands, "init"))
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
        baseTx.amount = 60000000000L;
        baseTx.token = "HGO";
        baseTx.time = std::time(nullptr);

        chain.requestTransaction(baseTx);
        chain.requestTransaction(baseTx);

        chain.save("blk.chain");
    }

    try {
        Blockchain chain = Blockchain::load("blk.chain");
        Wallet wal(chain);
        if(hasOption(commands, "wallet"))
        {
            wal.unlockWallet(commands["wallet"]);
            std::cout<<wal;
        }
        
        
        chain.save("blk.chain");

    } catch (BlockchainException e) {
        std::cout<<"HGO Error => " << e.what()<<"\n";
    }

  
    
    return 0;
}
