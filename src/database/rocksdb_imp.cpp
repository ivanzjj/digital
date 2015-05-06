#include <unistd.h>

#include "rocksdb_imp.h"
#include "encode_node.h"
#include "decode_node.h"

namespace Bubi {

std::string RocksdbInstance::db_name_ = "";
std::shared_ptr<RocksdbImp> RocksdbInstance::rocksdb_ptr_ = nullptr;
std::mutex	RocksdbInstance::rocksdb_mutex_;


std::shared_ptr<RocksdbImp>
RocksdbInstance::instance (){
	std::unique_lock<std::mutex> lock (rocksdb_mutex_);
	if (!rocksdb_ptr_){
		printf ("rocksdbimp::instance first time\n");
		rocksdb_ptr_ = std::make_shared<RocksdbImp> (db_name_);
	}
	else {
		printf ("rocksdbimp::instance second time\n");
	}
	return rocksdb_ptr_;
}

RocksdbImp::RocksdbImp (std::string name)
:db_name_ (name) {
	rocksdb::Options options;

	options.create_if_missing = true;
	options.IncreaseParallelism ();
	options.OptimizeLevelStyleCompaction ();
	
	rocksdb::Status status = rocksdb::DB::Open (options, db_name_, &db_);
	assert (status.ok());
//	db_.reset (db);
	BUBI_LOG ("rocksdb open success");
}

RocksdbImp::~RocksdbImp (){
	printf ("RocksdbImp done!\n");
	rocksdb::Status sss = db_->Put (rocksdb::WriteOptions (), "asdaf", "sdfsafdsaf");
	assert (sss.ok ());
//	delete db_;
//	db_ = NULL;
	BUBI_LOG ("rocksdb close success");
}

RadixMerkleTreeNode::pointer
RocksdbImp::fetch (uint256 &hash){
	std::size_t	key_size = hash.get_bytes ();
	char* key = hash.begin ();
	rocksdb::Slice slice (key, key_size);
	std::string value_string;

	rocksdb::Status status = db_->Get (rocksdb::ReadOptions (), slice, &value_string );
	RadixMerkleTreeNode::pointer read_node;

	if (status.ok()){
		db_->Delete (rocksdb::WriteOptions (), slice);
		read_node = std::make_shared<RadixMerkleTreeNode> ();
		read_node->set_hash (hash);
		read_node->decode (value_string);
	}
	else{
		assert (false);
	}
	return read_node;
}

void
RocksdbImp::store (RadixMerkleTreeNode::pointer node){
	Batch batch = {node};
	store_batch (batch);
}

void
RocksdbImp::store_batch (Batch &batch){
	rocksdb::WriteBatch wbatch;
	rocksdb::Status status;
	EncodeNode encode;

	for (auto e : batch){
		encode.do_encode (e);
		wbatch.Put (
			rocksdb::Slice(encode.get_key(), encode.get_key_size()),
			rocksdb::Slice(encode.get_value(), encode.get_value_size())
				);
	}
	status = db_->Write (rocksdb::WriteOptions(), &wbatch);
	assert (status.ok());
}

}
