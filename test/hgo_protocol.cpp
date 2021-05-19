#include <iostream>
#include <HGO>

using std::cout;
using std::cin;
using namespace HGO::P2P;

void testcb(const HGOPeer &peer, const HGOProtocolManager::EVENT_TYPE & event, const std::string &data)
{
    using EVENT_TYPE = HGOProtocolManager::EVENT_TYPE;
    switch(event)
    {
        case EVENT_TYPE::SERVER_LAUNCHED:
            std::cout<<"\033[35mServer has been launched on port : \033[0m"<<peer.port<<"\n";
        break;
        case EVENT_TYPE::SERVER_STOPPED:
            std::cout<<"\033[35mServer stopped \033[0m"<<"\n";
        break;
        case EVENT_TYPE::NEW_INCOMING:
            std::cout<<"\033[32m["<<peer.ip_address<<"] - New incoming connection \033[0m\n";
        break;
        case EVENT_TYPE::NEW_OUTGOING:
            std::cout<<"\033[33m["<<peer.ip_address<<"] - New outgoing connection \033[0m\n";
        break;
        case EVENT_TYPE::MESSAGE:
            std::cout<<"\033[34m["<<peer.ip_address<<"] - Message received \033[0m => "<< data;
        break;
        case EVENT_TYPE::PEER_DISCONNECTED :
            std::cout<<"\033[31m["<<peer.ip_address<<"] - Has been disconnected \033[0m\n";
        break;
    }
}

int main(int argc, char ** argv)
{
    try{
        HGOProtocolManager mgr;
        mgr.addCallback(testcb);
        mgr.run();

        std::string v;
        while(std::cin >> v)
        {
            if(v == "c") {
                mgr.connectToPeer("192.168.1.122", 999);
            }
            if(v == "b") {
                mgr.broadcast("Hello");
            }

            if( v == "q") {
                mgr.stop();
                break;
            }
                
        }
  

    }catch( HGO::EXCEPTION::ProtocolError &e) {
        std::cout<<e.what()<<"\n";
    }


    return 0;
}