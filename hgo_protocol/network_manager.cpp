#include "network_manager.h"
#include "../exceptions.h"

using namespace HGO::NETWORK;
using namespace HGO::EXCEPTION;

std::mutex HGONetworkManager::_mut;

HGONetworkManager::HGONetworkManager() 
: _running(false), _socket_server(-1), _port(DEFAULT_PORT)
{
    std::cout<<"MAX PEER ACCEPTED : "<<MAX_PEERS<<"\n";
}

HGONetworkManager::~HGONetworkManager()
{
    stop();
}

bool HGONetworkManager::stop()
{
    if(_running) {
        _running = false;
        if(_tHandler.joinable())
            _tHandler.join();
        if(_tAcceptor.joinable())
            _tAcceptor.join();

        for(auto & fd : _fds) {
            close(fd.fd);
        }

        _fds.clear();
        _peers.clear();
        close(_socket_server);
        _emitEvent(HGOPeer(), EVENT_TYPE::SERVER_STOPPED);
    }
    return !_running;
}

unsigned short HGONetworkManager::serverPort() const
{
    return _port;
}

bool HGONetworkManager::isRunning() const
{
    return _running;
}

bool HGONetworkManager::run(const unsigned short &port)
{
    _port = port;
    _socket_server = socket(AF_INET, SOCK_STREAM, 0);
    int opt = 1;

    setsockopt(_socket_server, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)); //enable reusable socket

    if(_socket_server < 0) 
        throw ProtocolError(errno);

    //Formating port in network way
    unsigned char rp[2];
    rp[1] = port & 0xFF;
    rp[0] = (port >> 8) & 0xFF;
    unsigned short real_port = *reinterpret_cast<unsigned short *>(&rp);
   
    sockaddr_in server_infos;
    server_infos.sin_addr.s_addr = INADDR_ANY;
    server_infos.sin_family = AF_INET;
    server_infos.sin_port = real_port;

    int result = bind(_socket_server,reinterpret_cast<sockaddr *>(&server_infos), sizeof(server_infos));
    if(result < 0) {
        close(_socket_server);
        throw ProtocolError(errno);
    }

    result = listen(_socket_server, MAX_PEERS);
    if(result < 0) {
        close(_socket_server);
        throw ProtocolError(errno);
    }
    
    _running = true;

    _tAcceptor = std::thread(std::function<void(HGONetworkManager*)>(&HGONetworkManager::_acceptNewConnection), this);
    _tHandler = std::thread(std::function<void(HGONetworkManager*)>(&HGONetworkManager::_messageHandler), this);

    //Use peer only to send port in event
    HGOPeer server;
    server.port = port;
    _emitEvent(server, EVENT_TYPE::SERVER_LAUNCHED);

    return true;
}

bool HGONetworkManager::connectToPeer(const std::string &ip_address, const unsigned short &port)
{
    if(_peers.size() == MAX_PEERS) {
            return false;
    }
    HGOPeer peer;
    peer.ip_address = ip_address;
    peer.port = port;
    
    if(_getPeerIndex(peer) != -1) {
        return false;
    }
 
    //Format IP
    std::istringstream iss(ip_address);
    int a,b,c,d;
    (((iss>>a).ignore() >> b).ignore() >> c).ignore() >> d;  
    unsigned char ip_split[4];
    ip_split[3] = d;
    ip_split[2] = c;
    ip_split[1] = b;
    ip_split[0] = a;
    unsigned int ip_address_network = *reinterpret_cast<unsigned int*>(&ip_split);

    //Format port
    unsigned char rp[2];
    rp[1] = port & 0xFF;
    rp[0] = (port >> 8) & 0xFF;
    unsigned short port_network = *reinterpret_cast<unsigned short*>(&rp);

    sockaddr_in client;
    client.sin_addr.s_addr = ip_address_network;
    client.sin_port = port_network;
    client.sin_family = AF_INET;

    int SOCK_CLIENT = socket(AF_INET, SOCK_STREAM, 0);
    if(SOCK_CLIENT == -1) {
        std::cout<<"Unable to connect to peer : "<<ip_address<<":"<<port<<"\n";
        std::cout<<"\t"<<strerror(errno)<<"\n";
        return false;
    }

    int result = connect(SOCK_CLIENT, reinterpret_cast<sockaddr *>(&client), sizeof(client));
    if(result == -1) {
        std::cout<<"Unable to connect to peer : "<<ip_address<<":"<<port<<"\n";
        std::cout<<"\t"<<strerror(errno)<<"\n";
        return false;
    }

    pollfd fd;
    fd.fd = SOCK_CLIENT;
    fd.events = POLLIN | POLLRDHUP | POLLERR;
    fd.revents = 0;

    _fds.push_back(fd);
    _peers.push_back(peer);

    _emitEvent(peer, EVENT_TYPE::NEW_OUTGOING);

    return true;
}

