#ifndef _BUBI_RADIX_MERKLE_TREE_LEAF_H_
#define _BUBI_RADIX_MERKLE_TREE_LEAF_H_

namespace Bubi{

class RadixMerkleTreeLeaf{
public:
	typedef	std::shared_ptr<RadixMerkleTreeLeaf> pointer;
	
	RadixMerkleTreeLeaf (uint256 &, Serializer &);
	uint256 & get_index () const{
		return index_;
	}
	std::vector <unsigned char>& peek_data ();
	
	
private:
	uint256		index_;
	Serializer	data_;
};

}

#endif //end of ifndef