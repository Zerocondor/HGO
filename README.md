# HGO Blockchain Basics

A simple implementation of blockchain

## Available features
* Blockchain Management
* Wallet Managemement
* P2P Network
* Transaction and Wallet signature

## Available binaries
* MasterNode - An validator node allowed to build block from transactions
* Wallet - An wallet application allowed to read wallet file, retrieve tx and send amount
* Toolbox - An application to manage blockchain and wallet (Create new blockchain, create wallet, explore chain)

# Build
## Dependencies
The only dependency is Openssl for cryptographic signature and an (included) sha256 builder

## Build commands
To build the 3 tools (Masternode, Wallet and Toolbox)

`make bin`

Each of thoses tools can be builded individually by typing 

`make [masternode|wallet|toolbox]`

# Usage
Each application can print their help using command `:$ [application] --help`