void HGONetworkManager::_acceptNewConnection()
{
    while(_running) {
        if(_peers.size() == MAX_PEERS) {
            continue;
        }
        sockaddr_in client_infos;
        socklen_t sz = sizeof(client_infos);

        pollfd ss[1];
        ss[0].fd = _socket_server;
        ss[0].events = POLLIN;
        ss[0].revents = 0;

        int result = poll(ss, 1, 0);
        if(result != 1)
            continue;

        int socket_client = accept4(_socket_server,reinterpret_cast<sockaddr *>(&client_infos), &sz, SOCK_NONBLOCK);
        if(socket_client < 0) {
            std::cout<<"Unable to accept this client\n";
        }

        
        pollfd fd_client;
        fd_client.fd = socket_client;
        fd_client.events = POLLIN | POLLRDHUP | POLLERR;
        fd_client.revents = 0;

        _fds.push_back(fd_client);

        unsigned char ip_address[4];
        ip_address[0] = client_infos.sin_addr.s_addr & 0xFF;
        ip_address[1] = (client_infos.sin_addr.s_addr >> 8) & 0xFF;
        ip_address[2] = (client_infos.sin_addr.s_addr >> 16)& 0xFF;
        ip_address[3] = (client_infos.sin_addr.s_addr >> 24) & 0xFF;
        std::ostringstream oss;
        oss<<(int)ip_address[0]<<"."<<(int)ip_address[1]<<"."<<(int)ip_address[2]<<"."<<(int)ip_address[3];

        HGOPeer peer;
        peer.ip_address = oss.str();
        _peers.push_back(peer);
        
        _emitEvent(peer, EVENT_TYPE::NEW_INCOMING);
    }
}

