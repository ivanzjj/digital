PROTOBUF := $(top_builddir)/src/protobuf/src/.libs/libprotobuf.so
PROTOBUF_PATH := $(top_builddir)/src/protobuf

$(PROTOBUF) :
	@echo "*************building protobuf now***************" && cd $(PROTOBUF_PATH) && $(MAKE) && $(MAKE) install

lib_LTLIBRARIES = libbubi_ledger.la

PTHREAD_LDFLAFS = -pthread

libbubi_ledger_la_SOURCES = ledger.cpp	\
							radix_merkle_tree.cpp	\
							radix_merkle_tree_node.cpp	\
							radix_merkle_tree_leaf.cpp	\
							transaction.cpp		\
							account.cpp		\
							transaction.pb.cc	\
							$(NULL)

libbubi_ledger_la_CXXFLAGS =  -I$(top_builddir)/src/base/	\
							 -I$(top_builddir)/src/database/	\
							 -I$(top_builddir)/src/protobuf/src/	\
							 -std=c++11		\
							 $(NULL)

libbubi_ledger_la_LIBADD =  $(PROTOBUF)			\
							$(top_builddir)/src/base/libbubi_base.la \
							$(top_builddir)/src/database/libbubi_database.la	\
							-lpthread	\
							-lprotobuf		\
							$(NULL)

libbubi_ledger_la_LDFLAGS = $(PTHREAD_LDFLAGS)		\
							$(NULL)


noinst_HEADERS = ledger.h	\
				 radix_merkle_tree.h	\
				 radix_merkle_tree_node.h	\
				 radix_merkle_tree_leaf.h	\
				 transaction.h	\
				 account.h	\
				 transaction.pb.h	\
				 $(NULL)
