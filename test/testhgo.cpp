#include <iostream>
#include <map>
#include <HGO>

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


bool eventHandler(const EVENTS::ChainEvent &ev)
{
    std::cout<<"\033[33m[NEW EVENT] - "<<ev.eventName()<<"\033[0m - New event received\n";
    return true;
}

int main(int argc, char ** argv)
{

    ARGS commands = parseArgs(argc, argv);
    printHeader();

    //Has init
    if(hasOption(commands, "init"))
    {
        Block genesis("HGO Genesis block");
        Blockchain chain;
        chain.createBlock(genesis);

        //Install initial Amounts
        Wallet nicoWallet(chain);
        nicoWallet.unlockWallet("niko.wal");
        Transaction baseTx;
        baseTx.from = nicoWallet.getAddress();
        baseTx.to = nicoWallet.getAddress();
        baseTx.amount = 60000000000L;
        baseTx.token = "HGO";
        baseTx.time = std::time(nullptr);

        nicoWallet.signTransaction(baseTx);
        chain.requestTransaction(Transaction::unserialize(baseTx.serialize()));
        chain.requestTransaction(Transaction::unserialize(baseTx.serialize()));
        //chain.requestTransaction(baseTx);
        //chain.requestTransaction(baseTx);
        chain.save("blk.chain");
    }

    try {
        Blockchain chain = Blockchain::load("blk.chain");
        chain.eventManager().registerCallback(eventHandler);
        if(hasOption(commands, "wallet"))
        {
            Wallet wal(chain);
            wal.unlockWallet(commands["wallet"]);
            std::cout<<wal;

            

            chain.eventManager().registerCallback([&wal](const EVENTS::ChainEvent &ev) -> bool {
                if(ev.eventName() == "NEW_BLOCK") {
                    std::cout<<"\033[32m Wallet printed from event \033[0m\n"<<wal;
                }
                return true;
            });

            
            
            std::cin.get();
   
            
        }
        
        
        chain.save("blk.chain");

    } catch (BlockchainException e) {
        std::cout<<"HGO Error => " << e.what()<<"\n";
    }

  
    
    return 0;
}
