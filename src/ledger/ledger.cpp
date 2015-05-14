#include "ledger.h"

#include "utils.h"
#include "sqlite_imp.h"

#include <sstream>

namespace Bubi{

Ledger::Ledger (uint256& hash, uint256& parent_hash, uint256& transaction_tree_hash, uint256& account_tree_hash, std::uint32_t& total_coin, std::uint32_t& ledger_sequence, std::uint32_t& close_time)
: hash_ (hash)
, parent_hash_ (parent_hash)
, transaction_tree_hash_ (transaction_tree_hash)
, account_tree_hash_ (account_tree_hash)
, total_coins_ (total_coin)
, ledger_sequence_ (ledger_sequence)
, close_time_ (close_time)
{
	account_tree_ = std::make_shared<RadixMerkleTree> (account_tree_hash_, ledger_sequence_, false);
	transaction_tree_ = std::make_shared<RadixMerkleTree> (transaction_tree_hash_, ledger_sequence_, true);	
}

Ledger::Ledger (){
	hash_.zero ();
	parent_hash_.zero ();
	total_coins_ = 0;
	ledger_sequence_ = 1;
	close_time_ = 0;
	transaction_tree_ = std::make_shared <RadixMerkleTree> (true);
	account_tree_ = std::make_shared <RadixMerkleTree> (false);
	update_account_tree_hash ();
	update_transaction_tree_hash ();
}
Ledger::~Ledger (){
	printf ("Ledger done!\n");
	add_ledger_to_database ();
}

std::string int_to_string (std::uint32_t& a){
	std::ostringstream sout;
	sout << a;
	return sout.str ();
}
int
Ledger::add_ledger_to_database (){
	SqliteImp::pointer ledger_db = SqliteInstance::instance ();
	int ret;
	std::string sql = "";
	std::ostringstream out;

	out << "insert into BubiLedger (ledger_sequence,hash,phash,txhash,accounthash,total_coins,close_time) values (" << ledger_sequence_ << ",'" << hash_.to_string () << "','" << parent_hash_.to_string() << "','" << transaction_tree_hash_.to_string() << "','" << account_tree_hash_.to_string() << "'," << total_coins_ << "," << close_time_ << ");";
	sql = out.str ();
	std::cout << sql << std::endl;
	ret = ledger_db->exec_sql (sql, NULL);
	if (ret != SQLITE_OK){
		std::ostringstream out2;

		out2 << "update BubiLedger set hash='" << hash_.to_string()<< "',phash='" << parent_hash_.to_string() << "',txhash='" << transaction_tree_hash_.to_string() << "',accounthash='" << account_tree_hash_.to_string() << "',total_coins=" << total_coins_ << ",close_time=" << close_time_ << " where ledger_sequence=" << ledger_sequence_ << ";";
		sql = out2.str ();
		std::cout << sql << std::endl;
		ret = ledger_db->exec_sql (sql, NULL);
		if (ret != SQLITE_OK){
			BUBI_LOG ("add ledger failed");
			return 1;
		}
	}	
	return 0;
}

RadixMerkleTree::ref
Ledger::get_account_tree (){
	return account_tree_;
}

RadixMerkleTree::ref
Ledger::get_transaction_tree (){
	return transaction_tree_;
}

int
Ledger::update_ledger_hash (){
	Serializer s;
	s.add256 (parent_hash_);
	s.add256 (transaction_tree_hash_);
	s.add256 (account_tree_hash_);
	s.add_raw ((char *)&total_coins_, sizeof (total_coins_));
	s.add_raw ((char *)&ledger_sequence_, sizeof (ledger_sequence_));
	s.add_raw ((char *)&close_time_, sizeof (close_time_));
	hash_ = s.get_sha512_half ();
}

int
Ledger::update_account_tree_hash (){
	account_tree_hash_ = account_tree_->get_hash ();
	update_ledger_hash ();
}
int
Ledger::update_transaction_tree_hash (){
	transaction_tree_hash_ = transaction_tree_->get_hash ();
	update_ledger_hash ();
}


int
Ledger::add_transaction_entry (Transaction::pointer tx){

	uint256& source_address = tx->get_source_address ();
	uint256& destination_address = tx->get_destination_address ();
	double	 amount = tx->get_payment_amount ();

	Account::pointer source_account = get_account_entry (source_address);
	Account::pointer destination_account = get_account_entry (destination_address);

	if (!source_account->add_balance (-amount))
		return 1;
	destination_account->add_balance (amount);

	std::uint32_t source_previous_ledger_seq = source_account->get_previous_ledger_seq ();
	std::uint32_t destination_previous_ledger_seq = destination_account->get_previous_ledger_seq ();

	uint256& source_tx_hash = source_account->get_previous_tx_hash ();
	uint256& destination_tx_hash = destination_account->get_previous_tx_hash ();

	std::cout << source_previous_ledger_seq << std::endl;
	std::cout << destination_previous_ledger_seq << std::endl;
	std::cout << source_tx_hash.to_string () << std::endl;
	std::cout << destination_tx_hash.to_string () << std::endl;
	
	tx->set_source_previous_ledger_seq (source_previous_ledger_seq);
	tx->set_source_previous_tx_hash (source_tx_hash);
	tx->set_destination_previous_ledger_seq (destination_previous_ledger_seq);
	tx->set_destination_previous_tx_hash (destination_tx_hash);

	Serializer s (tx->serializer ());
	uint256 hash = s.get_sha512_half ();
	RadixMerkleTreeLeaf::pointer item = std::make_shared<RadixMerkleTreeLeaf> (hash, s);
	transaction_tree_->add_item (item, true);
	update_transaction_tree_hash ();


	source_account->set_previous_ledger_seq (ledger_sequence_);
	source_account->set_previous_tx_hash (hash);

	destination_account->set_previous_ledger_seq (ledger_sequence_);
	destination_account->set_previous_tx_hash (hash);

	Serializer ss (source_account->serializer ());
	Serializer ds (destination_account->serializer ());
	RadixMerkleTreeLeaf::pointer s_item = std::make_shared<RadixMerkleTreeLeaf> (source_account->get_account_address (), ss);

	RadixMerkleTreeLeaf::pointer d_item = std::make_shared<RadixMerkleTreeLeaf> (destination_account->get_account_address (), ds);

	update_account_tree_entry (s_item);
	update_account_tree_entry (d_item);
	update_account_tree_hash ();
	return 0;
}

Account::pointer
Ledger::get_account_entry (uint256& hash){
	return	account_tree_->get_account_entry (hash); 
}

Transaction::pointer
Ledger::get_transaction_entry (uint256& hash){
	return transaction_tree_->get_transaction_entry (hash); 
}

bool 
Ledger::add_account_tree_entry (uint256 &tag, Account::pointer acc){
	Serializer s (acc->serializer ());
	RadixMerkleTreeLeaf::pointer item = std::make_shared <RadixMerkleTreeLeaf> (tag, s);
//	RadixMerkleTreeLeaf item (tag, s);
	account_tree_->add_item (item, false);
	update_account_tree_hash ();
	return true;
}

bool 
Ledger::has_account (uint256& hash){
	return account_tree_->has_item (hash);
}

bool
Ledger::update_account_tree_entry (RadixMerkleTreeLeaf::ref item){
	account_tree_->update_given_item (item, false);
	update_account_tree_hash ();
	return true;
}

}
