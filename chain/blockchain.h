#ifndef __HGO_BLOCKCHAIN__
#define __HGO_BLOCKCHAIN__
#include <iostream>
#include <fstream>
#include <vector>
#include <deque>
#include "../exceptions.h"
#include "block.h"
#include "../token/transaction.h"

namespace HGO::CHAIN
{

    class Blockchain
    {
        
        friend std::ostream &operator<<(std::ostream&, const Blockchain &);

        public:
            using BLOCK_LIST = std::vector<Block>;
            using TRANSACTION_BUFFER = std::deque<HGO::TOKEN::Transaction>;
            using T_LIST = std::vector<HGO::TOKEN::Transaction>;
            constexpr static TRANSACTION_BUFFER::size_type MIN_TX_PER_BLOCK = 2;
            
        public:
            Blockchain() = default;
            Blockchain(const BLOCK_LIST & blocks);
            void addBlock(Block blk);
            bool verify() const;
            Blockchain &operator<<(const Block & blk);
            const Block &getLastBlock() const;
            const BLOCK_LIST &getChain() const;
            void save(const std::string &path) const;
            
            bool requestTransaction(const HGO::TOKEN::Transaction &tx);
            
            static Blockchain load(const std::string &path);
            static T_LIST parseTransactions(const std::string &rawTx);

        protected:

            void _createTransactionBlock();

            BLOCK_LIST _chain;
            TRANSACTION_BUFFER _txBuffer;
            
    };

    std::ostream &operator<<(std::ostream & o, const Blockchain &bc);
}

#endif