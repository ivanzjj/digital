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

Serializer&
RadixMerkleTreeLeaf::peek_serializer (){
	return data_;
}

std::vector <char> &
RadixMerkleTreeLeaf::peek_data (){
	return data_.peek_data();
}

void
RadixMerkleTreeLeaf::data_to_string (){
	std::vector <char>& data = peek_data ();
	int sz = data.size ();
	for (int i = 0; i < sz; i++){
		printf ("%2x ", data[i]);
	}
	printf ("\n");
}

}
