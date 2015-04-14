noinst_PROGRAMS = main

main_SOURCES = main.cpp

main_CXXFLAGS = -std=c++11	\
				-I$(top_builddir)/src/ledger/	\
				-I$(top_builddir)/src/base/		\
				-I$(top_builddir)/src/database	\
				$(NULL)

main_LDADD = $(top_builddir)/src/ledger/libbubi_ledger.la	\
			 $(top_builddir)/src/base/libbubi_base.la	\
			 $(top_builddir)/src/database/libbubi_database.la	\
			 $(NULL)
