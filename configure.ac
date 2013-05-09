#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.63])
AC_INIT([libsiot], [0.4.1], [caoimhechaos@protonmail.com])
AC_CONFIG_SRCDIR([server.cc])
AC_CONFIG_HEADERS([config.h])

AM_INIT_AUTOMAKE
LT_INIT

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_PROG_LIBTOOL
AC_LANG(C++)

LIBRARY_VERSION=`echo $VERSION | $SED -e's/^0\.//g' -e's/\./:/g'`
AC_SUBST(LIBRARY_VERSION)

OLDCXXFLAGS="$CXXFLAGS"
CXXFLAGS="$CXXFLAGS -std=c++11"
AC_MSG_CHECKING([whether $CXX accepts -std=c++11])
AC_LINK_IFELSE([AC_LANG_CALL([], [main])], [AC_MSG_RESULT(yes)],
	[AC_MSG_RESULT(no)
	 CXXFLAGS="$OLDCXXFLAGS -std=c++0x"
	 AC_MSG_CHECKING([whether $CXX accepts -std=c++0x])
	 AC_LINK_IFELSE([AC_LANG_CALL([], [main])], [AC_MSG_RESULT(yes)],
	 [AC_MSG_RESULT(no); CXXFLAGS="$OLDCXXFLAGS"])])
unset OLDCXXFLAGS
AC_SUBST(CXXFLAGS)

# Hack to make libclib work under C++.
AC_DEFINE(HAVE_CLIB_HASH_H, [1], [Hack to make clib not include hash.h])
AC_DEFINE(_GLIBCXX_USE_C99, [1], [Use C99 for the glibc++ parts.])

# Checks for libraries.
GTEST_LIBS=""
QT_LIBS=""
OPENSSL="1"
AC_CHECK_LIB([thread++], [main],
	     [AC_LIBS="$AC_LIBS -lthread++"],
	     [AC_ERROR(libthread++ is required)])
AC_CHECK_LIB([toolbox], [main],
	     [AC_LIBS="$AC_LIBS -ltoolbox"],
	     [AC_ERROR(libtoolbox is required)])
AC_CHECK_LIB([clib], [c_sockaddr2str],
	     [AC_LIBS="$AC_LIBS -lclib"],
	     [AC_ERROR(libclib is required)])
AC_CHECK_LIB([protobuf], [main],
	     [AC_LIBS="$AC_LIBS -lprotobuf"],
	     [AC_ERROR(libprotobuf is required)])
AC_CHECK_LIB([crypto], [main],
	     [AC_LIBS="$AC_LIBS -lcrypto"],
	     [OPENSSL=0])
AC_CHECK_LIB([ssl], [SSL_new],
	     [AC_LIBS="$AC_LIBS -lssl"],
	     [OPENSSL=0])
AC_CHECK_LIB([gtest], [main],
	     [GTEST_LIBS="$GTEST_LIBS -lgtest"])
AC_CHECK_LIB([gtest_main], [main],
	     [GTEST_LIBS="$GTEST_LIBS -lgtest_main"])
AC_CHECK_LIB([gmock], [main],
	     [GTEST_LIBS="$GTEST_LIBS -lgmock"])
AC_LIBS="$AC_LIBS"
AC_SUBST(GTEST_LIBS)
AC_SUBST(AC_LIBS)

AC_DEFINE_UNQUOTED(USE_OPENSSL, [$OPENSSL], [Use OpenSSL for crypto.])

# Checks for header files.
AC_CHECK_HEADERS([clib/clib.h errno.h fcntl.h inttypes.h memory.h	\
		  netdb.h netinet/in.h stdint.h string.h strings.h	\
		  sys/epoll.h sys/errno.h sys/kqueue.h sys/socket.h	\
		  toolbox/expvar.h unistd.h])
AC_CHECK_HEADER_STDBOOL

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T
AC_TYPE_UINT32_T
AC_TYPE_UINT64_T

# Checks for library functions.
AC_CHECK_FUNCS([epoll_create epoll_create1 epoll_wait epoll_pwait \
		memset socket strerror])

AC_CONFIG_FILES([Makefile
		 siot/Makefile])
AC_OUTPUT
