#ifndef _BUBI_ENCODE_NODE_H_
#define _BUBI_ENCODE_NODE_H_

#include "radix_merkle_tree_node.h"

#include <cstddef>

namespace Bubi {

class EncodeNode {

public:
	
	char*		get_key (){
		return (char *)(&key_ptr[0]);
	}
	char*		get_value (){
		return (char *)(&value_ptr[0]);
	}
	std::size_t	get_key_size (){
		return key_size;
	}
	std::size_t get_value_size (){
		return value_size;
	}
	std::string get_key_ptr (){
		return key_ptr;
	}
	std::string get_value_ptr (){
		return value_ptr;
	}
	void do_encode (RadixMerkleTreeNode::pointer node);

private:
	std::size_t		key_size;
	std::size_t		value_size;
	std::string		key_ptr;
	std::string		value_ptr;
};

}

#endif
