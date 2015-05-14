#include "interface.h"
#include "utils.h"
#include "serializer.h"

namespace Bubi {


extern Ledger::pointer last_ledger;

int create_transaction (std::string source_addr, std::string det_addr, double amount){
	Serializer ss (source_addr);
	uint256 source = ss.get_sha512_half (); 
	Serializer ds (det_addr);
	uint256 det = ds.get_sha512_half (); 
	Transaction::pointer tx = std::make_shared <Transaction> (source, det, amount);
	return last_ledger->add_transaction_entry (tx);
}

uint256 string_address_to_uint256 (std::string str_addr){
	Serializer ss (str_addr);
	return ss.get_sha512_half ();
}


int create_account (std::string acc_pub){
	uint256 hash = string_address_to_uint256 (acc_pub);
    uint256 p_tx;
	p_tx.zero ();
	Account::pointer acc = std::make_shared <Account> (hash, 1000, 0, p_tx);
	last_ledger->add_account_tree_entry (hash, acc);
	return 0;
}

double get_balance (std::string acc_pub){
	uint256 hash = string_address_to_uint256 (acc_pub);
    Account::pointer acc = last_ledger->get_account_entry (hash);
	return acc->get_account_balance ();
}

std::vector <Transaction::pointer> get_transaction_history (std::string acc_pub){
	uint256 hash = string_address_to_uint256 (acc_pub);
	Account::pointer acc = last_ledger->get_account_entry (hash);
	std::vector <Transaction::pointer> res;

	uint256 tx_hash;
	std::uint32_t   tx_seq;

	tx_seq = acc->get_previous_ledger_seq ();
	tx_hash = acc->get_previous_tx_hash ();


	while ( tx_seq != 0 ){
		Transaction::pointer tx = last_ledger->get_transaction_entry (tx_hash);
		res.push_back (tx);
/*
		std::cout << "------------------------------------" << std::endl;
		std::cout << tx->get_source_address ().to_string () << std::endl;
		std::cout << tx->get_destination_address ().to_string () << std::endl;
		std::cout << tx->get_payment_amount () << std::endl;
*/
		if (hash == tx->get_source_address ()){
			tx_seq = tx->get_source_previous_ledger_seq ();
			tx_hash = tx->get_source_previous_tx_hash ();
		}
		else {
			tx_seq = tx->get_destination_previous_ledger_seq ();
			tx_hash = tx->get_destination_previous_tx_hash ();
		}
	}
	return res;
}

}
