#ifndef SRPF_BLOCKCHAIN
#define SRPF_BLOCKCHAIN

// selon l'article : https://davenash.com/2017/10/build-a-blockchain-with-c/
// cf : Blockchain-LICENSE.txt

#include <cstdint>
#include <vector>
#include "Block.hpp"

class Blockchain
{
public:
    Blockchain(std::uint32_t difficulty);

    void prepareBlock(Block & block);
    void AddBlock(Block bNew);

    Block GetLastBlock() const;

    inline std::uint32_t getDifficulty() { return _nDifficulty; }

private:
    std::uint32_t _nDifficulty;
    std::vector<Block> _vChain;
};

#endif // SRPF_BLOCKCHAIN
