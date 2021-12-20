#include <iostream>
#include <atomic>
#include <array>
#include <random>

#include <SFML/Network.hpp>
#include <SFML/System/Thread.hpp>

#include "Blockchain.hpp"

#include "Server.hpp"
#include "Client.hpp"

std::string random_string(std::size_t size)
{
    std::array<char, 26> letters { 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'z' };
    std::mt19937 random_engine(std::random_device{}());
    std::uniform_int_distribution dist(0, 25);

    std::string ret;
    for(std::size_t i = 0; i < size; ++i)
    {
        ret += letters[dist(random_engine)];
    }

    return ret;
}

int main(int argc, char** argv)
{
    std::uint16_t serverPort = 5643;
    std::string serverIp = "localhost";
    
    std::uint32_t difficulty = 5;
    Server::Mode mode = Server::Mode::PoS;

    bool doRunServer = true;
    std::size_t client_n = 10;

    bool benchmark_mode = true;

    bool running = true;

    std::unique_ptr<Server> server;
    std::unique_ptr<sf::Thread> server_thread;

    struct ClientRunnable
    {
        std::unique_ptr<Client> client;
        std::unique_ptr<sf::Thread> client_thread;
    };

    std::vector<ClientRunnable> clients;

    sf::Clock benchmark_clock;
    sf::Clock benchmark_clock_glob;
    std::atomic_int64_t total_nonce = 0;
    std::uint32_t prev_block_id = 0;
    std::size_t benchmark_string_size = 20;
    std::size_t benchmark_block_n = 10;

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
    
    if(benchmark_mode)
    {
        server->setCallback([&prev_block_id, &benchmark_clock, &benchmark_clock_glob, &benchmark_block_n, &clients, &benchmark_string_size, &total_nonce](Operation op, std::size_t i)
        {
            if(op == Operation::REQUEST_VALIDATION)
            {
                if(benchmark_block_n > 0)
                {
                    --benchmark_block_n;
                    clients[0].client->sendTransaction(random_string(benchmark_string_size));
                }
                prev_block_id = i;
                benchmark_clock.restart();
            }
            if(op == Operation::END_MINING && prev_block_id == i)
            {
                std::cout << "[BENCHMARK] MINED BLOCK " << prev_block_id << " IN " << benchmark_clock.getElapsedTime().asMilliseconds() << " MS" << std::endl;
                std::cout << "[BENCHMARK] TOTAL TIME " << benchmark_clock_glob.getElapsedTime().asMilliseconds() << " MS" << std::endl;
                std::cout << "[BENCHMARK] TOTAL HASH " << total_nonce << std::endl;
            }
        });

        for(auto& [cli, thead] : clients)
        {
            cli->setCallbackNonce([&total_nonce](std::int64_t nonce)
            {
                total_nonce += nonce;
            });
        }
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
        else if(cmd == "benchmark")
        {
            benchmark_clock.restart();
            benchmark_clock_glob.restart();
            total_nonce = 0;
            benchmark_block_n = 10;

            clients[0].client->sendTransaction(random_string(benchmark_string_size));
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