#include "wallet.h"
#include "../exceptions.h"

using namespace HGO::CHAIN;
using namespace HGO::TOKEN;
using namespace HGO::EXCEPTION;

Wallet::Wallet(BLOCKCHAIN & chain) 
: _balance(0.0), _chain(chain) {}

long double Wallet::getBalance() const
{
    return _balance;
}

bool Wallet::unlockWallet(const std::string & walletFile)
{
    std::ifstream wallet(walletFile, std::ios_base::in | std::ios_base::binary);
    if(!wallet)
        throw WalletError("Cannot open wallet file : " + walletFile);

    wallet >> _address;    
    _retrieveTransactions();
    return true;
}

bool Wallet::send(const std::string & walletAddress, long double amount)
{
    if(_balance < amount)
        return false;

    Transaction tx = buildTransaction(walletAddress, amount);
    if(_chain.requestTransaction(tx))
    {
        _transactions.push_back(tx);
        _balance -= amount;
        return true;
    }

    return false;
}

Transaction Wallet::buildTransaction(const std::string & walletAddress, long double amount) const
{
    Transaction tx;
    tx.from = _address;
    tx.to = walletAddress;
    tx.amount = amount;
    tx.time = std::time(nullptr);
    tx.token = TOKEN_NAME;
    return tx;
}

std::string Wallet::getAddress() const
{
    return _address;
}
std::string Wallet::getToken() const
{
    return TOKEN_NAME;
}

void Wallet::_retrieveTransactions()
{
    _balance = 0.0L;
    for( const Block & blk : _chain.getChain())
    {
        
        T_LIST txns = Blockchain::parseTransactions(blk.getData());
        if(txns.size()) {
            for(const Transaction & tx : txns)
            {
                if(tx.from == _address || tx.to == _address) {
                    Transaction::Direction tx_type = (tx.from == _address) ? Transaction::OUT : Transaction::IN;
                    if(tx_type == Transaction::IN)
                    {
                        _balance += tx.amount;
                    } else {
                        _balance -= tx.amount;
                    }
                    _transactions.push_back(tx);
                }
            }
        }
    }
}

Wallet::T_LIST Wallet::getTransactions() const
{
    return _transactions;
}

Wallet::~Wallet() {}



std::ostream &HGO::TOKEN::operator<<(std::ostream &o, const Wallet & wallet)
{
    o<<"========= Wallet : "<<wallet.getAddress()<<" ("
    <<std::setprecision(std::numeric_limits<long double>::digits10 + 1)<<wallet.getBalance()<<" "<<wallet.getToken()<<")\n";

    for(const Transaction &t : wallet.getTransactions())
    {
        o << t;
    }

    return o.flush();

}

