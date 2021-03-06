#include "radix_merkle_tree.h"

#include <assert.h>
#include <queue>
#include "utils.h"
#include "rocksdb_imp.h"

namespace Bubi{

RadixMerkleTree::RadixMerkleTree (bool is_transaction_tree){
	root_ = std::make_shared <RadixMerkleTreeNode> (is_transaction_tree);
	state_ = STATE_VALID;
	type_ = is_transaction_tree ? TYPE_TRANSACTION_TREE : TYPE_ACCOUNT_TREE;
	radix_merkle_tree_db_ = RocksdbInstance::instance ();
}

RadixMerkleTree::~RadixMerkleTree (){
	printf ("RadixMerkleTree done\n");
	backup_to_database ();
	printf ("RadixMerkleTree done2\n");
}

RadixMerkleTree::RadixMerkleTree (uint256 hash, std::uint32_t ledger_seq, bool is_transaction_tree) : ledger_sequence_ (ledger_seq){
	radix_merkle_tree_db_ = RocksdbInstance::instance ();
	root_ = fetch_node_from_db (hash);
	state_ = STATE_VALID;
	type_ = is_transaction_tree ? TYPE_TRANSACTION_TREE : TYPE_ACCOUNT_TREE;
}

void 
RadixMerkleTree::backup_to_database (){
	Batch batch;
	RadixMerkleTreeNode::pointer now, tmp;
	std::queue <RadixMerkleTreeNode::pointer> que;
	batch.push_back (root_);
	que.push (root_);

	while (!que.empty ()){
		now = que.front(); que.pop();
		printf ("AQAAAA\n");
		if (now->is_inner()){
			for (int i = 0; i < 16; i++) if (tmp = now->get_child(i)){
				que.push (tmp);
				batch.push_back (tmp);
			}
		}
	}
	radix_merkle_tree_db_->store_batch (batch);
}

RadixMerkleTreeNode::pointer
RadixMerkleTree::fetch_node_from_db (uint256 &hash){

	RadixMerkleTreeNode::pointer ret;
	ret = radix_merkle_tree_db_->fetch (hash);
	return ret;
}
void
RadixMerkleTree::store_node (RadixMerkleTreeNode::pointer node){
	radix_merkle_tree_db_->store (node);
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
RadixMerkleTree::select_branch (uint256 &hash, unsigned int tree_depth){
	unsigned int byte_branch = *(unsigned char *)(hash.begin() + (tree_depth >> 1));
	if (tree_depth & 1){
		return (int) (byte_branch & 0xF);
	}
	else{
		return (int)(byte_branch >> 4);
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

Account::pointer
RadixMerkleTree::get_account_entry (uint256& hash){
	RadixMerkleTreeNode::pointer now = root_;
	int depth = 0, branch;
	Account::pointer ret = nullptr;

	while (now->is_inner ()){
		branch = select_branch (hash, depth);
		if (now->is_empty_branch (branch)){
			return ret;
		}
		now = descend (now, branch);
		depth++;
	}
	if (now->peek_leaf ()->get_index () != hash){
		return ret;
	}
	ret = std::make_shared<Account> ();
	std::string str = now->peek_leaf ()->peek_string ();
	ret->unserializer (str);
	return ret;
}

Transaction::pointer
RadixMerkleTree::get_transaction_entry (uint256& hash){
	RadixMerkleTreeNode::pointer now = root_;
	int depth = 0, branch;
	Transaction::pointer ret = nullptr;

	while (now->is_inner ()){
		branch = select_branch (hash, depth);
		if (now->is_empty_branch (branch))
			return ret;
		now = descend (now, branch);
		depth++;
	}
	if (now->peek_leaf ()->get_index () != hash)
		return ret;
	ret = std::make_shared <Transaction> ();
	std::string str = now->peek_leaf ()->peek_string ();
	ret->unserializer (str);
	return ret;
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
	
uint256
RadixMerkleTree::get_hash (){
	return root_->get_hash ();
}


}
