#include <stdio.h>
#include <string.h>
#include <string>
#include <memory>
#include <assert.h>

#include <sqlite3.h>
#include "sqlite_imp.h"
#include "rocksdb_imp.h"
#include "ledger.h"


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
	hash.init (argv[1]);
	parent_hash.init (argv[2]);
	transaction_tree_hash.init (argv[3]);
	account_tree_hash.init (argv[4]);
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
		last_ledger = std::make_shared<Ledger> ();
	}
	return ret;
}

int recover_ledger (){
	ledger_db = SqliteInstance::instance ();
	create_if_not_exist();
	if (load_last_ledger ())
		return 1;

	//TODO
}

int main (){
	RocksdbInstance::set_db_name (radix_db_name);
	SqliteInstance::set_db_name (ledger_db_name);
	if (recover_ledger ()){
		return 1;
	}
	return 0;
}
