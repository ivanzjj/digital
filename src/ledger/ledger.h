#ifndef _BUBI_LEDGER_H_
#define _BUBI_LEDGER_H_

#include <memory>

#include "radix_merkle_tree.h"
#include "radix_merkle_tree_leaf.h"
#include "serializer.h"
#include "utils.h"
#include "transaction.h"
#include "account.h"

#include <stdint.h>

namespace Bubi{

class Ledger{
public:
	typedef std::shared_ptr <Ledger>	pointer;
	
	//accout API
	bool add_account_tree_entry (uint256&, Account::pointer acc);
	bool has_account (uint256& hash);
	Account::pointer get_account_entry (uint256& hash);
	Transaction::pointer	get_transaction_entry (uint256& hash);

	bool update_account_tree_entry (RadixMerkleTreeLeaf::ref item);
	RadixMerkleTree::ref get_transaction_tree ();
	RadixMerkleTree::ref get_account_tree ();
	int	 update_account_tree_hash ();
	int	 update_transaction_tree_hash ();
	
	int  add_ledger_to_database ();
	int update_ledger_hash ();
	
	//transaction API
	int add_transaction_entry (Transaction::pointer tx);
	
	Ledger ();
	Ledger (uint256& hash, uint256& parent_hash, uint256& transaction_tree_hash, uint256& account_tree_hash, std::uint32_t& total_coin, std::uint32_t& ledger_sequence, std::uint32_t& close_time);
	~Ledger ();
	
private:
	uint256			hash_;
	uint256 		parent_hash_;
	uint256 		transaction_tree_hash_;
	uint256			account_tree_hash_;
	
	std::uint32_t	total_coins_;
	std::uint32_t	ledger_sequence_;
	
	std::uint32_t	close_time_;
	
	RadixMerkleTree::pointer 	transaction_tree_;
	RadixMerkleTree::pointer	account_tree_;
};

}
#endif //end of ifndef
