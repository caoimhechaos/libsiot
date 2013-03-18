TESTS=			unixsocketconnection_test server_test	\
			linebufferdecorator_test
check_PROGRAMS=		${TESTS}
lib_LTLIBRARIES=	libsiot.la

libsiot_la_SOURCES=	server.cc unixsocketconnection.cc	\
			linebufferdecorator.cc sslcontext.cc
libsiot_la_LDFLAGS=	-version-info ${LIBRARY_VERSION}
libsiot_la_LIBADD=	${AC_LIBS}

CLEANFILES=
SUBDIRS=		siot

${TESTS}:	LDADD="${AC_LIBS} ${GTEST_LIBS} ${lib_LTLIBRARIES}"
