#ifndef __HGO_P2P__
#define __HGO_P2P__
#include <string>
#include <sstream>
#include <memory>
#include <functional>
#include <thread>
#include <mutex>
#include <vector>
//Network
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <poll.h>


#include <errno.h>

namespace HGO::P2P
{
    struct HGOPeer
    {
        std::string peer_tag;
        std::string ip_address;
        unsigned short port;
        bool isMasterNode;
    };

    class HGOProtocolManager
    {
            using PEER_LIST = std::vector<HGOPeer>;
            using POLL_LIST = std::vector<pollfd>;
            
            constexpr static unsigned int MAX_PEERS = 10;
            
            public:
                enum EVENT_TYPE {
                    NEW_CLIENT,
                    DATA_AVAILABLE
                };
                using EVENT_CALLBACK = std::function<void(const HGOPeer &sender, const EVENT_TYPE & event, const std::string &data)>;
            protected:
                using CB_LIST = std::vector<EVENT_CALLBACK>;     

            public:
                constexpr static unsigned short DEFAULT_PORT = 2016;
                HGOProtocolManager();
                ~HGOProtocolManager();

                bool run(const unsigned short &port = 2016);
                bool stop();
                bool connectToPeer(const std::string &ip_address, const unsigned short &port);
                
                bool sendTo(const HGOPeer& peer, const std::string & data);
                bool broadcast(const std::string & data);

            protected:
                bool _running;
                PEER_LIST _peers;
                POLL_LIST _fds;

                int _socket_server;
                unsigned short _port;

                std::thread _tHandler;
                std::thread _tAcceptor;

                void _messageHandler();
                void _acceptNewConnection();
                void _removePeer(const std::size_t &index);

                static std::mutex _mut;

    };
}


#endif