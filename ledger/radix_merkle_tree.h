#ifndef _BUBI_RADIX_MERKLE_TREE_H_
#define _BUBI_RADIX_MERKLE_TREE_H_

#include <memory>

#include "radix_merkle_tree_node.h"
#include "radix_merkle_tree_node.h"

#include "utils.h"

namespace Bubi{

enum RadixMerkleTreeState{ 
	STATE_VALID 		= 1
};

enum RadixMerkleTreeType{
	TYPE_TRANSACTION_TREE = 1,
	TYPE_ACCOUNT_TREE	  = 2,
	TYPE_UNKNOWN_TREE	  = 3
};

class RadixMerkleTree{
public:
	typedef std::shared_ptr <RadixMerkleTree>	pointer;
	typedef std::shared_ptr <RadixMerkleTree>& 	ref;
	typedef std::stack <std::pair<RadixMerkleTreeLeaf::pointer, int> > RadixMerkleTreeLeafStack;

	bool 							add_item (const RadixMerkleTreeLeaf& item, bool is_transaction);
	bool 							add_given_item (const RadixMerkleTreeLeaf& item, bool is_transaction);
	RadixMerkleTreeLeafStack 		get_stack (uint256& hash);
	int 							select_branch (uint256 &hash, int tree_depth);
	RadixMerkleTreeNode::pointer 	fetch_node_from_db (uint256& hash);
	RadixMerkleTreeNode::pointer	descend (RadixMerkleTreeNode::ref parent, int branch);
	void 							dirty_up (RadixMerkleTreeLeafStack& stack, uint256& index, RadixMerkleTreeNode::ref child);
	bool 							has_item (uint256 const& hash);
	RadixMerkleTreeNode* 			walk_to_leaf (uint256 const& hash);
	RadixMerkleTreeLeaf::pointer 	peek_item (uint256 const& hash);
	
	bool 							update_given_item (RadixMerkleTreeLeaf::ref item, bool is_transaction);
	
private:
	
	DataBase	radix_merkle_tree_db_;
	std::uint32_t	ledger_sequence_;
	
	RadixMerkleTreeNode::pointer 	root_;
	RadixMerkleTreeState			state_;
	RadixMerkleTreeType				type_;
	
};

}

#endif //end if ifndef _BUBI_RADIX_MERKLE_TREE_H_