/**
 * HGO Protocol
 * This set of classes intend to manage the P2P network
 * It will be able to launch a server listening on a specific port and connect to peer at same time 
 * The structure is based on 2 threads one accepting new client and one polling on FileDescriptor event for incoming message
 * Outgoing / Incoming connection are stored in the same container PEER_LIST
 * 
 * Some events are defined and thrown through each callback binded to the Class via HGOProtocolManager::addCallback(EVENT_CALLBACK) method
 * 
 */ 
#ifndef __HGO_NETWORK__
#define __HGO_NETWORK__

#include <iostream>
#include <string>
#include <sstream>
#include <memory>
#include <functional>
#include <thread>
#include <mutex>
#include <vector>
#include <algorithm>
//Network
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <poll.h>

#include <errno.h>

#define __HGO_NETWORK__READ_BUFFER 1024

namespace HGO::NETWORK
{
    struct HGOPeer
    {
        std::string peer_tag; //For future implementation
        std::string ip_address;
        unsigned short port;
        bool isMasterNode; //for future implementation
        inline bool operator==(const HGOPeer &o) const {return (o.ip_address == ip_address && o.port == port);}
    };

    inline std::ostream &operator<<(std::ostream &o, const HGOPeer & peer) {
        if(!peer.peer_tag.empty())
            o<<"["<<peer.peer_tag<<"] ";
        
        return (o<<peer.ip_address<<":"<<peer.port).flush();
    }

    class HGONetworkManager
    {
            protected:
                using POLL_LIST = std::vector<pollfd>;
                constexpr static unsigned int MAX_PEERS = 150;
            
            public:
                using PEER_LIST = std::vector<HGOPeer>;
                enum EVENT_TYPE {
                    NEW_INCOMING,
                    NEW_OUTGOING,
                    PEER_DISCONNECTED,
                    MESSAGE,
                    MESSAGE_SENT,
                    SERVER_LAUNCHED,
                    SERVER_STOPPED
                };
                using EVENT_CALLBACK = std::function<void(const HGOPeer &peer, const EVENT_TYPE & event, const std::string &data, HGONetworkManager * server)>;
            protected:
                using CB_LIST = std::vector<EVENT_CALLBACK>;     

            public:
                constexpr static unsigned short DEFAULT_PORT = 2016;
                HGONetworkManager();
                ~HGONetworkManager();

                //Server Section
                bool run(const unsigned short &port = 2016);
                bool stop();
                unsigned short serverPort() const;

                //Peer Section
                bool connectToPeer(const std::string &ip_address, const unsigned short &port);
                bool disconnectPeer(const HGOPeer &peer);
                bool sendTo(const HGOPeer& peer, const std::string & data);
                bool broadcast(const std::string & data) const;
                std::string sendAndWait(const HGOPeer &peer, const std::string & data);
                bool updatePeer(const HGOPeer &peer, bool isMasternode, const std::string tagname = "", const unsigned short & port = 0);
                PEER_LIST getPeerList() const;

                //Callback section
                void addCallback(EVENT_CALLBACK cb);

            protected:
                bool _running;
                
                PEER_LIST _peers;
                POLL_LIST _fds;

                int _socket_server;
                unsigned short _port;

                std::thread _tHandler;
                std::thread _tAcceptor;

                int _getPeerIndex(const HGOPeer & peer) const;

                void _messageHandler(); //thread 1
                void _acceptNewConnection(); // thread 2

                void _removePeer(const std::size_t &index);
                void _emitEvent(const HGOPeer &peer, const EVENT_TYPE & event, const std::string &data = "") ;

                CB_LIST _callbacks;

                static std::mutex _mut;

    };
}


#endif