#include "Blockchain.hpp"

// selon l'article : https://davenash.com/2017/10/build-a-blockchain-with-c/
// cf : Blockchain-LICENSE.txt

Blockchain::Blockchain()
{
    Block initialBlock("Genesis Block");
    initialBlock.nIndex = 0;

    _vChain.emplace_back(initialBlock);
    _nDifficulty = 6;
}

void Blockchain::prepareBlock(Block & block)
{
    block.nIndex = GetLastBlock().nIndex + 1;
    block.sPrevHash = GetLastBlock().sHash;
}

void Blockchain::AddBlock(Block bNew)
{
    _vChain.push_back(bNew);
}

Block Blockchain::GetLastBlock() const
{
    return _vChain.back();
}