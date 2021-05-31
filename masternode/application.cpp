#include "application.h"

using namespace HGO::APP;
using namespace HGO::CHAIN;
using namespace HGO::NETWORK;
using namespace HGO::EXCEPTION;
using namespace std::placeholders;

MasterNode::MasterNode(int argc, char **argv)
{
    _parseArguments(argc, argv);
   
    HGO::NETWORK::P2PServer::MESSAGE_CALLBACK cb = std::bind(&MasterNode::_handleP2PEvent, this, _1, _2);
    _network.setBlockchainHandlers(cb); 
}

MasterNode::~MasterNode()
{}

bool MasterNode::_handleChainEvent(const HGO::CHAIN::EVENTS::ChainEvent &ev)
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

void MasterNode::_handleP2PEvent(const HGOPeer &peer, const Message &msg)
{
    using TYPE = Message::TYPE;

    switch(msg.msg_type)
    {
        case TYPE::ACQUITED:
            _checkSynchState(peer);  
        break;
        case TYPE::NEW_BLOCK:
            std::cout<<"\033[32m[NEW BLOCK RECEIVED]\033[0m\n";
            _chain << Block::unserialize(msg.str);
        break;
        case TYPE::NEW_TRANSACTION:
            std::cout<<"\033[32m[NEW TX RECEIVED] - \033[0m\n" <<HGO::TOKEN::Transaction::unserialize(msg.str)<<"\n";
            _chain.requestTransaction(HGO::TOKEN::Transaction::unserialize(msg.str));
        break;
        case TYPE::REQUEST_BLOCK:
            _processRequestBlock(peer, msg);
        break;
        case TYPE::SYNCHRONIZE:
            _checkBlockID(peer, msg);
        break;
    }
}

void MasterNode::_checkSynchState(const HGOPeer & peer)
{
    Message msg;
    msg.header.config.full_header = 0b00011100;
    msg.msg_type = Message::TYPE::SYNCHRONIZE;

    msg.str = std::to_string(_chain.getLastBlockID());
    msg.msg_size = msg.str.size();

    _network.pushTickMessage(std::make_shared<HGOPeer>(peer), msg); 
}

void MasterNode::_checkBlockID(const HGO::NETWORK::HGOPeer & peer, const HGO::NETWORK::Message &msg)
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
        msg.header.config.full_header = 0b00011100;
        msg.msg_type = Message::TYPE::REQUEST_BLOCK;
        msg.str = std::to_string(_chain.getLastBlockID());
        msg.msg_size = msg.str.size();

        _network.pushTickMessage(std::make_shared<HGOPeer>(peer), msg);

    } else {
        std::cout<<" We are equal nothing to do\n";
    }
}

void MasterNode::_processRequestBlock(const HGO::NETWORK::HGOPeer & peer, const HGO::NETWORK::Message &msg)
{
    std::istringstream iss(msg.str);
    Block::BLOCK_INDEX idxBlockFrom;
    iss>>idxBlockFrom;
    _sendBlocks(peer, idxBlockFrom);
}

void MasterNode::_sendBlocks(const HGO::NETWORK::HGOPeer & peer, const HGO::CHAIN::Block::BLOCK_INDEX &fromBlock)
{
    Blockchain::BLOCK_LIST _lst = _chain.getChain();
    if(_lst.size() < fromBlock)
        return;

    Blockchain::BLOCK_LIST::const_iterator it = _lst.cbegin() + fromBlock;
    Message msg;
    msg.header.config.full_header = 0b00011100;
    msg.msg_type = Message::TYPE::NEW_BLOCK;
    while(it != _lst.cend())
    {
        msg.str = it->serialize();
        msg.msg_size = it->serialize().size();
        _network.pushTickMessage(std::make_shared<HGOPeer>(peer), msg);

        ++it;
    }
}

void MasterNode::_printHelp()
{
    std::cout<<R"f(
First master node of the network should be started only as server (omit --peer option)

--port=[Listening port](default 2016)
--peer=[First node to reach](ip:port)
--tag=[Name on network]
--chain=[Filename of the chain] (default "chain.blk")
)f";

}

int MasterNode::exec()
{
    unsigned short default_port = 2016;
    std::string tag = "";
    HGOPeer firstPeerToReach;
    std::string chainName = "chain.blk";
    std::cout<<"\033[034m"<<MASTERNODE_BANNER<<"\033[0m\n";
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
    std::cout << MASTER_NODE_FOOTPRINT<<"\n";
    std::cout<<"Initialisation ....\n";
    _chain = Blockchain::load(chainName);
    _chain.eventManager().registerCallback(std::bind(&MasterNode::_handleChainEvent, this, _1));
    _network.setTagName(tag);
    _network.setMasterNode(true);
    _network.startNetwork(default_port);

    std::cout<<"HGO Blockchain - \033[32m[Status OK]\033[0m\n";
    if(_chain.getChain().empty()) 
    {
        std::cout<<"Chain is empty and will be synched on network\n";
    } else {
        std::cout<<"Current Block : \n"<<_chain.getLastBlock()<<"\n";
    }
    
    if(!firstPeerToReach.ip_address.empty())
    {
        _network.connectToNode(firstPeerToReach.ip_address, firstPeerToReach.port);
    }


    while(std::cin.get() != 'q')
        ;;

    _chain.save(chainName);
    return 0;
}

bool MasterNode::_hasOption(const std::string & option)
{
    return (std::find_if(_list.cbegin(), _list.cend(), [&option](const std::pair<std::string,std::string> &other){
        return other.first==option;
    }) != _list.cend());
}

void MasterNode::_parseArguments(int argc, char ** argv)
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