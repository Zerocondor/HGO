#ifndef __APP_MASTERNODE__
#define __APP_MASTERNODE__
#include <HGO>
#include <string>
#include <map>
#include <algorithm>
#include <sstream>
#include <functional>

namespace HGO::APP
{

    class MasterNode
    {
            using ARGUMENTS = std::map<std::string, std::string>;
        public:
            MasterNode(int argc, char **argv);
            ~MasterNode();

            int exec();
        protected:

            void _parseArguments(int argc, char ** argv);
            bool _hasOption(const std::string &option);

            bool _handleChainEvent(const HGO::CHAIN::EVENTS::ChainEvent &ev);
            void _handleP2PEvent(const HGO::NETWORK::Message &msg);
            ARGUMENTS _list;
            HGO::CHAIN::Blockchain _chain;
            HGO::NETWORK::P2PServer _network;

    };
}

#endif