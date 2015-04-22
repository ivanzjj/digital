#include "rocksdb_imp.h"

namespace Bubi {

RocksdbImp::RocksdbImp (std::string name)
:db_name_(name) {
	
	rocksdb::DB* db;
	rocksdb::Options options;

	options.create_if_missing = true;
	options.IncreaseParallelism ();
	options.OptimizeLevelStyleCompaction ();
	
	rocksdb::Status status = rocksdb::DB::Open (options, db_name_, &db);
	assert (status.ok());
	db_.reset (db);
	BUBI_LOG ("rocksdb open success");
}

RocksdbImp::~RocksdbImp (){
	db_.reset ();
	BUBI_LOG ("rocksdb close success");
}

RadixMerkleTreeNode::pointer
RocksdbImp::fetch (uint256 &hash){
	std::size_t	key_size = hash.get_bytes ();
	char* key = hash.begin ();
	rocksdb::Slice slice (key, key_size);
	std::string value_string;

	rocksdb::Status status = db_->Get (ReadOptions (), slice, value_string );
	RadixMerkleTreeNode::pointer read_node;

	if (status.ok()){
		read_node = std::make_shared<RadixMerkleTreeNode> ();
		read_node->set_child (hash);
		read_node->decode (value_string);
	}
	else{
		assert (false);
	}
	return read_node;
}

void
RocksdbImp::store (RadixMerkleTreeNode::pointer node){
	Batch batch;
	batch.push_back (node);
	store_batch (batch);
}

void
RocksdbImp::store_batch (Batch &batch){
	rocksdb::WriteBatch batch;
	rocksdb::Status status;
	EncodeNode encode;

	for (auto e : batch){
		encode.do_encode (e);
		batch.Put (
			rocksdb::Slice(encode.get_key(), encode.get_key_size()),
			rocksdb::Slice(encode.get_value(), encode.get_value_size())
				);
	}
	status = db_->Write (WriteOptions(), &batch);
	assert (status.ok());
}

}
