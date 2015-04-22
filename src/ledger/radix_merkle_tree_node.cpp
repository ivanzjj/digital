#include "radix_merkle_tree_node.h"

#include <assert.h>
#include <vector>

namespace Bubi{

std::mutex RadixMerkleTreeNode::child_lock_;

RadixMerkleTreeNode::RadixMerkleTreeNode (){
	branch_mask_ = 0;
	type_ = TREE_NODE_TYPE_INNER_NODE; 
	for (int i = 0; i < 16; i++){
		children_hash_[i].zero ();
		//TODO
	}
}
RadixMerkleTreeNode::~RadixMerkleTreeNode (){

}

void 
RadixMerkleTreeNode::set_hash (uint256& hash){
	hash_ = hash;
}
void
RadixMerkleTreeNode::encode (Serializer &s){
	s.add_raw (reinterpret_cast<char *>(&type_), sizeof(TreeNodeType));
	if (type_ == TREE_NODE_TYPE_INNER_NODE){
		for (int i = 0; i < 16; i++){
			s.add256 (children_hash_[i]);
		}
	}
	else if (type_ == TREE_NODE_TYPE_TRANSACTION_LEAF){
		s.add256 (item_->get_index ());
		s.add_serializer (item_->peek_serializer ());
	}
	else if (type_ == TREE_NODE_TYPE_ACCOUNT_LEAF){
		s.add256 (item_->get_index ());
		s.add_serializer (item_->peek_serializer ());
	}
	else {
		assert (false);
	}
}

void
RadixMerkleTreeNode::decode (std::string value_string){
	const char* value_ptr = value_string.c_str ();
	std::size_t pos = sizeof (TreeNodeType);

	type_ = *(reinterpret_cast<TreeNodeType *>(const_cast<char *>(value_ptr)));
	if (type_ == TREE_NODE_TYPE_INNER_NODE){
		for (int i = 0; i < 16; i++){
			children_hash_[i].init (value_ptr + pos + 32 * i);
		}	
	}
	else if (type_ == TREE_NODE_TYPE_TRANSACTION_LEAF
			|| type_ == TREE_NODE_TYPE_ACCOUNT_LEAF){
		uint256 index;
		index.init (value_ptr + pos);
		Serializer s;
		s.add_raw (value_ptr + pos + 32, value_string.length () - pos - 32);
		item_ = std::make_shared <RadixMerkleTreeLeaf> (index, s);
	}
	else{
		assert (false);
	}
}

uint256&
RadixMerkleTreeNode::get_hash (){
	return hash_;
}

bool
RadixMerkleTreeNode::is_inner (){
	return (type_ == TREE_NODE_TYPE_INNER_NODE);
}

bool
RadixMerkleTreeNode::is_empty_branch (int branch){
	return (branch_mask_ & (1 << branch)) == 0;
}

RadixMerkleTreeNode::pointer
RadixMerkleTreeNode::get_child (int branch){
	assert (branch >= 0 && branch < 16);
	assert (is_inner());
	
	std::unique_lock <std::mutex> lock (child_lock_);
	return children_[branch];
}

uint256 & 
RadixMerkleTreeNode::get_child_hash (int branch){
	assert (branch >= 0 && branch < 16);
	return children_hash_[branch];
}

void
RadixMerkleTreeNode::canonicalize (int branch, RadixMerkleTreeNode::ref node){
	assert (branch >= 0 && branch < 16);
	assert (is_inner ());
	assert (node);
	
	std::unique_lock <std::mutex> lock (child_lock_);
	children_[branch] = node;
}

bool
RadixMerkleTreeNode::set_child (RadixMerkleTreeNode::ref new_node, uint256 &hash, int branch){
	std::unique_lock <std::mutex> lock (child_lock_);
	children_hash_ [branch] = hash;
	children_ [branch] = new_node;
	branch_mask_ |= (1 << branch);
	return update_hash ();
}

bool
RadixMerkleTreeNode::update_hash (){
	uint256 nh;
	if (type_ == TREE_NODE_TYPE_INNER_NODE){
		Serializer s;
		//TODO need to add Hash prefix ?
		for (int i=0; i<16; i++){
			s.add256 (children_hash_[i]);
		}
		nh = s.get_sha512_half ();
	}
	else if (type_ == TREE_NODE_TYPE_TRANSACTION_LEAF){
		std::vector <char>& data = item_->peek_data();
		nh = Serializer::get_prefix_hash (&(data.front()), data.size());
	}
	else if (type_ == TREE_NODE_TYPE_ACCOUNT_LEAF){
		Serializer s;
		std::vector <char>& data = item_->peek_data ();
		s.add_raw (&(data.front()), data.size());
		s.add256 (item_->get_index ());
		nh = s.get_sha512_half();
	}
	else {
		// error;
		assert (false);
	}
	if (hash_ == nh){
		return false;
	}
	hash_ = nh;
	return true;
}

RadixMerkleTreeLeaf::ref
RadixMerkleTreeNode::peek_leaf (){
	return item_;
}

void
RadixMerkleTreeNode::make_inner (){
	item_.reset ();
	branch_mask_ = 0;
	type_ = TREE_NODE_TYPE_INNER_NODE; 
	hash_.zero ();
	for (int i=0; i<16; ++i){
		children_hash_[i].zero ();
		children_[i] = NULL;
	}
}

bool 
RadixMerkleTreeNode::set_item (RadixMerkleTreeLeaf::ref item, TreeNodeType node_type){
	std::unique_lock <std::mutex> lock (child_lock_);
	type_ = node_type;
	item_ = item;
	return update_hash ();
}

}
