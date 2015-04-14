#include "radix_merkle_tree.h"

#include <assert.h>
#include "utils.h"

namespace Bubi{

RadixMerkleTree::RadixMerkleTree (){
	root_ = std::make_shared <RadixMerkleTreeNode> ();
	state_ = STATE_VALID;
	type_ = TYPE_ACCOUNT_TREE;
}

RadixMerkleTree::~RadixMerkleTree (){
	
}

RadixMerkleTreeNode::pointer
RadixMerkleTree::fetch_node_from_db (uint256 &hash){

	RadixMerkleTreeNode::pointer ret;
	ret = radix_merkle_tree_db_.fetch (hash);
	return ret;
}

RadixMerkleTreeNode::pointer
RadixMerkleTree::descend (RadixMerkleTreeNode::ref parent, int branch){
	RadixMerkleTreeNode::pointer node = parent->get_child (branch);
	if (node)	return node;
	
	node = fetch_node_from_db (parent->get_child_hash (branch));
	
	parent->canonicalize (branch, node);
	return node;
}

RadixMerkleTree::RadixMerkleTreeLeafStack
RadixMerkleTree::get_stack (uint256 & hash){
	RadixMerkleTreeLeafStack stack;
	RadixMerkleTreeNode::pointer node = root_;
	int tree_depth = 0;
	
	while ( node->is_inner() ){
		stack.push (MP (node,tree_depth));
		int branch = select_branch (hash, tree_depth);
		assert ((branch >=0) && (branch<16));
		
		if (node->is_empty_branch (branch)){
			return stack;
		}
		node = descend (node, branch);
		tree_depth++;
	}
	// leaf node
	stack.push ( MP(node, tree_depth) );
	return stack;
}

int 
RadixMerkleTree::select_branch (uint256 &hash, int tree_depth){
	int byte_branch = *(hash.begin() + (tree_depth >> 1));
	if (tree_depth & 1){
		return (byte_branch & 0xF);
	}
	else{
		return (byte_branch >> 4);
	}
}

bool 
RadixMerkleTree::add_given_item (RadixMerkleTreeLeaf::pointer item, bool is_transaction){
	uint256 index = item->get_index ();
	RadixMerkleTreeLeafStack stack = get_stack (index);
	RadixMerkleTreeNode::pointer node;
	int tree_depth;
	
	RadixMerkleTreeNode::TreeNodeType node_type = is_transaction ? RadixMerkleTreeNode::TREE_NODE_TYPE_TRANSACTION_LEAF : RadixMerkleTreeNode::TREE_NODE_TYPE_ACCOUNT_LEAF;
	// make sure stack is not empty
	assert (!stack.empty());
	node = stack.top().first;
	tree_depth = stack.top().second;
	stack.pop();
	
	if (node->is_inner()){
		// the parent node of the added node is a inner node
		int branch = select_branch (index, tree_depth);
		assert (node->is_empty_branch (branch));
		RadixMerkleTreeNode::pointer new_node = 
			std::make_shared <RadixMerkleTreeNode> (item, node_type);
		if (!node->set_child (new_node, new_node->get_hash(), branch)){
			assert (false);
		}
	}
	else {
		// leaf node 
		RadixMerkleTreeLeaf::pointer other_item = node->peek_leaf ();
		assert (other_item && (index != other_item->get_index()));
		uint256 other_index = other_item->get_index ();
		
		node->make_inner ();
		int b1, b2;
		
		while ((b1 = select_branch (index, tree_depth)) == 
			   (b2 = select_branch (other_index, tree_depth)) ){
			
			stack.push (MP (node,tree_depth));
			tree_depth++;
			node = std::make_shared <RadixMerkleTreeNode> ();
			node->make_inner ();
		}
		assert (node->is_inner());
		
		RadixMerkleTreeNode::pointer new_node = 
			std::make_shared <RadixMerkleTreeNode> (item, node_type);
		if (!node->set_child (new_node, new_node->get_hash (), b1)){
			assert (false);
		}
		
		new_node = std::make_shared <RadixMerkleTreeNode> (other_item, node_type);
		if (!node->set_child (new_node, new_node->get_hash (), b2)){
			assert (false);
		}
	}

	dirty_up (stack, index, node);
	return true;
}

void 
RadixMerkleTree::dirty_up (RadixMerkleTreeLeafStack& stack, uint256& index, RadixMerkleTreeNode::ref child){
	
	while (!stack.empty()){
		RadixMerkleTreeNode::pointer node = stack.top().first;
		int tree_depth = stack.top().second;
		stack.pop();
		assert (node->is_inner());
		
		int branch = select_branch (index, tree_depth);
		if (!node->set_child (child, child->get_hash(), branch)){
			assert (false);
		}
		child = std::move (node);
	}
}

bool 
RadixMerkleTree::add_item (const RadixMerkleTreeLeaf::pointer item, bool is_transaction){
	return add_given_item (item, is_transaction);
}

bool
RadixMerkleTree::has_item (uint256& hash){
	RadixMerkleTreeNode *leaf = walk_to_leaf (hash);
	return (leaf != NULL);
}

RadixMerkleTreeNode*
RadixMerkleTree::walk_to_leaf (uint256& hash){
//	RadixMerkleTreeNode *now = root_.get ();
	RadixMerkleTreeNode::pointer now = root_;

	int depth = 0, branch;
	
	while (now->is_inner()){
		branch = select_branch (hash, depth);
		if (now->is_empty_branch (branch)){
			return NULL;
		}
		now = descend (now, branch);
		depth++;
	}
	return (now->peek_leaf()->get_index() == hash ? now.get () : NULL);
}

// this smart pointer is the same to NULL in pointer
static const RadixMerkleTreeLeaf::pointer no_item;

RadixMerkleTreeLeaf::pointer
RadixMerkleTree::peek_item (uint256& hash){
	RadixMerkleTreeNode* leaf = walk_to_leaf (hash);
	if (leaf == NULL)	
		return no_item;
	return leaf->peek_leaf ();
}

bool 							
RadixMerkleTree::update_given_item (RadixMerkleTreeLeaf::ref item, bool is_transaction){
	uint256 index = item->get_index ();
	RadixMerkleTreeLeafStack stack = get_stack (index);
	RadixMerkleTreeNode::TreeNodeType node_type = is_transaction ? RadixMerkleTreeNode::TREE_NODE_TYPE_TRANSACTION_LEAF : RadixMerkleTreeNode::TREE_NODE_TYPE_ACCOUNT_LEAF;
	
	if (stack.empty()){
		assert (false);
	}
	RadixMerkleTreeNode::pointer node = stack.top().first;
	int tree_depth = stack.top().second;
	stack.pop ();
	
	assert (!node->is_inner() && node->peek_leaf()->get_index() == index);
	
	if (!node->set_item (item, node_type)){
		BUBI_LOG ("leaf has no change");
		return true;
	}
	dirty_up (stack, index, node);
	return true;
}
	

}
