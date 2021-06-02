#include "application.h"

using namespace HGO::APP;
using namespace HGO::CHAIN;
using namespace HGO::NETWORK;
using namespace HGO::EXCEPTION;
using namespace std::placeholders;

Wallet::Wallet(int argc, char **argv)
    : _wallet(nullptr)
{
    _parseArguments(argc, argv);
    
    HGO::NETWORK::P2PServer::MESSAGE_CALLBACK cb = std::bind(&Wallet::_handleP2PEvent, this, _1, _2);
    _network.setBlockchainHandlers(cb);
}

Wallet::~Wallet()
{}

bool Wallet::_handleChainEvent(const HGO::CHAIN::EVENTS::ChainEvent &ev)
{
    if(ev.eventName() == "NEW_BLOCK")
    {
        std::cout<<"Send Block\n"<<ev.data();
        Message msg;
        msg.header.config.full_header = 0b00011100;
        msg.msg_type = Message::TYPE::NEW_BLOCK;
        msg.str = ev.data();
        msg.msg_size = msg.str.size();

        _network.pushTickMessage(nullptr, msg); 
    }

    return true;
}
void Wallet::_handleP2PEvent(const HGOPeer &peer, const Message &msg)
{
    using TYPE = Message::TYPE;
    Block blk;
    switch(msg.msg_type)
    {
        case TYPE::ACQUITED:
            _checkSynchState(peer);  
        break;
        case TYPE::NEW_BLOCK:
            blk = Block::unserialize(msg.str);
            _wallet->update(blk);
            std::cout<<*_wallet<<"\n";
            _chain << blk;
        break;
        case TYPE::REQUEST_BLOCK:
            _processRequestBlock(peer, msg);
        break;
        case TYPE::SYNCHRONIZE:
            _checkBlockID(peer, msg);
        break;
    }
}

void Wallet::_checkSynchState(const HGOPeer & peer)
{
    Message msg;
    msg.header.config.full_header = 0b00001000;
    msg.msg_type = Message::TYPE::SYNCHRONIZE;

    msg.str = std::to_string(_chain.getLastBlockID());
    msg.msg_size = msg.str.size();

    _network.pushTickMessage(std::make_shared<HGOPeer>(peer), msg); 
}

void Wallet::_checkBlockID(const HGO::NETWORK::HGOPeer & peer, const HGO::NETWORK::Message &msg)
{
    std::cout<<" My current block ID : "<< _chain.getLastBlockID()<< " | Block ID from peer: "<<msg.str<<"\n";
    std::istringstream iss(msg.str);
    Block::BLOCK_INDEX idxFromPeer;
    iss>>idxFromPeer;

    if(_chain.getLastBlockID() > idxFromPeer) {
        std::cout<<" Im in advance from you\n";
        _sendBlocks(peer, idxFromPeer + 1);

    } else if (_chain.getLastBlockID() < idxFromPeer) {
        std::cout<<" Im in late need to request from block ID :" << _chain.getLastBlockID() << "\n";

        Message msg;
        msg.header.config.full_header = 0b00001000;
        msg.msg_type = Message::TYPE::REQUEST_BLOCK;
        msg.str = std::to_string(_chain.getLastBlockID());
        msg.msg_size = msg.str.size();

        _network.pushTickMessage(std::make_shared<HGOPeer>(peer), msg);

    } else {
        std::cout<<" We are equal nothing to do\n";
    }
}

void Wallet::_processRequestBlock(const HGO::NETWORK::HGOPeer & peer, const HGO::NETWORK::Message &msg)
{
    std::istringstream iss(msg.str);
    Block::BLOCK_INDEX idxBlockFrom;
    iss>>idxBlockFrom;
    _sendBlocks(peer, idxBlockFrom);
}

