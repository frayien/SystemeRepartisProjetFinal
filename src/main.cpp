#include <iostream>

#include <SFML/Network.hpp>

#include "Blockchain.hpp"

int main(int argc, char** argv)
{
    sf::IpAddress ip{};
    
    Blockchain bChain{};

    std::cout << "Mining block 1..." << std::endl;
    bChain.AddBlock(Block(1, "Block 1 Data"));

    std::cout << "Mining block 2..." << std::endl;
    bChain.AddBlock(Block(2, "Block 2 Data"));

    std::cout << "Mining block 3..." << std::endl;
    bChain.AddBlock(Block(3, "Block 3 Data"));

    return 0;
}