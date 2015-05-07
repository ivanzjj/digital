#include <stdio.h>
#include <memory>
#include <string>
#include <cstring>

#include "ledger.h"
#include "utils.h"
#include "serializer.h"
#include "rocksdb_imp.h"
#include "sqlite_imp.h"

using namespace Bubi;

void dfs (RadixMerkleTreeNode::pointer node, int tree_depth){
//	node->get_hash ().to_string ();	
	if (!node->is_inner ()){
		printf ("DEPTH: %d\n", tree_depth);
		printf ("INDEX: ");
		node->peek_leaf ()->get_index ().to_string ();
		printf ("SERI_DATA: ");
		node->peek_leaf ()->data_to_string ();
		return;
	}
	for (int i = 0; i < 16; i++){
		if (!node->is_empty_branch (i)){
			dfs (node->get_child (i), tree_depth+1);
		}
	}
}

int main (){
	std::string db_name = "/home/ivanzjj/radix_tree";
	RocksdbInstance::set_db_name (db_name);
	Ledger::pointer ledger = std::make_shared <Ledger> ();
	
	std::string db_name2 = "/home/ivanzjj/ledger.db";
	SqliteInstance::set_db_name (db_name2);

	uint256 hash;
	char hash_ch[32];
	for (int i=0;i<32;i++)
		hash_ch[i] = i;
	hash.init (hash_ch);
	hash.to_string ();

	Serializer ss;
	char ch[100];
	for (int i=0;i<100;i++)	ch[i] = i;
	ss.add_raw (ch, 100);

	if (!ledger->add_account_tree_entry (hash, ss)){
		printf ("add_account_tree_entry error!\n");
		return 1;
	}
//	dfs (ledger->get_account_tree ()->get_root (), 0);

	for (int i = 0; i < 32; i++){
		hash_ch[i] = i;
	}
	hash_ch[0] = 1;
	hash.init (hash_ch);
	hash.to_string ();
	if (!ledger->add_account_tree_entry (hash, ss)){
		printf ("add_account_tree_entry error2\n");
		return 1;
	}
//	dfs (ledger->get_account_tree ()->get_root (), 0);

	
	for (int i = 0; i < 32; i++)	hash_ch[i] = i;
	hash_ch[0] = 1;
	hash_ch[1] = 2;

	hash.init (hash_ch);
	hash.to_string ();
	if (!ledger->add_account_tree_entry (hash, ss)){
		printf ("add_account_tree_entry error2\n");
		return 1;
	}
//	dfs (ledger->get_account_tree ()->get_root (), 0);
	
	for (int i = 0; i < 32; i++){
		hash_ch[i] = i;
	}
	hash.init (hash_ch);
	if (ledger->has_account (hash)){
		hash.to_string ();
		printf ("YES\n");
	
		ch[0] = 10;
		ss.peek_data ().clear();
		ss.add_raw (ch, 100);
		RadixMerkleTreeLeaf::pointer new_item = std::make_shared<RadixMerkleTreeLeaf> (hash, ss);
		if (!ledger->update_account_tree_entry (new_item)){
			printf ("update_account_tree_entry error!\n");
			return 1;
		}
	}
	else {
		printf ("NO\n");
	}	
//	dfs (ledger->get_account_tree ()->get_root (), 0);
	return 0;
}
