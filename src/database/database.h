#ifndef _BUBI_DATABASE_H_
#define _BUBI_DATABASE_H_

#include <memory>

#include <stdio.h>

#include "radix_merkle_tree_node.h"
#include "utils.h"

namespace Bubi {

class DataBase {
	typedef std::shared_ptr <DataBase>	pointer;
	
public:
	DataBase ();
	~DataBase ();

	RadixMerkleTreeNode::pointer fetch (uint256 &);

};

}
#endif

