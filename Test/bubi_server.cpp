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

#if 0
int test (){
	uint256 hash;
	char hash_ch[32];
	for (int i = 0; i < 32; i++)	hash_ch[i] = i;
	hash.init (hash_ch);
	
	Serializer ss;
	char ch[100];
	for (int i = 0; i < 100; i++){
		ch[i] = i;
	}
	ss.add_raw (ch, 100);
	last_ledger->add_account_tree_entry (hash, ss);
	
	for (int i = 0; i < 32; ++i){
		hash_ch[i] = i;
	}
	hash_ch[0] = 1;
	hash.init (hash_ch);
	last_ledger->add_account_tree_entry (hash, ss);

	for (int i = 0; i < 32; i++)	hash_ch[i] = i;
	hash_ch[0] = 1; hash_ch[1] = 2;
	hash.init (hash_ch);
	last_ledger->add_account_tree_entry (hash, ss);
	
	for (int i = 0; i < 32; i++)	hash_ch[i] = i;
	hash.init (hash_ch);
	if (last_ledger->has_account (hash)){
		printf ("YES\n");
		
		ch[0] = 10;
		ss.peek_data().clear();
		ss.add_raw (ch, 100);
		RadixMerkleTreeLeaf::pointer new_item = std::make_shared <RadixMerkleTreeLeaf> (hash, ss);
		last_ledger->update_account_tree_entry (new_item);
	}
}
#endif

void dfs (RadixMerkleTreeNode::pointer node, int tree_depth){
	if (!node->is_inner ()){
		printf ("DEPTH:%d\n", tree_depth);
		printf ("INDEX: ");
		std::cout << node->peek_leaf ()->get_index ().to_string () << std::endl;
		printf ("SERI_DATA: ");
		node->peek_leaf ()->data_to_string ();
		return ;
	}
	printf ("INNER::%d\n", tree_depth);
	printf ("HASH: ");
	std::cout << node->get_hash ().to_string () << std::endl;
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
}


/***
  transaction test
***/
#if 0
void transaction_serializer_test (){
	uint256 source;
	char hash_ch[32];
	for (int i = 0; i < 32; i++){
		hash_ch[i] = i;
	}
	source.init (hash_ch);

	uint256 destination;
	for (int i =  0; i < 32; i++)
		hash_ch[i] = 32 - i;
	destination.init (hash_ch);

	uint256 sp_ledger, sp_tx, dp_ledger, dp_tx;
	sp_ledger.zero (); sp_tx.zero ();
	dp_ledger.zero (); dp_tx.zero ();

	Transaction::pointer tx = std::make_shared<Transaction> (source, destination, 20.0, sp_ledger, sp_tx, dp_ledger, dp_tx);

	std::string res = tx->serializer ();
	std::cout << res << std::endl;

	Transaction::pointer tx2 = std::make_shared<Transaction> ();
	tx2->unserializer (res);

	std::cout << "***********OUT***************" << std::endl;

	std::cout << source.to_string () << std::endl;
	std::cout << tx2->get_source_address ().to_string () << std::endl;
	std::cout << destination.to_string () << std::endl;
	std::cout << tx2->get_destination_address ().to_string ()  << std::endl;
	std::cout << tx2->get_payment_amount () << std::endl;

	std::cout << tx2->get_source_previous_ledger_hash ().to_string () << std::endl;
	std::cout << tx2->get_source_previous_tx_hash ().to_string () << std::endl;
	std::cout << tx2->get_destination_previous_ledger_hash ().to_string () << std::endl;
	std::cout << tx2->get_destination_previous_tx_hash ().to_string () << std::endl;

}
#endif

#if 0
void account_serializer_test (){
	uint256 address;
	char hash_ch[32];
	for (int i = 0; i < 32; i++){
		hash_ch[i] = i;
	}
	address.init (hash_ch);

	uint256 previous;
	for (int i = 0; i < 32; i++)
		hash_ch[i] = 32 - i;
	previous.init (hash_ch);
	
	uint256 p_tx;
	p_tx.zero ();

	Account::pointer acc = std::make_shared <Account> (address, 20, previous, p_tx);
	std::string res = acc->serializer ();

	std::cout << "******************** ACCOUNT OUT ***************" << std::endl;
	std::cout << res << std::endl;
	
	Account::pointer acc2 = std::make_shared <Account> ();
	acc2->unserializer (res);

	std::cout << address.to_string () << std::endl;
	std::cout << acc2->get_account_address ().to_string () << std::endl;
	std::cout << acc2->get_account_balance () << std::endl;
	std::cout << previous.to_string () << std::endl;
	std::cout << acc2->get_previous_ledger_hash ().to_string () << std::endl;
	std::cout << acc2->get_previous_tx_hash ().to_string () << std::endl;

}
#endif

int main (){
	RocksdbInstance::set_db_name (radix_db_name);
	SqliteInstance::set_db_name (ledger_db_name);
	if (recover_ledger ()){
		return 1;
	}
	
//	test ();	
	fetch_from_db ();
	dfs (last_ledger->get_account_tree ()->get_root (), 0);

//	transaction_serializer_test ();	
//	account_serializer_test ();

	return 0;
}
