#include "p2p.h"
#include "../exceptions.h"

using namespace HGO::P2P;
using namespace HGO::EXCEPTION;

std::mutex HGOProtocolManager::_mut;

HGOProtocolManager::HGOProtocolManager() 
: _running(false), _socket_server(-1), _port(DEFAULT_PORT)
{}

HGOProtocolManager::~HGOProtocolManager()
{
    stop();
}
bool HGOProtocolManager::stop()
{
    if(_running) {
        _running = false;
        if(_tHandler.joinable())
            _tHandler.join();
        if(_tAcceptor.joinable())
            _tAcceptor.join();

        for(auto & fd : _fds) {
            std::cout<<"Closing peer\n";
            close(fd.fd);
        }

        _fds.clear();
        _peers.clear();
        close(_socket_server);
        std::cout<<"Closing Server\n";
    }
    return !_running;
};
bool HGOProtocolManager::run(const unsigned short &port)
{
    _socket_server = socket(AF_INET, SOCK_STREAM, 0);
    int opt = 1;
    setsockopt(_socket_server, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
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

    std::cout<<"Server is running and listening on port : "<<port<<"\n";
    
    _running = true;

    _tAcceptor = std::thread(std::function<void(HGOProtocolManager*)>(&HGOProtocolManager::_acceptNewConnection), this);
    _tHandler = std::thread(std::function<void(HGOProtocolManager*)>(&HGOProtocolManager::_messageHandler), this);

    return true;
}

bool HGOProtocolManager::connectToPeer(const std::string &ip_address, const unsigned short &port)
{



    return true;
}

void HGOProtocolManager::_acceptNewConnection()
{
    while(_running) {
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

        _mut.lock();
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
        std::cout<<"New Peer : "<<peer.ip_address<<"\n";
        _mut.unlock();
    }

}
void HGOProtocolManager::_messageHandler()
{
    while(_running)
    {
        _mut.lock();
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
                        char buffer[1024]{0};
                        
                        result = read(_fds[i].fd, buffer, 1024);                 
                       
                        if(result == -1) {
                            std::cout<<"Error while reading : "<<strerror(errno)<<"\n";
                            continue;
                        } else if (result > 0) {
                             std::cout<<"Peer ["<<current_peer.ip_address<<"] : " <<buffer;
                        }

                    }
                }
            }
        }
        _mut.unlock();
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
}

void HGOProtocolManager::_removePeer(const std::size_t &index)
{
    if(index < _fds.size()) {
        HGOPeer peer = _peers[index];
        std::cout<<"Peer : "<<peer.ip_address<< " has been removed \n";
        close(_fds[index].fd);

        _peers.erase(_peers.begin() + index);
        _fds.erase(_fds.begin() + index);
    }
}

bool HGOProtocolManager::sendTo(const HGOPeer& peer, const std::string & data)
{
    return true;
}
bool HGOProtocolManager::broadcast(const std::string & data)
{
    return true;
}