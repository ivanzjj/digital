#ifndef _BUBI_LEDGER_H_
#define _BUBI_LEDGER_H_

#include <memory>

#include "radix_merkle_tree.h"

namespace Bubi{

class Ledger{
public:
	typedef std::shared_ptr <Ledger>	pointer;
	
	//accout API
	bool add_account_tree_entry (uint256&, Serializer&);
	bool has_account (uint256& hash);
	bool update_account_tree_entry (RadixMerkleTreeLeaf::ref item);
	
	
	
private:
	uint256			hash_;
	uint256 		parent_hash_;
	uint256 		transaction_tree_hash_;
	uint256			account_tree_hash_;
	
	std::uint64_t	total_coins_;
	std::uint64_t	ledger_sequence_;
	
	std::uint32_t	close_time_;
	int				close_resolution_;
	
	std::uint32_t	fee_;
	
	RadixMerkleTree::pointer 	transaction_tree_;
	RadixMerkleTree::pointer	account_tree_;
};

}
#endif //end of ifndef