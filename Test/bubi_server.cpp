#include <stdio.h>
#include <string.h>
#include <string>
#include <memory>
#include <assert.h>
#include <queue>

#include <sqlite3.h>
#include "sqlite_imp.h"
#include "rocksdb_imp.h"
#include "ledger.h"
#include "transaction.h"
#include "account.h"

using namespace Bubi;

std::string ledger_db_name = "/home/ivanzjj/bubi_ledger.db";
std::string radix_db_name = "/home/ivanzjj/radix_tree";

SqliteImp::pointer ledger_db;
Ledger::pointer	last_ledger = nullptr;

int create_if_not_exist (){
	std::string sql = "";
	int ret;

	sql = "create table BubiLedger("	\
		   "ledger_sequence	INT	PRIMARY KEY	NOT NULL," \
		   "hash	varchar(70),"	\
		   "phash	varchar(70),"	\
		   "txhash	varchar(70),"	\
		   "accounthash	varchar(70),"	\
		   "total_coins	INT,"	\
		   "close_time	INT);";
	ret = ledger_db->exec_sql (sql, NULL);
	if (ret != 0){
		BUBI_LOG ("table BubiLedger has exist");
	}
	else {
		BUBI_LOG ("create new table success");
	}
	return ret;
}

int char_to_uint32_t (std::uint32_t &val, const char* ch){
	std::istringstream scin (ch);
	scin >> val;
	return 0;
}

int load_last_ledger_call_back (void *data, int argc, char **argv, char **azColName){
	assert (argc == 7);
	std::uint32_t total_coin, ledger_sequence, close_time;
	uint256 hash, parent_hash, transaction_tree_hash, account_tree_hash;
	char_to_uint32_t (ledger_sequence, argv[0]);
	hash.binary_init (argv[1]);
	parent_hash.binary_init (argv[2]);
	transaction_tree_hash.binary_init (argv[3]);
	account_tree_hash.binary_init (argv[4]);
	char_to_uint32_t (total_coin, argv[5]);
	char_to_uint32_t (close_time, argv[6]);
	last_ledger = std::make_shared<Ledger> (hash, parent_hash, transaction_tree_hash, account_tree_hash, total_coin, ledger_sequence, close_time);	
	return 0;
}

int load_last_ledger (){
	std::string sql = "";
	int ret;

	sql = "select * from BubiLedger order by ledger_sequence desc limit 1;";
	ret = ledger_db->exec_sql (sql, &load_last_ledger_call_back);
	if (ret != 0){
		BUBI_LOG ("load last ledger failed");
		return 1;
	}
	if (!last_ledger){
		BUBI_LOG ("there is no ledger exist,then create a init ledger");
		last_ledger = std::make_shared<Ledger> ();
	}
	else {
		BUBI_LOG ("load ledger success");
	}
	return ret;
}

int recover_ledger (){
	ledger_db = SqliteInstance::instance ();
	create_if_not_exist();
	if (load_last_ledger ())
		return 1;	
}

int test (){
	uint256 hash;
	char hash_ch[32];
	for (int i = 0; i < 32; i++)	hash_ch[i] = i;
	hash.init (hash_ch);
	uint256 p_tx;
	p_tx.zero ();

	Account::pointer acc = std::make_shared<Account> (hash, 10, 0, p_tx);

	last_ledger->add_account_tree_entry (hash, acc);
	
	for (int i = 0; i < 32; ++i){
		hash_ch[i] = i;
	}
	hash_ch[0] = 1;
	hash.init (hash_ch);

	acc = std::make_shared <Account> (hash, 20, 0, p_tx);
	last_ledger->add_account_tree_entry (hash, acc);

	for (int i = 0; i < 32; i++)	hash_ch[i] = i;
	hash_ch[0] = 1; hash_ch[1] = 2;
	hash.init (hash_ch);

	acc = std::make_shared <Account> (hash, 30, 0, p_tx);
	last_ledger->add_account_tree_entry (hash, acc);
	
	for (int i = 0; i < 32; i++)	hash_ch[i] = i;
	hash.init (hash_ch);

	if (last_ledger->has_account (hash)){
		printf ("YES\n");
		acc = last_ledger->get_account_entry (hash);
		std::cout << acc->get_account_address ().to_string () << std::endl;
		std::cout << acc->get_account_balance () << std::endl;
		std::cout << acc->get_previous_ledger_seq () << std::endl;
		std::cout << acc->get_previous_tx_hash ().to_string () << std::endl;

		acc->add_balance (100000);
		Serializer ss (acc->serializer ());

		RadixMerkleTreeLeaf::pointer new_item = std::make_shared <RadixMerkleTreeLeaf> (hash, ss);
		last_ledger->update_account_tree_entry (new_item);
	}
}

