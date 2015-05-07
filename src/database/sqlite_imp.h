#ifndef _BUBI_SQLITE_IMP_H_
#define _BUBI_SQLITE_IMP_H_

#include <sqlite3.h>
#include <string.h>
#include <memory>
#include <mutex>

namespace Bubi {

typedef int (* Callback)(void *, int, char**, char**);

class SqliteImp {
public:
	typedef std::shared_ptr<SqliteImp>	pointer;

	SqliteImp (std::string name);
	~SqliteImp ();

	int exec_sql (std::string& sql, Callback f);

private:
	sqlite3* db_;
	std::string db_name_;
};

class SqliteInstance {
public:
	static void set_db_name (std::string& _name){
		db_name_ = _name;
	}
	static SqliteImp::pointer instance ();

private:
	SqliteInstance () {}
	static	std::string			db_name_;
	static	SqliteImp::pointer	sqlite_ptr_;
	static	std::mutex			sqlite_mutex_;
};

}

#endif

