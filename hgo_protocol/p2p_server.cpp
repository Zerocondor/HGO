#include "p2p_server.h"
#include "../exceptions.h"

using namespace HGO::NETWORK;
using namespace HGO::EXCEPTION;

std::mutex P2PServer::_lockTick;

P2PServer::P2PServer(const std::string & tagName, bool masternode)
:_tagName(tagName), _isMasterNode(masternode)
{
    _messagesHandler = nullptr;
}

P2PServer::~P2PServer()
{
    stopNetwork();
}

void P2PServer::setMasterNode(bool value)
{
    _isMasterNode = value;
}
void P2PServer::setTagName(const std::string &value)
{
    _tagName = value;
}

void P2PServer::startNetwork(const unsigned short &port)
{
    using namespace std::placeholders;
    HGONetworkManager::EVENT_CALLBACK bindedCallback = std::bind(&P2PServer::_p2phandler, this, _1, _2, _3, _4);
    _network.addCallback(bindedCallback);
    _network.run(port);

    _tTick = std::thread(&P2PServer::_tick, this);
}

void P2PServer::stopNetwork()
{
    _network.stop();
    if(_tTick.joinable())
    {
        _tTick.join();
    }
}

void P2PServer::_tick()
{
    while(_network.isRunning())
    {
        if(_lockTick.try_lock()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
            if(_buffer.size())
            {
                auto evt = _buffer.back();
                if(evt.first == nullptr)
                {
                    broadcast(evt.second);
                } else {
                    sendMessage(*evt.first.get(), evt.second);
                }
                _buffer.pop_back();
            }
            _lockTick.unlock();
        }
    }
}

void P2PServer::pushTickMessage(std::shared_ptr<HGOPeer>peer, Message msg)
{
    _buffer.push_front({peer, msg});
}

bool P2PServer::connectToNode(const std::string &ip_address, const unsigned short &port)
{
    return _network.connectToPeer(ip_address, port);
}

bool P2PServer::_requestPeerInfos(HGOPeer &p)
{
    std::cout<<"\t Ask for peer information\n";
    Message portMessage;
    portMessage.header.config.flags.isRequest = true;
    portMessage.header.config.flags.isResponse = false;
    portMessage.header.config.flags.isMasterNode = _isMasterNode;
    portMessage.header.config.flags.isOfficialNode =  true;
    portMessage.header.config.flags.isTest = true;
    portMessage.header.config.flags.isForward = false;

    portMessage.msg_type = Message::TYPE::PEER_INFORMATIONS;
    portMessage.msg_size = _tagName.size();
    portMessage.str = _tagName;

    Message port = requestMessage(p, portMessage);
    if(port.header.config.flags.isResponse && port.msg_type == Message::TYPE::PEER_INFORMATIONS)
    {
        std::istringstream iss(port.str);
        unsigned short network_port;
        std::string peerTag;
        iss>>network_port>>peerTag;
        if(_network.updatePeer(p, (bool)port.header.config.flags.isMasterNode, peerTag, network_port))
        {
            p.isMasterNode = (bool)port.header.config.flags.isMasterNode;
            p.peer_tag = peerTag;
            p.port = network_port;
            std::cout<<"\t Peer Has been updated "<<p<<"\n";
            return true;
        } else {
            return false;
        }
        
    } else {
        _network.disconnectPeer(p);
        return false;
    }
}
bool P2PServer::_requestPeerList(const HGOPeer &p)
{
    Message peerListRequest;
    peerListRequest.header.config.flags.isRequest = true;
    peerListRequest.header.config.flags.isResponse = false;
    peerListRequest.header.config.flags.isMasterNode = _isMasterNode;
    peerListRequest.header.config.flags.isOfficialNode =  true;
    peerListRequest.header.config.flags.isTest = true;
    peerListRequest.header.config.flags.isForward = false;

    peerListRequest.msg_type = Message::TYPE::PEER_LIST;
    peerListRequest.msg_size = 0;
    peerListRequest.str = "";

    Message response = requestMessage(p, peerListRequest);
    if(response.header.config.flags.isResponse && response.msg_type == Message::TYPE::PEER_LIST)
    {
        if(response.msg_size == 0)
        {
            std::cout<<"No peer available........\n";
            _sendAcquitment(p);
            return true;
        }
        std::istringstream iss(response.str);
        std::string peerStr;
        while( iss >> peerStr)
        {
            std::string::size_type posSemicolon = peerStr.find_first_of(':',0);
            if(posSemicolon == std::string::npos) {
                continue;
            }
            peerStr.replace(posSemicolon,1, " ");

            std::istringstream peerIn(peerStr);
            unsigned short port;
            std::string ip_address;
            if(peerIn>>ip_address>>port)
            {
                if(!connectToNode(ip_address, port))
                {
                    std::cout<<"\t Peer connection issue, we are maybe already connected to it ! \n";
                }
            }
        }
        _sendAcquitment(p);
        return true;
    }
    return false;
}

