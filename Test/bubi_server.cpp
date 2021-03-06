#include <stdio.h>
#include <string.h>
#include <string>
#include <memory>
#include <assert.h>
#include <queue>
#include <vector>
#include <signal.h>
#include <sys/time.h>
#include <thread>

#include <sqlite3.h>
#include "sqlite_imp.h"
#include "rocksdb_imp.h"
#include "ledger.h"
#include "transaction.h"
#include "account.h"
#include "net.h"
#include "netbase.h"
#include "interface.h"

using namespace Bubi;

std::string ledger_db_name = "/var/bubi_ledger.db";
std::string radix_db_name = "/var/radix_tree";

SqliteImp::pointer ledger_db;
Ledger::pointer	Bubi::last_ledger = nullptr;

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
	Bubi::last_ledger = std::make_shared<Ledger> (hash, parent_hash, transaction_tree_hash, account_tree_hash, total_coin, ledger_sequence, close_time);	
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
	if (!Bubi::last_ledger){
		BUBI_LOG ("there is no ledger exist,then create a init ledger");
		Bubi::last_ledger = std::make_shared<Ledger> ();
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

	Bubi::last_ledger->add_account_tree_entry (hash, acc);
	
	for (int i = 0; i < 32; ++i){
		hash_ch[i] = i;
	}
	hash_ch[0] = 1;
	hash.init (hash_ch);

	acc = std::make_shared <Account> (hash, 20, 0, p_tx);
	Bubi::last_ledger->add_account_tree_entry (hash, acc);

	for (int i = 0; i < 32; i++)	hash_ch[i] = i;
	hash_ch[0] = 1; hash_ch[1] = 2;
	hash.init (hash_ch);

	acc = std::make_shared <Account> (hash, 30, 0, p_tx);
	Bubi::last_ledger->add_account_tree_entry (hash, acc);
	
	for (int i = 0; i < 32; i++)	hash_ch[i] = i;
	hash.init (hash_ch);

	if (Bubi::last_ledger->has_account (hash)){
		printf ("YES\n");
		acc = Bubi::last_ledger->get_account_entry (hash);
		std::cout << acc->get_account_address ().to_string () << std::endl;
		std::cout << acc->get_account_balance () << std::endl;
		std::cout << acc->get_previous_ledger_seq () << std::endl;
		std::cout << acc->get_previous_tx_hash ().to_string () << std::endl;

		acc->add_balance (100000);
		Serializer ss (acc->serializer ());

		RadixMerkleTreeLeaf::pointer new_item = std::make_shared <RadixMerkleTreeLeaf> (hash, ss);
		Bubi::last_ledger->update_account_tree_entry (new_item);
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

		std::cout << acc->get_account_address ().to_string () << "::";
		std::cout << acc->get_account_balance () << std::endl;
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
	RadixMerkleTree::pointer tree = Bubi::last_ledger->get_account_tree ();
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
	
	tree = Bubi::last_ledger->get_transaction_tree ();
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

int create_transaction (std::string source_addr, std::string det_addr, double amount){
	Serializer ss (source_addr);
	uint256 source = ss.get_sha512_half ();
	Serializer ds (det_addr);
	uint256 det = ds.get_sha512_half ();
	Transaction::pointer tx = std::make_shared <Transaction> (source, det, amount);
	return Bubi::last_ledger->add_transaction_entry (tx);
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
	Bubi::last_ledger->add_account_tree_entry (hash, acc);
	return 0;
}

double get_balance (std::string acc_pub){
	uint256 hash = string_address_to_uint256 (acc_pub);
	Account::pointer acc = Bubi::last_ledger->get_account_entry (hash);
	return acc->get_account_balance ();
}

std::vector <Transaction::pointer>	get_transaction_history (std::string acc_pub){
	uint256 hash = string_address_to_uint256 (acc_pub);
	Account::pointer acc = Bubi::last_ledger->get_account_entry (hash);
	std::vector <Transaction::pointer> res;

	uint256 tx_hash;
	std::uint32_t	tx_seq;

	tx_seq = acc->get_previous_ledger_seq ();
	tx_hash = acc->get_previous_tx_hash ();


	while ( tx_seq != 0 ){
		Transaction::pointer tx = Bubi::last_ledger->get_transaction_entry (tx_hash);
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


void create_transaction2 (){
	uint256 source, destination;
	char hash_ch[32];
	for (int i = 0; i < 32; i++)
		hash_ch[i] = i;
	source.init (hash_ch);

	for (int i = 0; i < 32; i++)	hash_ch[i] = i;
	hash_ch[0] = 1; hash_ch[1] = 2;
	destination.init (hash_ch);

/*
	if (create_transaction (source, destination, 200)){
		printf ("transaction has failed!\n");
	}
	else {
		printf ("transaction is ok\n");
	}
*/
}


void find_transaction_history (){
	uint256 hash;
	char hash_ch[32];
	for (int i = 0; i < 32; ++i)	hash_ch[i] = i;
	hash_ch[0] = 1; hash_ch[1] = 2;
	hash.init (hash_ch);

//	vector <Transaction::pointer> vet;
	Account::pointer acc = Bubi::last_ledger->get_account_entry (hash);

	uint256 tx_hash;
	std::uint32_t	tx_seq;

	tx_seq = acc->get_previous_ledger_seq ();
	tx_hash = acc->get_previous_tx_hash ();


	while ( tx_seq != 0 ){
		Transaction::pointer tx = Bubi::last_ledger->get_transaction_entry (tx_hash);
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


void auto_print_account_balance (int sig){
	std::cout << "**********************************" << std::endl;
	dfs (Bubi::last_ledger->get_account_tree ()->get_root (), 0);
	std::cout << "**********************************" << std::endl;
}

int setup_timer (){
	signal (SIGALRM, auto_print_account_balance);
	
	struct itimerval tick;
	memset (&tick, 0, sizeof (tick));
	tick.it_value.tv_sec = 5;
	tick.it_value.tv_usec = 0;
	tick.it_interval.tv_sec = 5;
	tick.it_interval.tv_usec = 0;

	int res = setitimer (ITIMER_REAL, &tick, NULL);
	if (res){
		printf ("setup timer failed!\n");
		return 1;
	}
}

void shut_down_call_back (int sig){
	BUBI_LOG ("shut down the system......");
	exit (0);
}

int init (){
	signal (SIGINT, shut_down_call_back);
	RocksdbInstance::set_db_name (radix_db_name);
	SqliteInstance::set_db_name (ledger_db_name);
	if (recover_ledger ()){
		return 1;
	}
	fetch_from_db ();
	setup_timer ();
	return 0;
}

int main (){
	if (init ()){
		printf ("init error!\n");
		return 1;
	}

	Discover();
	for (auto &addr : hostAddr) {
		BService bService(addr, ntohs(30000));
		BindListenPort(bService);
	}
    std::thread thread1(&ThreadSocketHandler);
	std::thread thread2(&ThreadOpenConnections);
	std::thread thread3(&ThreadMessageHandler);
	thread1.join();
	thread2.join();
	thread3.join();

//	test ();	
//	dfs (last_ledger->get_transaction_tree ()->get_root (), 0);
//	dfs (last_ledger->get_account_tree ()->get_root (), 0);
//	create_transaction2 ();
//	std::cout << "**************After the transaction added to the ledger*******************" << std::endl;
//	dfs (last_ledger->get_account_tree ()->get_root (), 0);
//	dfs (last_ledger->get_transaction_tree ()->get_root (), 0);
//	find_transaction_history ();

//	transaction_serializer_test ();	
//	account_serializer_test ();

	return 0;
}
