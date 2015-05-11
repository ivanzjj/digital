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

std::string
RadixMerkleTreeLeaf::peek_string (){
	std::vector <char>& ret = peek_data ();
	std::string res_str = "";
	std::size_t sz = ret.size ();
	for (int i = 0; i < sz; i++)
		res_str.push_back (ret[i]);
	return res_str;
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
