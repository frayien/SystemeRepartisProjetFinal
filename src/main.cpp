#include <iostream>

#include <SFML/Network.hpp>
#include <SFML/System/Thread.hpp>

#include "Blockchain.hpp"

#include "Server.hpp"
#include "Client.hpp"

int main(int argc, char** argv)
{
    try
    {
    
    Server server(5643);
    bool running = true;

    sf::Thread thread_server([&](){
        while(running)
        {
            try
            {
                server.run();
            }
            catch(const std::exception& e)
            {
                std::cerr << e.what() << '\n';
            }
        }
    });
    thread_server.launch();

    Client client(sf::IpAddress("localhost"), 5643);

    sf::Thread thread_client([&](){
        while(running)
        {
            try
            {
                client.run();
            }
            catch(const std::exception& e)
            {
                std::cerr << e.what() << '\n';
            }
        }
    });
    thread_client.launch();

    client.connect();
    client.sendTransaction("block 1");

    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }
    
    return 0;
}