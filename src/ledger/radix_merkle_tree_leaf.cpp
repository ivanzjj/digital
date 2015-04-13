#include "radix_merkle_tree_leaf.h"

#include <vector>

namespace Bubi{

RadixMerkleTreeLeaf::RadixMerkleTreeLeaf(
	uint256 &_index,
	Serializer &_data)
	: index_ (_index)
	, data_ (_data)
{
}

std::vector <unsigned char> &
RadixMerkleTreeLeaf::peek_data (){
	return data_.peek_data();
}

}
