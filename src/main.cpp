#include <iostream>

#include <SFML/Network.hpp>
#include <SFML/System/Thread.hpp>

#include "Blockchain.hpp"

#include "Server.hpp"
#include "Client.hpp"

int main(int argc, char** argv)
{
    std::uint16_t serverPort = 5643;
    std::string serverIp = "localhost";
    
    std::uint32_t difficulty = 5;

    bool doRunServer = true;
    bool doRunClient = true;

    bool running = true;

    std::unique_ptr<Server> server;
    std::unique_ptr<sf::Thread> server_thread;

    std::unique_ptr<Client> client;
    std::unique_ptr<sf::Thread> client_thread;

    if(doRunServer)
    {
        serverIp = "localhost";
    }

    try
    {
    if(doRunServer)
    {
        std::cout << "Starting Server on port " << serverPort << std::endl;

        server = std::make_unique<Server>(serverPort, difficulty);

        server_thread = std::make_unique<sf::Thread>([&]()
        {
            while(running)
            {
                try
                {
                    server->run();
                }
                catch(std::exception& e)
                {
                    std::cerr << e.what() << '\n';
                }
            }
        });
        server_thread->launch();
    }

    if(doRunClient)
    {
        std::cout << "Starting Client on server " << serverIp << ":" << serverPort << std::endl;

        client = std::make_unique<Client>(sf::IpAddress(serverIp), serverPort);

        client_thread = std::make_unique<sf::Thread>([&]()
        {
            while(running)
            {
                try
                {
                    client->run();
                }
                catch(std::exception& e)
                {
                    std::cerr << e.what() << '\n';
                }
            }
        });
        client_thread->launch();

        client->connect();
    }

    while(running)
    {
        std::string cmd;

        std::cin >> cmd;

        if(doRunClient && (cmd == "transaction" || cmd == "tr"))
        {   
            std::string args;
            std::getline(std::cin, args);

            while(std::isspace(args[0])) // trim
            {
                args = args.substr(1);
            }

            client->log({"Send Transaction '", args, "'"});
            client->sendTransaction(args);
        }
        else if(cmd == "stop")
        {
            running = false;
            if(doRunClient)
            {
                client->disconnect();
                client_thread->terminate();
            }
            if(doRunServer)
            {
                sf::sleep(sf::milliseconds(50));
                server_thread->terminate();
            }
        }

        sf::sleep(sf::milliseconds(10));
    }

    }
    catch(std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }
    
    return 0;
}