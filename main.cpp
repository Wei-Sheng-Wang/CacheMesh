#include "node.h"

#include <iostream>
#include <vector>


int main(int argc, char* argv[]){

    if(argc < 3){
        std::cerr << "Usage: " << argv[0] << " <address> <peer1> <peer2> ..." << std::endl;
        return 1;
    }

    std::string address = argv[1];
    std::vector<std::string> peers;
    for(int i = 2; i < argc; ++i){
        peers.push_back(argv[i]);
    }
    try{
        Node node(address, peers);
        node.start();
        std::cout << "Node started at " << address << std::endl;

        std::string input;
        std::getline(std::cin, input);
   

        node.stop();
    }catch(const std::exception& e){
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
        
    }

    return 0;

}