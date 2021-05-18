#include <iostream>
#include "HGO"

using std::cout;
using std::cin;
using namespace HGO::P2P;

int main(int argc, char ** argv)
{
    try{
        HGOProtocolManager mgr;
        mgr.run();

        std::string v;
        while(std::cin >> v)
        {
            std::cout<<"Copy : "<<v<<"\n";

            if(v == "c") {
                mgr.connectToPeer("192.168.1.122", 999);
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