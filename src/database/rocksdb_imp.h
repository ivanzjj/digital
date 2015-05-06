#ifndef _BUBI_ROCKSDB_IMP_H_
#define _BUBI_ROCKSDB_IMP_H_

#include <memory>
#include <string>

#include "rocksdb/db.h"
#include "rocksdb/slice.h"
#include "rocksdb/options.h"

#include "database.h"
#include "radix_merkle_tree_node.h"
#include <utils.h>

namespace Bubi {

class RocksdbImp
	: public DataBase {
public:
	typedef std::shared_ptr <RocksdbImp>	pointer;
	RocksdbImp (std::string name);
	~RocksdbImp ();

	RadixMerkleTreeNode::pointer	fetch (uint256 &hash);
	void	store (RadixMerkleTreeNode::pointer node);
	void	store_batch (Batch &batch);

private :
	std::unique_ptr <rocksdb::DB>	db_;
	std::string db_name_;
};

class RocksdbInstance {
public:
	static void set_db_name (std::string& _name){
		db_name_ = _name;
	}
	static RocksdbImp::pointer instance ();

private:
	RocksdbInstance ();
	static std::string		db_name_;
	static std::shared_ptr<RocksdbImp>	rocksdb_ptr_;
	static std::mutex		rocksdb_mutex_;
};
}
#endif
