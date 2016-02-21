# AM_CPPFLAGS = $(INCLUDE_DIRS)

bin_PROGRAMS = ggpserver externalserver

SERVER_SOURCES = 	../utils/childprocess.cpp ../utils/childprocess.h \
		 	../utils/fdstream.h

ggpserver_SOURCES = 	ggpserver.cpp \
			server.cpp server.h \
			$(SERVER_SOURCES)

externalserver_SOURCES = externalserver.cpp \
			../utils/tcpserver.cpp ../utils/tcpserver.h \
			$(SERVER_SOURCES)

# ggpserver_LDADD = ${BOOST_SYSTEM_LIB}
# externalserver_LDADD = ${BOOST_SYSTEM_LIB}
