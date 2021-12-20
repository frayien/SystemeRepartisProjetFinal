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
    Server::Mode mode = Server::Mode::PoW;

    bool doRunServer = true;
    std::size_t client_n = 10;

    bool running = true;

    std::unique_ptr<Server> server;
    std::unique_ptr<sf::Thread> server_thread;

    struct ClientRunnable
    {
        std::unique_ptr<Client> client;
        std::unique_ptr<sf::Thread> client_thread;
    };

    std::vector<ClientRunnable> clients;

    if(doRunServer)
    {
        serverIp = "localhost";
    }

    try
    {
    if(doRunServer)
    {
        std::cout << "Starting Server on port " << serverPort << std::endl;

        server = std::make_unique<Server>(serverPort, difficulty, mode);

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

    for(std::size_t i = 0; i < client_n; ++i)
    {
        std::cout << "Starting Client " << i << " on server " << serverIp << ":" << serverPort << std::endl;

        auto& client = clients.emplace_back();

        client.client = std::make_unique<Client>(sf::IpAddress(serverIp), serverPort, i);

        client.client_thread = std::make_unique<sf::Thread>([&clients, &running, i]()
        {
            while(running)
            {
                try
                {
                    clients[i].client->run();
                }
                catch(std::exception& e)
                {
                    std::cerr << "[ERROR][CLIENT-" << i << "] " << e.what() << '\n';
                }
            }
        });
        client.client_thread->launch();

        client.client->connect();
    }

    while(running)
    {
        std::string cmd;

        std::cin >> cmd;

        if(client_n > 0 && (cmd == "transaction" || cmd == "tr"))
        {   
            std::string args;
            std::getline(std::cin, args);

            while(std::isspace(args[0])) // trim
            {
                args = args.substr(1);
            }

            clients[0].client->log({"Send Transaction '", args, "'"});
            clients[0].client->sendTransaction(args);
        }
        else if(cmd == "stop")
        {
            running = false;
            if(client_n > 0)
            {
                for(auto& cli : clients)
                {
                    cli.client->disconnect();
                    cli.client_thread->terminate();
                }
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