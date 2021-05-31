#ifndef __HGO__
#define __HGO__

/**
 * HGO Cryptocurrency
 * Custom blockchain to handle HGO Token
 * Based on flat file, easy to use
 * 
 * @todo Add consensus to choose a Masternode for transaction
 * @todo Add a fees system to reward validator node
 * @todo Implement inflate/deflate blockchain file or directly block data to improve data transfer
 * @author Nicolas BASTIDE <nicolas.bastide1@gmail.com>
 */ 

#define HGO_VERSION_MAJOR 1
#define HGO_VERSION_MINOR 0
#include "various/banner.h"

#include "exceptions.h"
#include "chain/block.h"
#include "chain/blockchain.h"
#include "chain/chainevent.h"

#include "token/wallet.h"
#include "token/transaction.h"
#include "token/hgo_key.h"

#include "hgo_protocol/network_manager.h"
#include "hgo_protocol/protocol.h"
#include "hgo_protocol/p2p_server.h"

#endif