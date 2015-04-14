#include "ledger.h"

#include "utils.h"

namespace Bubi{

Ledger::Ledger (){
	transaction_tree_ = std::make_shared <RadixMerkleTree> ();
	account_tree_ = std::make_shared <RadixMerkleTree> ();
}
Ledger::~Ledger (){

}

RadixMerkleTree::ref
Ledger::get_account_tree (){
	return account_tree_;
}

RadixMerkleTree::ref
Ledger::get_transaction_tree (){
	return transaction_tree_;
}

bool 
Ledger::add_account_tree_entry (uint256 &tag, Serializer &s){
	RadixMerkleTreeLeaf::pointer item = std::make_shared <RadixMerkleTreeLeaf> (tag, s);
//	RadixMerkleTreeLeaf item (tag, s);
	return account_tree_->add_item (item, false);
}

bool 
Ledger::has_account (uint256& hash){
	return account_tree_->has_item (hash);
}

bool
Ledger::update_account_tree_entry (RadixMerkleTreeLeaf::ref item){
	return account_tree_->update_given_item (item, false);
}

}
