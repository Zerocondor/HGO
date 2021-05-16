#ifndef __HGO__
#define __HGO__

/**
 * HGO Cryptocurrency
 * Custom blockchain to handle HGO Token
 * Based on flat file, easy to use
 * 
 * @todo Implement P2P network
 * @todo Implement Wallet unlock with RSA key to add security
 * @todo Implement inflate/deflate blockchain file
 * @author Nicolas BASTIDE <nicolas.bastide1@gmail.com
 */ 

#define HGO_VERSION_MAJOR 1
#define HGO_VERSION_MINOR 0

#include "exceptions.h"
#include "chain/block.h"
#include "chain/blockchain.h"

#include "token/wallet.h"
#include "token/transaction.h"

#endif