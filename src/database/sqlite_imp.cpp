#include "sqlite_imp.h"
#include "utils.h"

#include <assert.h>


namespace Bubi {

std::string	SqliteInstance::db_name_ = "";
SqliteImp::pointer	SqliteInstance::sqlite_ptr_ = nullptr;
std::mutex	SqliteInstance::sqlite_mutex_;

SqliteImp::pointer
SqliteInstance::instance (){
	std::unique_lock<std::mutex> lock (sqlite_mutex_);
	if (sqlite_ptr_)
		return sqlite_ptr_;
	printf ("first time!!!\n");
	sqlite_ptr_ = std::make_shared <SqliteImp> (db_name_);
	return sqlite_ptr_;
}

SqliteImp::SqliteImp (std::string name)
:db_name_ (name) {
	int ret;
	ret = sqlite3_open (db_name_.c_str(), &db_);
	assert (ret == 0);
	BUBI_LOG ("sqlite database open success");
}

SqliteImp::~SqliteImp (){
	printf ("sqlite db close\n");
	sqlite3_close (db_);	
}

int
SqliteImp::exec_sql (std::string& sql, Callback f){
	int ret;
	char *err = 0;
	ret = sqlite3_exec (db_, sql.c_str(), f, 0, &err);
	if (ret != SQLITE_OK)
		return 1;
	else
		return 0;
}

}
