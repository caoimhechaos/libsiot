TESTS=			unixsocketconnection_test server_test	\
			linebufferdecorator_test		\
			opensslconnection_test			\
			rangereaderdecorator_test		\
			acknowledgementdecorator_test
check_PROGRAMS=		${TESTS}
noinst_HEADERS=		opensslconnection.h unixsocketconnection.h
lib_LTLIBRARIES=	libsiot.la

libsiot_la_SOURCES=	server.cc unixsocketconnection.cc	\
			linebufferdecorator.cc sslcontext.cc	\
			acknowledgementdecorator.cc		\
			rangereaderdecorator.cc			\
			opensslconnection.cc
libsiot_la_LDFLAGS=	-version-info ${LIBRARY_VERSION}
libsiot_la_LIBADD=	${AC_LIBS}

CLEANFILES=		test.crt test.key
SUBDIRS=		siot

${TESTS}:	LDADD="${AC_LIBS} ${GTEST_LIBS} ${lib_LTLIBRARIES}"
${TESTS}:	test.key test.crt

test.key test.crt:
	openssl req -new -newkey rsa:1024 -x509 -out test.crt -keyout test.key -nodes -subj "/CN=test-dont-use-ever.local"
