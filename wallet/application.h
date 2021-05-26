#ifndef __APP_WALLET__
#define __APP_WALLET__
#include <HGO>
#include <string>
#include <map>
#include <algorithm>
#include <sstream>
#include <functional>

namespace HGO::APP
{

    class Wallet
    {
            constexpr static char const * WALLET_NODE_FOOTPRINT = "HGO Wallet version 1.0";
            using ARGUMENTS = std::map<std::string, std::string>;
        public:
            Wallet(int argc, char **argv);
            ~Wallet();

            int exec();
        protected:

            void _checkSynchState(const HGO::NETWORK::HGOPeer & peer);
            void _checkBlockID(const HGO::NETWORK::HGOPeer & peer, const HGO::NETWORK::Message &msg);
            void _processRequestBlock(const HGO::NETWORK::HGOPeer & peer, const HGO::NETWORK::Message &msg);
            void _sendBlocks(const HGO::NETWORK::HGOPeer & peer, const HGO::CHAIN::Block::BLOCK_INDEX &fromBlock);

            void _parseArguments(int argc, char ** argv);
            bool _hasOption(const std::string &option);

            bool _handleChainEvent(const HGO::CHAIN::EVENTS::ChainEvent &ev);
            void _handleP2PEvent(const HGO::NETWORK::HGOPeer &peer, const HGO::NETWORK::Message &msg);
            ARGUMENTS _list;
            HGO::CHAIN::Blockchain _chain;
            HGO::NETWORK::P2PServer _network;
            HGO::TOKEN::Wallet * _wallet;

    };
}

#endif