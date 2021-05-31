#include <iostream>
#include <map>
#include <string>
#include <HGO>


using ARGS = std::map<std::string, std::string>;
ARGS parseArgs(int argc, char ** argv)
{
    ARGS lst;
    if(argc > 1)
    {
        for( int i = 1; i < argc; ++i)
        {
            std::string command(argv[i]);
            if(command.substr(0,2) == "--")
            {
                std::string::size_type posEqual = command.find_first_of('=');
                std::string commandName;
                std::string value;
                if(posEqual == std::string::npos)
                {
                    commandName = command.substr(2);
                    value = "";
                    
                } else {
                    commandName = std::string(command.begin() + 2, command.begin() + posEqual);
                    value = std::string(command.begin() + posEqual + 1, command.end());
                }
                lst[commandName] = value;   
            }
        }
    }
    return std::move(lst);
}

bool hasOption(const ARGS & args, const std::string & search)
{
    ARGS::const_iterator found = args.find(search);
    if(found == args.cend())
        return false;

        return true;

}


using namespace HGO::NETWORK;
//static HGONetworkManager server;
/*
HGONetworkManager::PEER_LIST parsePeer(const std::string &peers_lst)
{
  
    HGONetworkManager::PEER_LIST returned_peers;
    std::cout<<"\033[31m[Received peer list]\033[0m"<<peers_lst<<"\n";
    std::string::size_type pos = std::string::npos;
    std::string::size_type previousPos = 0;
    if(peers_lst.size() < 2)
        return returned_peers;
    while((pos = peers_lst.find_first_of(";", previousPos)) != std::string::npos)
    {
        std::string peer(peers_lst.cbegin() + previousPos, peers_lst.cbegin() + pos);
        std::string::size_type posSemicolon = peer.find_first_of(':', 0);

        if(posSemicolon == std::string::npos)
            continue;

        unsigned short port;
        std::string ip_addr = peer.substr(0, posSemicolon);

        std::istringstream iss(peer);
        iss.ignore(posSemicolon + 1);

        std::cout<<"Try peer "<<ip_addr;
        if(iss>>port)
        {
            std::cout<<" port "<<port;
            server.connectToPeer(ip_addr, port);
        }    
        std::cout<<"\n";

        previousPos = pos + 1;
    }
    return returned_peers;
}

static bool canbroadcast = true;

void processMessage(const HGOPeer &peer, const std::string & msg)
{
    canbroadcast = false;
    if(msg.substr(0,13) == "<<peer_list>>")
    {
            std::cout<<"\033[32m["<<peer<<"]\033[0m - requested peer list\n";
            std::cout.flush();
            HGONetworkManager::PEER_LIST _peers = server.getPeerList();
            std::string plist_message = "";
            for(const auto & p : _peers) {
                if(p == peer)
                    continue;
                plist_message += p.ip_address + ":" + std::to_string(p.port) + ";";
            }
            if(plist_message.empty())
                plist_message = " ";
            server.sendTo(peer, plist_message); 
    }

    if(msg.substr(0,15) == "<<server_port>>")
    {
        std::cout<<"\033[32m["<<peer.ip_address<<":"<<peer.port<<"]\033[0m - requested my port server\n";
        server.sendTo(peer, std::to_string(server.serverPort()));
        std::string peerList = server.sendAndWait(peer,"<<peer_list>>");
        parsePeer(peerList);
    }

    if(msg.substr(0,11) == "<<message>>")
    {
        std::cout<<"\033[32m["<<peer<<"]\033[0m - Message received "<< msg << "\n";
    }
    canbroadcast = true;
}


void callback(const HGOPeer &peer, const HGONetworkManager::EVENT_TYPE & event, const std::string &data, HGONetworkManager * instance)
{
    using EV = HGONetworkManager::EVENT_TYPE;
    
    switch(event)
    {
        case EV::SERVER_LAUNCHED:
            std::cout<<"Server Launcher on : "<<peer.port<<"\n";
            break;
        case EV::SERVER_STOPPED:
            std::cout<<"Server stopped\n";
            break;
        case EV::MESSAGE_SENT:
            //std::cout<<peer<<" "<<std::time(nullptr)<<" <- "<<data<<"\n";
            //std::cout.flush();
            break;
        case EV::MESSAGE:
            //std::cout<<peer<<" "<<std::time(nullptr)<<" -> "<<data<<"\n";
            //std::cout.flush();
            processMessage(peer, data);
            break;
        case EV::NEW_OUTGOING:
            std::cout<<"\033[34m["<< peer <<"]\033[0m - Accepted our connection \n";
            break;
        case EV::NEW_INCOMING:
            //Ask peer server port
            unsigned short server_port;
            std::string server_port_str = server.sendAndWait(peer, "<<server_port>>");
            HGOPeer pp = peer;
            std::istringstream iss(server_port_str);
            iss >> pp.port;
            server.updatePeer(peer, false, "", pp.port); 
            std::cout<<"I Received the port : "<<pp.port<<"\n";
            std::cout<<"\033[33m["<< pp<<"]\033[0m - Has reached us\n";
            break;
        
    }
}
*/

void msg(const HGO::NETWORK::HGOPeer &p, const Message &msg)
{
    std::cout<<"From CB"<<msg<<"\n";
}


int main(int argc, char ** argv)
{

    ARGS _cmd = parseArgs(argc, argv);

    
    std::string tag;
    unsigned short srv_port = 2016;
    if(hasOption(_cmd, "tag"))
    {
        tag = _cmd["tag"];
    }
    
    bool masternode = false;
    if(hasOption(_cmd,"masternode"))
        masternode = true;

    P2PServer p(tag, masternode);

    p.setBlockchainHandlers(msg);
    if(hasOption(_cmd, "srv"))
    {
        std::istringstream iss(_cmd["srv"]);
        iss >> srv_port;
        
    }
    p.startNetwork(srv_port);

    if(hasOption(_cmd, "peer") && hasOption(_cmd, "port"))
    {
        std::string ip_address = _cmd["peer"];
        unsigned short port;
        std::istringstream iss(_cmd["port"]);
        iss >> port;
        p.connectToNode(ip_address, port);
       
    }

    std::string command;

    while(std::cin>>command)
    {
        if(command == "q")
            break;

        if(command == "block")
        {
            std::string blockData = "testblock";
            Message blk;
            blk.header.config.full_header = 0b00111000;
            blk.msg_type = Message::TYPE::NEW_BLOCK;
            blk.msg_size = blockData.size();
            blk.str = blockData;
            p.broadcast(blk);
        }

    }
        
    
    p.stopNetwork();

    return 0;

    /////////////////////////FIRST TRY without P2PServer class////////////////////

    /*server.addCallback(callback);

    unsigned short server_port = 2016;
    if(hasOption(_cmd, "srv"))
    {
        std::istringstream iss(_cmd["srv"]);
        iss >> server_port;
        
    }

    server.run(server_port);

    if(hasOption(_cmd, "peer") && hasOption(_cmd, "port"))
    {
        std::string ip_address = _cmd["peer"];
        unsigned short port;
        std::istringstream iss(_cmd["port"]);
        iss >> port;
        server.connectToPeer(ip_address, port);
       
    }

    std::thread t1([](){
        while(!server.getPeerList().empty())
        {
            
            if(!canbroadcast)
                continue;
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            server.broadcast("<<message>>coco");
            
        }
    });

    std::string in;
    while( std::cin >> in)
    {
        if(in == "b")
            server.broadcast("<<message>>Coucou");
        if(in == "q")
            break;
    }
    server.stop();
    t1.join();*/

    return 0;
}