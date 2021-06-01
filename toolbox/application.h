#ifndef __APP_TOOL__
#define __APP_TOOL__
#include <HGO>
#include "sha256/sha256.h"

namespace HGO::APP
{

    class Toolbox
    {
            using ARGUMENTS = std::map<std::string, std::string>;
        public:
            Toolbox(int argc, char **argv);
            ~Toolbox();

            int exec();
        protected:

            void _initBlockchain(const std::string &filename, const std::string & genesis_block_data, const std::string & addr = "", const long double & amout = 0.0L) const;
            void _readWallet(const std::string &filename, const std::string & chainFileName="") const;
            void _exploreChain(const std::string &filename, const HGO::CHAIN::Block::BLOCK_INDEX & idx) const;
            void _exploreChain(const std::string &filename) const;

            void _parseArguments(int argc, char ** argv);
            bool _hasOption(const std::string &option);

            void _printHelp();

            ARGUMENTS _list;
    };
}

#endif