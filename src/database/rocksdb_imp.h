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
	typedef std::shared_ptr <RocksdbImp>	pointer;

public:
	RocksdbImp (std::string name);
	~RocksdbImp ();

	RadixMerkleTreeNode::pointer	fetch (uint256 &hash);
	void	store (RadixMerkleTreeNode::pointer node);
	void	store_batch (Batch &batch);

private :
	std::unique_ptr <rocksdb::DB>	db_;
	std::string						db_name_;
}

}
#endif
