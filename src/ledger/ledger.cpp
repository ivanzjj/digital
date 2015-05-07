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
	parent_hash_.zero ();
	transaction_tree_hash_.zero ();
	account_tree_hash_.zero ();
	total_coins_ = 0;
	ledger_sequence_ = 0;
	close_time_ = 0;
	transaction_tree_ = std::make_shared <RadixMerkleTree> (false);
	account_tree_ = std::make_shared <RadixMerkleTree> (true);
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
	ret = ledger_db->exec_sql (sql, NULL);
	if (ret != SQLITE_OK){
		std::ostringstream out2;

		out2 << "update BubiLedger set hash='" << hash_.to_string()<< "',phash='" << parent_hash_.to_string() << "',txhash='" << transaction_tree_hash_.to_string() << "',accounthash='" << account_tree_hash_.to_string() << "',total_coins=" << total_coins_ << ",close_time=" << close_time_ << " where ledger_sequence=" << ledger_sequence_ << ";";
		sql = out2.str ();
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
Ledger::update_account_tree_hash (){
	account_tree_hash_ = account_tree_->get_hash ();
}

bool 
Ledger::add_account_tree_entry (uint256 &tag, Serializer &s){
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
	return account_tree_->update_given_item (item, false);
}

}
