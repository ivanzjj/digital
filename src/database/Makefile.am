ROCKSDB := $(top_builddir)/src/rocksdb/librocksdb.so

$(ROCKSDB):
	@echo "******************Building rocksdb now*************" && cd $(@D) && pwd && $(MAKE) shared_lib && $(MAKE) install 

lib_LTLIBRARIES = libbubi_database.la

libbubi_database_la_CXXFLAGS =	-std=c++11	\
								-I$(top_builddir)/src/base	\
								-I$(top_builddir)/src/ledger	\
								-I$(top_builddir)/src/rocksdb/include	\
								$(NULL)

libbubi_database_la_SOURCES =	encode_node.cpp	\
								rocksdb_imp.cpp	\
							$(NULL)
	
libbubi_database_la_LDFLAGS =	\
							$(NULL)

libbubi_database_la_LIBADD = -lrocksdb	\
							 -lpthread	\
							 -lrt	\
							 -lz	\
							 $(ROCKSDB)	\
							$(NULL)	

noinst_HEADERS =	database.h			\
					encode_node.h		\
					decode_node.h		\
					rocksdb_imp.h		\
					$(NULL)

