#include "radix_merkle_tree_leaf.h"

namespace Bubi{

RadixMerkleTreeLeaf::RadixMerkleTreeLeaf(
	uint256 &_index,
	Serializer &_data)
	: index_ (_index)
	, data_ (_data)
{
}

vector <unsigned char> &
RadixMerkleTreeLeaf::peek_data (){
	return data_.peek_data();
}

}
