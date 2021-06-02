#include "wallet.h"
#include "../exceptions.h"

using namespace HGO::CRYPTO;
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
    
    std::getline(wallet, _address);

    std::string priv;
    std::string pub;

    std::string buffer;
    bool privLoaded = false;
    while(std::getline(wallet, buffer))
    {
        if(buffer == "") {
            privLoaded = true;
            continue;
        }
        
        if(privLoaded)
        {
            pub += buffer + "\n";
        } else {
            priv += buffer + "\n";
        }
    }

    KeyPair k;
    if(!pub.empty()) {
        pub.erase(pub.size() - 1, 1);
        KeyPair::loadFromPEM(k, pub, KeyPair::KEY_TYPE::PUBLIC);
    }
    if(!priv.empty()) {
        priv.erase(priv.size() - 1, 1);
        KeyPair::loadFromPEM(k, priv, KeyPair::KEY_TYPE::PRIVATE);
    }
    
    _key = k;
    _retrieveTransactions();
    return true;
}

KeyPair Wallet::getKeys() const
{
    return _key;
}
void Wallet::signTransaction(Transaction & tx) const
{
    tx.signature = _key.sign(tx.getHash());
    tx.public_key = _key.getRawPublicKey();  
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

void Wallet::update(const HGO::CHAIN::Block & blk)
{
    T_LIST txns = Blockchain::parseTransactions(blk.getData());
    if(txns.size()) {
        for(const Transaction & tx : txns)
        {
            if(tx.from == _address || tx.to == _address) {
                Transaction::Direction tx_type = Transaction::Direction::BOTH;
                if (tx.from != tx.to) {
                    if(tx.from == _address) {
                        tx_type = Transaction::Direction::OUT;
                    } else {
                        tx_type = Transaction::Direction::IN;
                    }
                }
                if(tx_type == Transaction::IN)
                {
                    _balance += tx.amount;
                } else if(tx_type == Transaction::OUT) {
                    _balance -= tx.amount;
                }
                _transactions.push_back(tx);
            }
        }
    }
}

void Wallet::_retrieveTransactions()
{
    _balance = 0.0L;
    for( const Block & blk : _chain.getChain())
    {
        update(blk);
    }
}

Wallet::T_LIST Wallet::getTransactions() const
{
    return _transactions;
}

Wallet::~Wallet() {}

bool Wallet::createWallet(const std::string walletFile)
{
    std::ofstream out (walletFile, std::ios_base::binary);
    if(out)
    {
        KeyPair keys = KeyPair::generate();
        std::string priv = keys.getPrivateKey();
        std::string pub = keys.getPublicKey();
        std::cout<<priv<<"\n"
        <<pub<<"\n\n";
        std::string raw = keys.getRawPublicKey();
        std::string keyDgst = sha256(raw);

        std::cout<<"Wallet Address : "<<keyDgst<<"\n\n";
        out<<keyDgst<<"\n"<<priv<<"\n"<<pub;
        if(out)
        {
            return true;
        }
        return false;
    }
    return false;   
}

std::ostream &HGO::TOKEN::operator<<(std::ostream &o, const Wallet & wallet)
{
    o<<"========= Wallet : "<<wallet.getAddress()<<" ("
    <<std::setprecision(std::numeric_limits<long double>::digits10 + 1)<<wallet.getBalance()<<" "<<wallet.getToken()<<")\n";

    return o.flush();

}