void Wallet::_sendBlocks(const HGO::NETWORK::HGOPeer & peer, const HGO::CHAIN::Block::BLOCK_INDEX &fromBlock)
{
    Blockchain::BLOCK_LIST _lst = _chain.getChain();
    if(_lst.size() < fromBlock)
        return;

    Blockchain::BLOCK_LIST::const_iterator it = _lst.cbegin() + fromBlock;
    Message msg;
    msg.header.config.full_header = 0b00001000;
    msg.msg_type = Message::TYPE::NEW_BLOCK;
    while(it != _lst.cend())
    {
        msg.str = it->serialize();
        msg.msg_size = it->serialize().size();
        _network.pushTickMessage(std::make_shared<HGOPeer>(peer), msg);
        ++it;
    }
}

int Wallet::exec()
{
    
    unsigned short default_port = 2016;
    std::string tag = "";
    HGOPeer firstPeerToReach;
    std::string chainName = "chain.blk";
    std::string walletFile = "default.wal";
    
    std::cout<<"\033[32m"<<WALLET_BANNER<<"\033[0m\n";
    if(_hasOption("help"))
    {
        _printHelp();
        return 0;
    }

    if(_hasOption("port"))
    {
        std::istringstream iss(_list["port"]);
        iss >> default_port;
    }

    if(_hasOption("peer"))
    {
        firstPeerToReach = HGOPeer::fromString(_list["peer"]);
    }

    if(_hasOption("tag"))
    {
        tag = _list["tag"];
    }

    if(_hasOption("chain"))
    {
        chainName = _list["chain"];
    }

    if(_hasOption("wal"))
    {
        walletFile = _list["wal"];
    }

    if(_hasOption("create"))
    {
        std::cout<<"Create Wallet on "<<walletFile<<"\n";
        HGO::TOKEN::Wallet::createWallet(walletFile);
        return 0;
    }

    std::cout << WALLET_NODE_FOOTPRINT<<"\n";
    std::cout<<"Initialisation ....\n";
    _chain = Blockchain::load(chainName);
    _chain.eventManager().registerCallback(std::bind(&Wallet::_handleChainEvent, this, _1));

    HGO::TOKEN::Wallet w(_chain);
    _wallet = &w;
    _network.setTagName(tag);
    _network.setMasterNode(false);
    _network.startNetwork(default_port);

    std::cout<<"HGO Blockchain - \033[32m[Status OK]\033[0m\n";
    if(_chain.getChain().empty()) 
    {
        std::cout<<"Chain is empty and will be synched on network\n";
    }

    if(!_wallet->unlockWallet(walletFile))
    {
        std::cout<<"Could not open the wallet file : "<<walletFile<<"\n";
        return -1;
    } else {
        std::cout<<*_wallet<<"\n";
    }
    
    if(!firstPeerToReach.ip_address.empty())
    {
        _network.connectToNode(firstPeerToReach.ip_address, firstPeerToReach.port);
    }

    std::string command;
    while(std::cin>>command)
    {
        if(command == "q")
            break;

        if(command == "send") {
            std::string address;
            long double amount = 0.0L;
            if(std::cin>>address>>amount) {

                HGO::TOKEN::Transaction tx = _wallet->buildTransaction(address, amount);
                _wallet->signTransaction(tx);
                Message msg;
                msg.header.config.full_header = 0b00001000;
                msg.msg_type = Message::TYPE::NEW_TRANSACTION;
                msg.str = tx.serialize();
                msg.msg_size = msg.str.size();

                _network.pushTickMessage(nullptr, msg);     
            }
        }
    }

    _chain.save(chainName);
    return 0;
}

void Wallet::_printHelp()
{
    std::cout<<R"f(
When wallet is running send command should be used to send token to another wallet address
    send [addr] [amount]

    ex: send 8aff694da3f2ed5e28214d1daab42e7401fbc99b119e281a2e72a0d8a6f1290f 30000.21155

--create (default "default.wal")
    Use --wal option to define a specific name
--port=[Listening port](default 2016)
--peer=[First node to reach](ip:port)
--tag=[Name on network]
--chain=[Filename of the chain] (default "chain.blk")
--wal=[Filename of wallet] (default "default.wal")
)f";

}

bool Wallet::_hasOption(const std::string & option)
{
    return (std::find_if(_list.cbegin(), _list.cend(), [&option](const std::pair<std::string,std::string> &other){
        return other.first==option;
    }) != _list.cend());
}

void Wallet::_parseArguments(int argc, char ** argv)
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