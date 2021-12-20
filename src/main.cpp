#include <iostream>
#include <atomic>
#include <array>
#include <random>
#include <fstream>
#include <filesystem>

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
    const std::uint16_t serverPort = 5643;
    const std::string serverIp = "localhost";
    
    const std::uint32_t difficulty = 4;
    const Server::Mode mode = Server::Mode::PoS;

    const bool doRunServer = true;
    const std::size_t client_n = 10;

    const bool benchmark_mode = true;

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
    std::atomic_int64_t total_nonce_1block = 0;
    const std::size_t benchmark_string_size = 10;
    const std::size_t benchmark_block_n = 1000;


    std::ofstream benchmark_output;

    if(benchmark_mode)
    {
        std::filesystem::path benchmark_output_path("benchmark.csv");

        for(std::size_t i = 1; std::filesystem::exists(benchmark_output_path); ++i)
        {
            benchmark_output_path.replace_filename("benchmark (" + std::to_string(i) + ").csv");
        }
        benchmark_output.open(benchmark_output_path);
        if(!benchmark_output) std::cerr << "Could not open " << benchmark_output_path << std::endl;
        benchmark_output << "Block id;Hash;Time (ms);Hash cumul; Time (ms) cumul\n";
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
        server->setCallback([&benchmark_output, &benchmark_clock, &benchmark_clock_glob, &benchmark_block_n, &clients, &benchmark_string_size, &total_nonce, &total_nonce_1block](Operation op, std::size_t i)
        {
            if(op == Operation::REQUEST_VALIDATION)
            {
                if(i < benchmark_block_n)
                {
                    clients[0].client->sendTransaction(random_string(benchmark_string_size));
                }
                benchmark_clock.restart();
                total_nonce_1block = 0;
            }
            if(op == Operation::END_MINING)
            {
                sf::Int32 current_block_time = benchmark_clock.getElapsedTime().asMilliseconds();
                sf::Int32 global_time = benchmark_clock_glob.getElapsedTime().asMilliseconds();
                std::cout << "[BENCHMARK] MINED BLOCK " << i << " IN " << current_block_time << " MS" << std::endl;
                std::cout << "[BENCHMARK] TOTAL TIME " << global_time << " MS" << std::endl;
                std::cout << "[BENCHMARK] TOTAL HASH " << total_nonce << std::endl;
                benchmark_output << i << ";" << total_nonce_1block << ";" << current_block_time << ";" << total_nonce << ";" << global_time << "\n";
                benchmark_output.flush();
            }
        });

        for(auto& [cli, thead] : clients)
        {
            cli->setCallbackNonce([&total_nonce, &total_nonce_1block](std::int64_t nonce)
            {
                total_nonce += nonce;
                total_nonce_1block += nonce;
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
            total_nonce_1block = 0;

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