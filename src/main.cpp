#include <iostream>

#include <SFML/Network.hpp>
#include <SFML/System/Thread.hpp>

#include "Blockchain.hpp"

#include "Server.hpp"
#include "Client.hpp"

int main(int argc, char** argv)
{
/*
    Blockchain bChain{};

    std::cout << "Mining block 1..." << std::endl;
    bChain.AddBlock(Block(1, "Block 1 Data"));

    std::cout << "Mining block 2..." << std::endl;
    bChain.AddBlock(Block(2, "Block 2 Data"));

    std::cout << "Mining block 3..." << std::endl;
    bChain.AddBlock(Block(3, "Block 3 Data"));
*/

    Server serv(5643);
    bool running = true;

    sf::Thread th([&](){
        while(running)
        {
            try
            {
                serv.run();
            }
            catch(const std::exception& e)
            {
                std::cerr << e.what() << '\n';
            }
        }
    });
    th.launch();

    try
    {
        Client cli(sf::IpAddress("localhost"), 5643);
        sf::sleep(sf::seconds(2));
        running = false;
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }
    
    return 0;
}