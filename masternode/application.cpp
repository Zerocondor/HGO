#include "application.h"

using namespace HGO::APP;
using namespace HGO::CHAIN;
using namespace HGO::NETWORK;
using namespace HGO::EXCEPTION;

MasterNode::MasterNode(int argc, char **argv)
    : _chain(Blockchain::load("chain.blk"))
{
    _parseArguments(argc, argv);
    using namespace std::placeholders;
    _chain.eventManager().registerCallback(std::bind(&MasterNode::_handleChainEvent, this, _1));
    HGO::NETWORK::P2PServer::MESSAGE_CALLBACK cb = std::bind(&MasterNode::_handleP2PEvent, this, _1);
    _network.setBlockchainHandlers(cb, cb); 
}

MasterNode::~MasterNode()
{}

bool MasterNode::_handleChainEvent(const HGO::CHAIN::EVENTS::ChainEvent &ev)
{
    if(ev.eventName() == "NEW_BLOCK")
    {
        Message msg;
        msg.header.config.full_header = 0b00011100;
        msg.msg_type = Message::TYPE::NEW_BLOCK;

        _network.broadcast(msg);
    }
    return true;
}
void MasterNode::_handleP2PEvent(const HGO::NETWORK::Message &msg)
{
    std::cout<<"P2P Event\n";
}

int MasterNode::exec()
{
    unsigned short default_port = 2016;
    std::string tag = "";
    HGOPeer firstPeerToReach;
    
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
    std::cout<<"Initialisation ....\n";
    _network.setTagName(tag);
    _network.setMasterNode(true);
    _network.startNetwork(default_port);

    std::cout<<"HGO Blockchain - \033[32m[Status OK]\033[0m\n";
    std::cout<<"Current Block : \n"<<_chain.getLastBlock()<<"\n";
    
    if(!firstPeerToReach.ip_address.empty())
    {
        _network.connectToNode(firstPeerToReach.ip_address, firstPeerToReach.port);
    }


    while(std::cin.get() != 'q')
        ;;
    _chain.save("chain.blk");
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