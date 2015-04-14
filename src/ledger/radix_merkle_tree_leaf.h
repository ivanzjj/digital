#ifndef _BUBI_RADIX_MERKLE_TREE_LEAF_H_
#define _BUBI_RADIX_MERKLE_TREE_LEAF_H_

#include <vector>

#include "utils.h"
#include "serializer.h"

namespace Bubi{

class RadixMerkleTreeLeaf{
public:
	typedef	std::shared_ptr<RadixMerkleTreeLeaf> pointer;
	typedef pointer&							 ref;
	
	RadixMerkleTreeLeaf (uint256 &, Serializer &);
	uint256& get_index (){
		return index_;
	}
	std::vector <unsigned char>& peek_data ();
	void data_to_string ();

private:
	uint256		index_;
	Serializer	data_;
};

}

#endif //end of ifndef
