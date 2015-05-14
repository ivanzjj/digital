lib_LTLIBRARIES = libbubi_network.la
libbubi_network_la_CXXFLAGS = -std=c++11	\
							  -I$(top_builddir)/src/ledger		\ 
							  -I$(top_builddir)/src/base        \
						      -I$(top_builddir)/src/database    \
							  $(NULL)

libbubi_network_la_SOURCES = net.cpp netbase.cpp bubi.pb.cc interface.cpp

libbubi_network_la_LIBADD = -lpthread \
							-lprotobuf \
							-ljson \
							$(top_builddir)/src/database/libbubi_database.la	\   
							$(top_builddir)/src/ledger/libbubi_ledger.la		\   
					        $(top_builddir)/src/base/libbubi_base.la        

							
noinst_HEADERS = net.h netbase.h bubi.pb.h interface.h