bool P2PServer::_sendAcquitment(const HGOPeer & peer)
{
    Message msg;
    msg.header.config.flags.isRequest = true;
    msg.header.config.flags.isResponse = false;
    msg.header.config.flags.isMasterNode = _isMasterNode;
    msg.header.config.flags.isOfficialNode =  true;
    msg.header.config.flags.isTest = true;
    msg.header.config.flags.isForward = false;
    msg.msg_type = Message::TYPE::ACQUITED;
    msg.msg_size = 0;
    msg.str = "";
    sendMessage(peer, msg);
    return true;
}

void P2PServer::_processMessage(const HGOPeer &p, const Message & msg)
{
    if(msg.header.config.flags.isResponse) {
        if(msg.msg_type == Message::TYPE::PEER_INFORMATIONS || msg.msg_type == Message::TYPE::PEER_LIST) {
            return;
        }
    }

    Message response;
    response.header.config.flags.isRequest = false;
    response.header.config.flags.isResponse = true;
    response.header.config.flags.isMasterNode = _isMasterNode;
    response.header.config.flags.isOfficialNode =  true;
    response.header.config.flags.isTest = true;
    response.header.config.flags.isForward = false;
    
    std::string str = "";
    HGONetworkManager::PEER_LIST pList;
    HGOPeer other = p;
    switch(msg.msg_type)
    {
        case Message::TYPE::NONE:
            std::cout<<"NONE MESSAGE : "<<msg.str<<"\n";
        break;
        case Message::TYPE::PEER_INFORMATIONS:
            _network.updatePeer(p, (bool)msg.header.config.flags.isMasterNode, msg.str, p.port);
            str = std::to_string(_network.serverPort()) + " " + _tagName;
            response.str = str;
            response.msg_size = str.size();
            response.msg_type = msg.msg_type;
            sendMessage(p, response);       
            _requestPeerList(p);

        break;
        case Message::TYPE::PEER_LIST:

            pList = _network.getPeerList();
            for(const HGOPeer& peerToSend : pList)
            {
                if(peerToSend == p)
                    continue;

                str+= peerToSend.ip_address + ":" + std::to_string(peerToSend.port) + " ";
            }
            response.str = str;
            response.msg_size = str.size();
            response.msg_type = msg.msg_type;
            sendMessage(p, response);
        break;
        case Message::TYPE::NEW_BLOCK:
        case Message::TYPE::NEW_TRANSACTION:
        case Message::TYPE::REQUEST_BLOCK:
        case Message::TYPE::SYNCHRONIZE:
        case Message::TYPE::MESSAGE:
        case Message::TYPE::ACQUITED:
            if(_messagesHandler)
            {
                _messagesHandler(p, msg);
            }
        break;
        default:
        break;
    }
}

Message P2PServer::requestMessage(const HGOPeer &peer, const Message & msg)
{
    std::string data = _network.sendAndWait(peer, msg.data());
    Message resulting;
    resulting.msg_type = Message::TYPE::NONE;

    try{
        resulting = Message::fromByteArray(data);
    }catch(P2PError &e) {
        std::cout<<"Data : "<<data<<"\n"<<"\terror: "<<e.what()<<"\n";
    }
    return resulting;
}

void P2PServer::sendMessage(const HGOPeer &peer, const Message & msg)
{
    _network.sendTo(peer, msg.data());
}

void P2PServer::broadcast(const Message &msg)
{
    _network.broadcast(msg.data());
}

void P2PServer::setBlockchainHandlers(MESSAGE_CALLBACK message_handler)
{
     _messagesHandler = message_handler;
}

void P2PServer::_p2phandler(const HGOPeer &peer, const HGONetworkManager::EVENT_TYPE & event, const std::string &data, HGONetworkManager * server)
{

    using TYPE = HGONetworkManager::EVENT_TYPE;
    HGOPeer new_peer = peer;
    switch(event)
    {
        case TYPE::NEW_INCOMING:
            std::cout<<"\033[32m[HGO P2P]\033[0m - Peer as joined the party : "<<peer<<"\n";           
            _requestPeerInfos(new_peer);
        break;
        case TYPE::NEW_OUTGOING:
            std::cout<<"\033[33m[HGO P2P]\033[0m - Trying to join "<<peer<<"\n";
        break;
        case TYPE::SERVER_LAUNCHED:
            std::cout<<"\033[34m[HGO P2P]\033[0m - Server Launched on port : "<<peer.port;
            if(_isMasterNode)
                std::cout<<" as MasterNode ";
            std::cout<<"\n";    
        break;
        case TYPE::SERVER_STOPPED:
            std::cout<<"\033[34m[HGO P2P]\033[0m - Server has been stopped\n";
        break;
        case TYPE::PEER_DISCONNECTED:
            std::cout<<"\033[31m[HGO P2P]\033[0m - Peer has been disconnected : "<<peer<<"\n";
        break;
        case TYPE::MESSAGE_SENT:
            std::cout<<"\033[32m[HGO P2P]\033[0m - Message sent to : "<<peer<<"\n";
            
        break;
        case TYPE::MESSAGE:
            try {
                
                _processMessage(peer, Message::fromByteArray(data));
            } catch (P2PError & e) {
                std::cout<<"\033[31m[HGO P2P]\033[0m - Unable to process message received from : "<<peer<<"\n";
            }     
        break;
    }
    std::cout.flush();

}
