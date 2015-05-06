#ifndef _BUBI_DATABASE_H_
#define _BUBI_DATABASE_H_

#include <memory>
#include <vector>

#include <stdio.h>

#include "radix_merkle_tree_node.h"
#include "utils.h"

namespace Bubi {

typedef std::vector<RadixMerkleTreeNode::pointer>	Batch;

class DataBase {
public:
	typedef std::shared_ptr <DataBase>	pointer;
	DataBase () {}
	virtual ~DataBase () {}

	virtual RadixMerkleTreeNode::pointer fetch (uint256 &hash) = 0;
	virtual void	store (RadixMerkleTreeNode::pointer node) = 0;
	virtual void	store_batch (Batch &Batch) = 0;
};

}
#endif

