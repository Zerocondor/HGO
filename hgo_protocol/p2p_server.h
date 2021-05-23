#ifndef __HGO_P2PSERVER__
#define __HGO_P2PSERVER__

#include "protocol.h"
#include "network_manager.h"

namespace HGO::NETWORK
{
    class P2PServer {

        public:
            P2PServer();
            ~P2PServer();

            void startNetwork(const unsigned short &port);
            void stopNetwork();

            void connectToNode(const std::string &ip_address, const unsigned short &port);
            Message sendMessage(const HGOPeer &peer, const Message & msg);

        protected :
            void _p2phandler(const HGOPeer &peer, const HGONetworkManager::EVENT_TYPE & event, const std::string &data, HGONetworkManager * server);
            void _processMessage(const Message &msg);

            void _requestPeerInfos(const HGOPeer &p);
            void _requestPeerList(const HGOPeer &p);

            HGONetworkManager _network;

    };
}

#endif