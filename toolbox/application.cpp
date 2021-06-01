#include "application.h"

using namespace HGO::APP;
using namespace HGO::CHAIN;
using namespace HGO::NETWORK;
using namespace HGO::EXCEPTION;
using namespace std::placeholders;

Toolbox::Toolbox(int argc, char **argv)
{
    _parseArguments(argc, argv);
}

Toolbox::~Toolbox()
{}


void Toolbox::_printHelp()
{
    std::cout<<R"f(
Some tools to manage HGO Blockchain

######### Blockchain initialisation #########
--init=[Blockchain Filename] (Create new blockchain defaultname chain.blk)
--wal_address=[Address] (Target address)
--amount=[Amount] (amount to send on --wal_address on --init)
--genesis_data=[Data] (Raw data to fill the genesis block)

######### Wallet Management #########
--create=[WalletFilename] (Create a new wallet)
--wallet=[Wallet Filename] (display wallet information)
--print_transaction==[chain filename] (print all transaction attached to the wallet in the chain)

######### Blockchain explorer #########
--chain=[Chain filename]
--print_block=[blockid]
)f";

}

void Toolbox::_initBlockchain(const std::string &filename, const std::string & genesis_block_data, const std::string & addr, const long double & amount) const
{
    Blockchain chain;
    Block blk(genesis_block_data);
    chain.createBlock(blk);

    if(!addr.empty())
    {
        HGO::CRYPTO::KeyPair key = HGO::CRYPTO::KeyPair::generate();

        HGO::TOKEN::Transaction baseTx;
        baseTx.from = sha256(key.getRawPublicKey());
        baseTx.to = addr;
        baseTx.amount = amount;
        baseTx.token = "HGO";
        baseTx.time = std::time(nullptr);
        baseTx.signature = key.sign(baseTx.getHash());
        baseTx.public_key = key.getRawPublicKey();


        chain.requestTransaction(baseTx, true);
        
    }

    chain.save(filename);
}

void Toolbox::_readWallet(const std::string &filename, const std::string & chainFileName) const
{
    Blockchain chain;
    if(chainFileName.empty())
    {
        HGO::TOKEN::Wallet wallet (chain);
        wallet.unlockWallet(filename);
        std::cout<<"Wallet Address: "<<wallet.getAddress()<<"\n"
        <<((wallet.getKeys().hasPrivate()) ? "Private Key is available\n" : "Private key is not available\n")
        <<((wallet.getKeys().hasPublic()) ? "Public Key is available\n" : "Public key is not available\n");
    } else {
        chain = Blockchain::load(chainFileName);
        HGO::TOKEN::Wallet wallet (chain);
        wallet.unlockWallet(filename);
        std::cout<<wallet<<"\n"
        <<((wallet.getKeys().hasPrivate()) ? "Private Key is available\n" : "Private key is not available\n")
        <<((wallet.getKeys().hasPublic()) ? "Public Key is available\n" : "Public key is not available\n");
        for(const auto & tx : wallet.getTransactions())
        {
            std::cout<<tx<<"\n";
        }
    }
}

void Toolbox::_exploreChain(const std::string &filename, const HGO::CHAIN::Block::BLOCK_INDEX & idx) const
{
    Blockchain chain = Blockchain::load(filename);
    if(chain.getChain().size() > idx) 
    {
        std::cout<<chain.getChain()[idx];
    } else {
        std::cout<<" Block ID : "<<idx<< "is not available, last block ID is: "<<chain.getLastBlockID()<<"\n";
    }
    
}

void Toolbox::_exploreChain(const std::string &filename) const
{
    Blockchain chain = Blockchain::load(filename);
    std::cout<<chain<<"\n";
}

int Toolbox::exec()
{
    std::string chainName = "chain.blk";
    std::string genesis_block_data = R"f(Genesis Block generated from toolbox, HGO 08/08/2016)f";
    bool gen_first_transaction = false;

    std::cout<<"\033[033m"<<TOOLBOX_BANNER<<"\033[0m\n";
    if(_hasOption("help") || _list.empty())
    {
        _printHelp();
        return 0;
    }

    if(_hasOption("init"))
    {
        if(!_list["init"].empty())
        {
            chainName = _list["init"];
        }

        std::cout<<"Create new blockchain on : "<<chainName<<"\n";

        if(_hasOption("genesis_block"))
        {
            genesis_block_data = _list["genesis_block"];
        }
        if(_hasOption("wal_address") && _hasOption("amount"))
        {
            gen_first_transaction = true;
        }

        if(gen_first_transaction)
        {
            long double amount;
            std::string addr = _list["wal_address"];
            std::istringstream iss(_list["amount"]);
            if(iss>> amount)
            {
                std::cout<<"Add transaction at \n\t"<<addr<<"\n\tamount : "<<std::setprecision(std::numeric_limits<long double>::digits10 + 1)<<amount<<"\n";
                _initBlockchain(chainName, genesis_block_data, addr, amount);
            }
            
        } else {
            
            _initBlockchain(_list["init"], genesis_block_data);
        }
    }

    if(_hasOption("create") && !_list["create"].empty())
    {
        HGO::TOKEN::Wallet::createWallet(_list["create"]);
    }

    if(_hasOption("wallet") && !_list["wallet"].empty())
    {
        _readWallet(_list["wallet"],_list["print_transaction"]);
    }

    if(_hasOption("chain") && !_list["chain"].empty())
    {
        if(_hasOption("print_block") && !_list["print_block"].empty())
        {
            HGO::CHAIN::Block::BLOCK_INDEX idx;
            std::istringstream iss(_list["print_block"]);
            if(iss >> idx) {
                _exploreChain(_list["chain"], idx);
            } else {
                _exploreChain(_list["chain"]);
            }
        } else {
            _exploreChain(_list["chain"]);
        }
    }

    

    
    return 0;
}

bool Toolbox::_hasOption(const std::string & option)
{
    return (std::find_if(_list.cbegin(), _list.cend(), [&option](const std::pair<std::string,std::string> &other){
        return other.first==option;
    }) != _list.cend());
}

void Toolbox::_parseArguments(int argc, char ** argv)
{
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
                _list[commandName] = value;   
            }
        }
    }
}