noinst_PROGRAMS = bubi_server

bubi_server_SOURCES = bubi_server.cpp
bubi_server_CXXFLAGS = -std=c++11		\
					  -I$(top_builddir)/src/ledger		\
					  -I$(top_builddir)/src/base		\
					  -I$(top_builddir)/src/database	\
					  -I$(top_builddir)/src/p2p			\
					  $(NULL)

bubi_server_LDADD = $(top_builddir)/src/database/libbubi_database.la	\
					$(top_builddir)/src/ledger/libbubi_ledger.la	\
					$(top_builddir)/src/base/libbubi_base.la		\
					$(top_builddir)/src/p2p/libbubi_network.la		\
					$(NULL)