void dfs (RadixMerkleTreeNode::pointer node, int tree_depth){
	if (!node->is_inner ()){
#if 0
		printf ("DEPTH:%d\n", tree_depth);
		printf ("INDEX: ");
		std::cout << node->peek_leaf ()->get_index ().to_string () << std::endl;
		printf ("SERI_DATA: ");
		node->peek_leaf ()->data_to_string ();
#endif
		Account::pointer acc = std::make_shared <Account> ();
		std::string str = node->peek_leaf ()->peek_string ();
		acc->unserializer (str);

		std::cout << "********************************************" << std::endl;
		std::cout << acc->get_account_address ().to_string () << std::endl;
		std::cout << acc->get_account_balance () << std::endl;
		std::cout << acc->get_previous_ledger_seq () << std::endl;
		std::cout << acc->get_previous_tx_hash ().to_string () << std::endl;
		return ;
	}
#if 0
	printf ("INNER::%d\n", tree_depth);
	printf ("HASH: ");
	std::cout << node->get_hash ().to_string () << std::endl;
#endif
	for (int i = 0; i < 16; i++){
		if (!node->is_empty_branch (i)){
			dfs (node->get_child (i), tree_depth + 1);
		}
	}
}

void fetch_from_db (){
	RadixMerkleTree::pointer tree = last_ledger->get_account_tree ();
	RadixMerkleTreeNode::pointer now, tmp;

	std::queue <RadixMerkleTreeNode::pointer> que;

	now = tree->get_root ();
	que.push (now);

	while (!que.empty()){
		now = que.front(); que.pop();
		for (int i = 0; i < 16; i++){
			if (!now->is_empty_branch (i)){
				tmp = tree->descend (now, i);
				que.push (tmp);
			}
		}
	}
	
	tree = last_ledger->get_transaction_tree ();
	while (!que.empty())	que.pop();
	now = tree->get_root ();
	que.push (now);

	while (!que.empty()){
		now = que.front(), que.pop();
		for (int i = 0; i < 16; i++){
			if (!now->is_empty_branch (i)){
				tmp = tree->descend (now, i);
				que.push (tmp);
			}
		}
	}

}

void create_transaction (){
	uint256 source, destination;
	char hash_ch[32];
	for (int i = 0; i < 32; i++)
		hash_ch[i] = i;
	source.init (hash_ch);

	for (int i = 0; i < 32; i++)	hash_ch[i] = i;
	hash_ch[0] = 1; hash_ch[1] = 2;
	destination.init (hash_ch);

	Transaction::pointer tx = std::make_shared <Transaction> (source, destination, 75);
	
	last_ledger->add_transaction_entry (tx);
}


void find_transaction_history (){
	uint256 hash;
	char hash_ch[32];
	for (int i = 0; i < 32; ++i)	hash_ch[i] = i;
	hash_ch[0] = 1; hash_ch[1] = 2;
	hash.init (hash_ch);

//	vector <Transaction::pointer> vet;
	Account::pointer acc = last_ledger->get_account_entry (hash);

	uint256 tx_hash;
	std::uint32_t	tx_seq;

	tx_seq = acc->get_previous_ledger_seq ();
	tx_hash = acc->get_previous_tx_hash ();


	while ( tx_seq != 0 ){
		Transaction::pointer tx = last_ledger->get_transaction_entry (tx_hash);
		std::cout << "------------------------------------" << std::endl;
		std::cout << tx->get_source_address ().to_string () << std::endl;
		std::cout << tx->get_destination_address ().to_string () << std::endl;
		std::cout << tx->get_payment_amount () << std::endl;

		if (hash == tx->get_source_address ()){
			tx_seq = tx->get_source_previous_ledger_seq ();
			tx_hash = tx->get_source_previous_tx_hash ();
		}
		else {
			tx_seq = tx->get_destination_previous_ledger_seq ();
			tx_hash = tx->get_destination_previous_tx_hash ();
		}
	}

}

int main (){
	RocksdbInstance::set_db_name (radix_db_name);
	SqliteInstance::set_db_name (ledger_db_name);
	if (recover_ledger ()){
		return 1;
	}
	
//	test ();	
	fetch_from_db ();
//	dfs (last_ledger->get_transaction_tree ()->get_root (), 0);
	dfs (last_ledger->get_account_tree ()->get_root (), 0);
//	create_transaction ();
//	std::cout << "**************After the transaction added to the ledger*******************" << std::endl;
//	dfs (last_ledger->get_account_tree ()->get_root (), 0);
//	dfs (last_ledger->get_transaction_tree ()->get_root (), 0);
	find_transaction_history ();

//	transaction_serializer_test ();	
//	account_serializer_test ();

	return 0;
}
