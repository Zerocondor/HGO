#ifndef __HGO_P2PSERVER__
#define __HGO_P2PSERVER__
#include <string>
#include <deque>
#include <map>
#include "protocol.h"
#include "network_manager.h"

namespace HGO::NETWORK
{
    class P2PServer {

        public:
            using MESSAGE_CALLBACK = std::function<void(const HGOPeer & peer, const Message&)>;
            using TICK_MSG_BUFFER = std::deque<std::pair<std::shared_ptr<HGOPeer>, Message>>;
        public:
            P2PServer(const std::string &tagName = "", bool masterNode = false);
            ~P2PServer();

            void startNetwork(const unsigned short &port);
            void stopNetwork();

            void setMasterNode(bool value);
            void setTagName(const std::string &value);

            bool connectToNode(const std::string &ip_address, const unsigned short &port);
            Message requestMessage(const HGOPeer &peer, const Message & msg);
            void sendMessage(const HGOPeer &peer, const Message & msg);
            void broadcast(const Message &msg);
            void setBlockchainHandlers(MESSAGE_CALLBACK messages);

            void pushTickMessage(std::shared_ptr<HGOPeer>peer, Message msg);

            HGONetworkManager &getNetworkManager();
            std::shared_ptr<HGOPeer> getRandomMasterNode();

        protected :
            void _p2phandler(const HGOPeer &peer, const HGONetworkManager::EVENT_TYPE & event, const std::string &data, HGONetworkManager * server);
            void _processMessage(const HGOPeer &p, const Message &msg);

            bool _requestPeerInfos(HGOPeer &p);
            bool _requestPeerList(const HGOPeer &p);
            bool _sendAcquitment(const HGOPeer & p);
            void _tick();
            std::thread _tTick;

            HGONetworkManager _network;
            std::string _tagName;
            bool _isMasterNode;

            MESSAGE_CALLBACK _messagesHandler;
            TICK_MSG_BUFFER _buffer;
            static std::mutex _lockTick;

    };
}

#endif