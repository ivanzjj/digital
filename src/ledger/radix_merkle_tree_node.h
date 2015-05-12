#ifndef _BUBI_RADIX_MERKLE_TREE_NODE_H_
#define _BUBI_RADIX_MERKLE_TREE_NODE_H_

#include <memory>
#include <mutex>

#include "radix_merkle_tree_leaf.h"
#include "utils.h"

namespace Bubi{

class RadixMerkleTreeNode{
public:
	typedef std::shared_ptr<RadixMerkleTreeNode>	pointer;
	typedef pointer&								ref;
	
	enum TreeNodeType{
		TREE_NODE_TYPE_ERROR = 0,
		TREE_NODE_TYPE_INNER_NODE = 1,
		TREE_NODE_TYPE_TRANSACTION_LEAF = 2,
		TREE_NODE_TYPE_ACCOUNT_LEAF = 3
	};
	
	RadixMerkleTreeNode (RadixMerkleTreeLeaf::pointer item,TreeNodeType node_type)
	:item_(item), type_(node_type){
		branch_mask_ = 0;
		is_transaction_tree_ = (type_ == TREE_NODE_TYPE_TRANSACTION_LEAF) ? true : false;
		update_hash ();
		hash_ = item_->get_index ();
	}
	RadixMerkleTreeNode (bool is_transaction_tree = false);
	~RadixMerkleTreeNode ();

	bool 							is_empty_branch (int branch);
	RadixMerkleTreeNode::pointer 	get_child (int branch);
	uint256& 						get_child_hash(int branch);
	void canonicalize (int branch, 	RadixMerkleTreeNode::ref node);
	bool 							set_child (RadixMerkleTreeNode::ref new_node, uint256 &hash, int branch);
	bool 							update_hash();
	RadixMerkleTreeLeaf::ref 		peek_leaf();
	void 							make_inner();
	bool 							set_item (RadixMerkleTreeLeaf::ref item, TreeNodeType node_type);
	bool							is_inner ();
	uint256&						get_hash ();
	void							set_hash (uint256& hash);
	void							encode (Serializer &s);
	void							decode (std::string value_string);
	
private:
	uint256							hash_;
	uint256							children_hash_[16];
	RadixMerkleTreeNode::pointer	children_[16];
	RadixMerkleTreeLeaf::pointer	item_;
	
	bool							is_transaction_tree_;
	TreeNodeType					type_;
	uint							branch_mask_;
	
	static  std::mutex				child_lock_;
};

}

#endif //end of ifndef
