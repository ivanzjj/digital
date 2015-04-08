#include "ledger.h"

namespace Bubi{

bool 
Ledger::add_account_tree_entry (uint256 &tag, Serializer &s){
	RadixMerkleTreeLeaf item (tag, s);
	account_tree_->add_item (item, false);
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
