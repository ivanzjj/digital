lib_LTLIBRARIES = libbubi_ledger.la

PTHREAD_LDFLAFS = -pthread

libbubi_ledger_la_SOURCES =  ledger.cpp	\
							radix_merkle_tree.cpp	\
							radix_merkle_tree_node.cpp	\
							radix_merkle_tree_leaf.cpp	\
							$(NULL)

libbubi_ledger_la_CXXFLAGS =  -I$(top_builddir)/src/base/	\
							 -I$(top_builddir)/src/database/	\
							 -std=c++11		\
							 $(NULL)

libbubi_ledger_la_LIBADD = $(top_builddir)/src/base/libbubi_base.la \
						  $(top_builddir)/src/database/libbubi_database.la	\
						$(NULL)

libbubi_ledger_la_LDFLAGS = $(PTHREAD_LDFLAGS)		\
							$(NULL)


noinst_HEADERS = ledger.h	\
				 radix_merkle_tree.h	\
				 radix_merkle_tree_node.h	\
				 radix_merkle_tree_leaf.h	\
				 $(NULL)