void HGONetworkManager::_messageHandler()
{
    while(_running)
    {       
        if(_fds.size())
        {
            int result = poll(_fds.data(), _fds.size(), 0);
            
            if(result < 0) {
                std::cout<<"Erreur poll : "<<strerror(errno)<<"\n";
                continue;
            } else if(result > 0) {
                for(POLL_LIST::size_type i = 0 ; i < _fds.size(); ++i){
                    HGOPeer current_peer = _peers[i];

                    if(_fds[i].revents & POLLRDHUP) {
                        _removePeer(i);
                        continue;
                    }
                    if(_fds[i].revents & POLLIN) {
                        char buffer[__HGO_NETWORK__READ_BUFFER]{0};
                        std::string returnData;
                        int dataReaden = -1;
                        
                        if((dataReaden = recv(_fds[i].fd, buffer, __HGO_NETWORK__READ_BUFFER,0)) > 0) {
                            returnData += std::string(buffer, dataReaden);
                        }
                        if(result == -1 && errno != EAGAIN) {
                            std::cout<<"Error while reading : "<<strerror(errno)<<"\n";
                            continue;
                        } else {
                             _emitEvent(current_peer, EVENT_TYPE::MESSAGE, returnData);
                        }

                    }
                }
            }
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }
}

void HGONetworkManager::_removePeer(const std::size_t &index)
{
    if(index < _fds.size()) {
        HGOPeer peer = _peers[index];
        close(_fds[index].fd);
        _peers.erase(_peers.begin() + index);
        _fds.erase(_fds.begin() + index);

        _emitEvent(peer, EVENT_TYPE::PEER_DISCONNECTED);
    }
}

bool HGONetworkManager::disconnectPeer(const HGOPeer & peer)
{
    int idx = _getPeerIndex(peer);
    if(idx != -1)
    {
        _removePeer(idx);
        return true;
    }
    return false;
}

bool HGONetworkManager::sendTo(const HGOPeer& peer, const std::string & data) 
{
    
    int idx = _getPeerIndex(peer);
    int result = -1;
    if(idx > -1) {
        result = write(_fds[idx].fd, data.c_str(), data.size());
        if(result == -1)
        {
            std::cout<<strerror(errno)<<"\n";
        } else {
            _emitEvent(peer, EVENT_TYPE::MESSAGE_SENT, data);
        }
    }
    
    return (result != -1);
}

bool HGONetworkManager::broadcast(const std::string & data) 
{
    bool everybodyReached = true;
    for(int i =0; i < _fds.size(); i++)
    {
        pollfd peer = _fds[i];
        int result = write(peer.fd, data.c_str(), data.size());
        if(result == -1) {
            everybodyReached = false;
        } else {
            _emitEvent(_peers[i], EVENT_TYPE::MESSAGE_SENT, data);
        }
    }

    return everybodyReached;
}

std::string HGONetworkManager::sendAndWait(const HGOPeer &peer, const std::string & data)
{
    pollfd fd[1];
    int idx = _getPeerIndex(peer);   
    fd[0] = _fds[idx];
    _fds[idx].events = 0;
    sendTo(peer, data);
    
    while((poll(fd, 1, 0) < 1) && (!(fd[0].revents & POLLIN) || !(fd[0].revents & POLLRDHUP)))
    ;;
    
    
    if(fd[0].revents & POLLRDHUP)
    {
        return "";
    }
    
    char buffer[__HGO_NETWORK__READ_BUFFER]{0};
    int result = -1;
    std::string return_str;
    if((result = read(fd[0].fd, buffer, __HGO_NETWORK__READ_BUFFER)) > 0){
        return_str += std::string(buffer, result);
    }
    
    _fds[idx].events = POLLIN | POLLRDHUP;
    
    _emitEvent(peer, EVENT_TYPE::MESSAGE, return_str);   
    return return_str;
}

bool HGONetworkManager::updatePeer(const HGOPeer &peer, bool isMasternode, const std::string tagname, const unsigned short & port)
{
    int idx = _getPeerIndex(peer);
    if(idx == -1)
        return false;
    
    _peers[idx].isMasterNode = isMasternode;
    _peers[idx].peer_tag = tagname;
    _peers[idx].port = port;
    return true;
}



int HGONetworkManager::_getPeerIndex(const HGOPeer & peer) const
{
    if(_peers.empty())
        return -1;

    PEER_LIST::const_iterator it = std::find_if(_peers.cbegin(), _peers.cend(), [&peer](const HGOPeer &p)->bool{
        return (peer == p);
    });

    if(it != _peers.cend())
    {
        return (it - _peers.cbegin());
    } else {
        return -1;
    }
}

HGONetworkManager::PEER_LIST HGONetworkManager::getPeerList() const
{
    return _peers;
}

void HGONetworkManager::addCallback(EVENT_CALLBACK cb)
{
    
    _callbacks.push_back(cb);
    
}

void HGONetworkManager::_emitEvent(const HGOPeer &peer, const EVENT_TYPE & event, const std::string &data) 
{
    for(auto _cb : _callbacks)
    {
        _cb(peer, event, data, this);
    }
}