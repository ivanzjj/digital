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

void
RadixMerkleTreeLeaf::data_to_string (){
	std::vector <unsigned char>& data = peek_data ();
	int sz = data.size ();
	for (int i = 0; i < sz; i++){
		printf ("%2x ", data[i]);
	}
	printf ("\n");
}

}
