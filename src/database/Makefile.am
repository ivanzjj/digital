ROCKSDB := $(top_builddir)/src/rocksdb/librocksdb.so

$(ROCKSDB):
	@echo "******************Building rocksdb now*************" && cd $(@D) && pwd && $(MAKE) shared_lib && $(MAKE) install

SQLITEDB := $(top_builddir)/src/sqlite/.libs/libsqlite3.so
SQLITEDB_BUILD_PATH := $(top_builddir)/src/sqlite

$(SQLITEDB):
	@echo ******************Building sqlitedb now************* && cd $(SQLITEDB_BUILD_PATH) && $(MAKE) && $(MAKE) install


lib_LTLIBRARIES = libbubi_database.la

libbubi_database_la_CXXFLAGS =	-std=c++11	\
								-I$(top_builddir)/src/base	\
								-I$(top_builddir)/src/ledger	\
								-I$(top_builddir)/src/rocksdb/include	\
								-I$(top_builddir)/src/sqlite		\
								$(NULL)

libbubi_database_la_SOURCES =	encode_node.cpp	\
								rocksdb_imp.cpp	\
								sqlite_imp.cpp	\
							$(NULL)
	
libbubi_database_la_LDFLAGS =	\
							$(NULL)

libbubi_database_la_LIBADD = -lrocksdb	\
							 -lsqlite3	\
							 -lpthread	\
							 -lrt	\
							 -lz	\
							 $(ROCKSDB)	\
							 $(SQLITEDB)	\
							$(NULL)	

noinst_HEADERS =	database.h			\
					encode_node.h		\
					decode_node.h		\
					rocksdb_imp.h		\
					sqlite_imp.h		\
					$(NULL)

