#include "p2p_server.h"
#include "../exceptions.h"

using namespace HGO::NETWORK;
using namespace HGO::EXCEPTION;

P2PServer::P2PServer()
{}
P2PServer::~P2PServer()
{}

void P2PServer::startNetwork(const unsigned short &port)
{
    using namespace std::placeholders;
    HGONetworkManager::EVENT_CALLBACK bindedCallback = std::bind(&P2PServer::_p2phandler, this, _1, _2, _3, _4);
    _network.addCallback(bindedCallback);
    _network.run(port);
}

void P2PServer::stopNetwork()
{
    _network.stop();
}

void P2PServer::connectToNode(const std::string &ip_address, const unsigned short &port)
{
    if(!_network.connectToPeer(ip_address, port))
    {
        throw P2PError("Cannot connect to peer : " + ip_address + ":" + std::to_string(port));
    }
}
void P2PServer::_requestPeerInfos(const HGOPeer &p)
{
    Message portMessage;
    portMessage.header.flags.isRequest = true;
    portMessage.header.flags.isResponse = false;
    portMessage.header.flags.isMasterNode = false;
    portMessage.header.flags.isOfficialNode =  true;
    portMessage.header.flags.isTest = true;
    portMessage.header.flags.isForward = false;

    portMessage.msg_type = Message::TYPE::PEER_INFORMATIONS;
    portMessage.msg_size = 0;
    portMessage.str = "";

    Message port = sendMessage(p, portMessage);
    if(port.header.flags.isResponse && port.msg_type == Message::TYPE::PEER_INFORMATIONS)
    {
        std::istringstream iss(port.str);
        unsigned short network_port;
        std::string peerTag;
        _network.updatePeer(p, (bool)port.header.flags.isMasterNode, peerTag, network_port);
    }
}
void P2PServer::_requestPeerList(const HGOPeer &p)
{

}

void P2PServer::_processMessage(const Message & msg)
{
    std::cout<<"Message Receiverd : "<<msg<<"\n";
}

Message P2PServer::sendMessage(const HGOPeer &peer, const Message & msg)
{
    std::string data = _network.sendAndWait(peer, msg.data());
    return Message::fromByteArray(data);
}

void P2PServer::_p2phandler(const HGOPeer &peer, const HGONetworkManager::EVENT_TYPE & event, const std::string &data, HGONetworkManager * server)
{
    using TYPE = HGONetworkManager::EVENT_TYPE;
    switch(event)
    {
        case TYPE::NEW_INCOMING:
            std::cout<<"\033[32m[HGO P2P]\033[0m - Peer as joined the party : "<<peer<<"\n";
            _requestPeerInfos(peer);
        break;
        case TYPE::NEW_OUTGOING:
            std::cout<<"\033[33m[HGO P2P]\033[0m - Trying to join "<<peer<<"\n";
        break;
        case TYPE::SERVER_LAUNCHED:
            std::cout<<"\033[34m[HGO P2P]\033[0m - Server Launched on port : "<<peer.port<<"\n";
        break;
        case TYPE::SERVER_STOPPED:
            std::cout<<"\033[34m[HGO P2P]\033[0m - Server has been stopped\n";
        break;
        case TYPE::PEER_DISCONNECTED:
            std::cout<<"\033[31m[HGO P2P]\033[0m - eeer has been disconnected : "<<peer<<"\n";
        break;
        case TYPE::MESSAGE:
        break;
    }

}
