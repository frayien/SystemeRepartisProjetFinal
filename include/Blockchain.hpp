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
    Blockchain();

    void AddBlock(Block bNew);

private:
    std::uint32_t _nDifficulty;
    std::vector<Block> _vChain;

    Block _GetLastBlock() const;
};

#endif // SRPF_BLOCKCHAIN
