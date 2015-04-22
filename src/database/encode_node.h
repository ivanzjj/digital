#ifndef _BUBI_ENCODE_NODE_H_
#define _BUBI_ENCODE_NODE_H_

#include "radix_merkle_tree_node.h"

#include <cstddef>

namespace Bubi {

class EncodeNode {

public:
	
	char*		get_key (){
		return reinterpret_cast<char *>key_ptr;
	}
	char*		get_value (){
		return reinterpret_cast<char *>(value_ptr.c_str());
	}
	std::size_t	get_key_size (){
		return key_size;
	}
	std::size_t get_value_size (){
		return value_size;
	}
	void do_encode (RadixMerkleTreeNode::pointer node);

private:
	std::size_t		key_size;
	std::size_t		value_size;
	void *			key_ptr;
	std::string		value_ptr;
};

}

#endif
