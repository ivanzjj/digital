#include "encode_node.h"

#include "serializer.h"

namespace Bubi {

void 
EncodeNode::do_encode (RadixMerkleTreeNode::pointer node){
	key_size = node->get_hash ().get_bytes ();
	key_ptr = node->get_hash ().begin ();
	
	Serializer s;
	node->encode (s);
	std::vector<char> &vet = s.peek_data ();
	value_size = vet.size ();
	for (int i = 0; i < value_size; i++){
		value_ptr.push_back (vet[i]);
	}
}

}
