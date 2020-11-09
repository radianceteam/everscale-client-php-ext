dnl config.m4 for extension ton_client

dnl Comments in this file start with the string 'dnl'.
dnl Remove where necessary.

PHP_ARG_WITH([ton_client],
   [for ton_client support],
   [AS_HELP_STRING([--with-ton_client],
     [Include ton_client support])])

if test "$PHP_TON_CLIENT" != "no"; then
  dnl Write more examples of tests here...

  dnl Remove this code block if the library does not support pkg-config.
  dnl PKG_CHECK_MODULES([LIBFOO], [foo])
  dnl PHP_EVAL_INCLINE($LIBFOO_CFLAGS)
  dnl PHP_EVAL_LIBLINE($LIBFOO_LIBS, TON_CLIENT_SHARED_LIBADD)

  dnl If you need to check for a particular library version using PKG_CHECK_MODULES,
  dnl you can use comparison operators. For example:
  dnl PKG_CHECK_MODULES([LIBFOO], [foo >= 1.2.3])
  dnl PKG_CHECK_MODULES([LIBFOO], [foo < 3.4])
  dnl PKG_CHECK_MODULES([LIBFOO], [foo = 1.2.3])

  dnl Remove this code block if the library supports pkg-config.
  dnl --with-ton_client -> check with-path
  dnl SEARCH_PATH="/usr/local /usr"     # you might want to change this
  dnl SEARCH_FOR="/include/ton_client.h"  # you most likely want to change this
  dnl if test -r $PHP_TON_CLIENT/$SEARCH_FOR; then # path given as parameter
  dnl   TON_CLIENT_DIR=$PHP_TON_CLIENT
  dnl else # search default path list
  dnl   AC_MSG_CHECKING([for ton_client files in default path])
  dnl   for i in $SEARCH_PATH ; do
  dnl     if test -r $i/$SEARCH_FOR; then
  dnl       TON_CLIENT_DIR=$i
  dnl       AC_MSG_RESULT(found in $i)
  dnl     fi
  dnl   done
  dnl fi
  dnl
  dnl if test -z "$TON_CLIENT_DIR"; then
  dnl   AC_MSG_RESULT([not found])
  dnl   AC_MSG_ERROR([Please reinstall the ton_client distribution])
  dnl fi

  dnl Remove this code block if the library supports pkg-config.
  dnl --with-ton_client -> add include path
  dnl PHP_ADD_INCLUDE($TON_CLIENT_DIR/include)

  dnl Remove this code block if the library supports pkg-config.
  dnl --with-ton_client -> check for lib and symbol presence
  dnl LIBNAME=TON_CLIENT # you may want to change this
  dnl LIBSYMBOL=TON_CLIENT # you most likely want to change this

  dnl If you need to check for a particular library function (e.g. a conditional
  dnl or version-dependent feature) and you are using pkg-config:
  dnl PHP_CHECK_LIBRARY($LIBNAME, $LIBSYMBOL,
  dnl [
  dnl   AC_DEFINE(HAVE_TON_CLIENT_FEATURE, 1, [ ])
  dnl ],[
  dnl   AC_MSG_ERROR([FEATURE not supported by your ton_client library.])
  dnl ], [
  dnl   $LIBFOO_LIBS
  dnl ])

  dnl If you need to check for a particular library function (e.g. a conditional
  dnl or version-dependent feature) and you are not using pkg-config:
  dnl PHP_CHECK_LIBRARY($LIBNAME, $LIBSYMBOL,
  dnl [
  dnl   PHP_ADD_LIBRARY_WITH_PATH($LIBNAME, $TON_CLIENT_DIR/$PHP_LIBDIR, TON_CLIENT_SHARED_LIBADD)
  dnl   AC_DEFINE(HAVE_TON_CLIENT_FEATURE, 1, [ ])
  dnl ],[
  dnl   AC_MSG_ERROR([FEATURE not supported by your ton_client library.])
  dnl ],[
  dnl   -L$TON_CLIENT_DIR/$PHP_LIBDIR -lm
  dnl ])
  dnl
  dnl PHP_SUBST(TON_CLIENT_SHARED_LIBADD)

  dnl In case of no dependencies
  AC_DEFINE(HAVE_TON_CLIENT, 1, [ Have TON Client support ])

  PHP_NEW_EXTENSION(ton_client, ton_client.c, $ext_shared)
fi
