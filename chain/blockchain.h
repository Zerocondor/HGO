#ifndef __HGO_BLOCKCHAIN__
#define __HGO_BLOCKCHAIN__
#include <iostream>
#include <fstream>
#include <vector>
#include <deque>
#include "../exceptions.h"
#include "../token/transaction.h"
#include "chainevent.h"
#include "block.h"

#ifndef __HGO_BLOCKCHAIN__MINTXBLOCK
#define __HGO_BLOCKCHAIN__MINTXBLOCK 15
#endif

namespace HGO::CHAIN
{

    class Blockchain
    {
        
        friend std::ostream &operator<<(std::ostream&, const Blockchain &);

        public:
            using BLOCK_LIST = std::vector<Block>;
            using TRANSACTION_BUFFER = std::deque<HGO::TOKEN::Transaction>;
            using T_LIST = std::vector<HGO::TOKEN::Transaction>;
            constexpr static TRANSACTION_BUFFER::size_type MIN_TX_PER_BLOCK = __HGO_BLOCKCHAIN__MINTXBLOCK;
            
        public:
            Blockchain();
            Blockchain(const BLOCK_LIST & blocks);
            Blockchain(const Blockchain &) = delete;
            Blockchain(Blockchain && other);
            Blockchain &operator=(Blockchain && other);
            Blockchain &operator=(const Blockchain & other) = delete;
            void addBlock(Block blk);
            void createBlock(Block blk);
            bool verify() const;
            EVENTS::ChainEventManager &eventManager();
            Blockchain &operator<<(const Block & blk);
            const Block &getLastBlock() const;
            const Block::BLOCK_INDEX getLastBlockID() const; 
            const BLOCK_LIST &getChain() const;
            void save(const std::string &path);
            
            bool requestTransaction(const HGO::TOKEN::Transaction &tx, bool forceBlock = false);
            
            static Blockchain load(const std::string &path);
            static T_LIST parseTransactions(const std::string &rawTx);

        protected:

            void _createTransactionBlock(bool forceBlock);

            BLOCK_LIST _chain;
            TRANSACTION_BUFFER _txBuffer;
            EVENTS::ChainEventManager _events;
            
    };

    std::ostream &operator<<(std::ostream & o, const Blockchain &bc);
}

#endif