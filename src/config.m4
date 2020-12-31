PHP_ARG_WITH([ton_client],
   [for ton_client support],
   [AS_HELP_STRING([--with-ton_client],
     [Include ton_client support])])

PHP_ARG_WITH([ton_client_debug],
   [for ton_client support (debug)],
   [AS_HELP_STRING([--with-ton_client_debug],
     [Include ton_client support (debug)])])

if test "$PHP_TON_CLIENT" != "no" || "$PHP_TON_CLIENT_DEBUG" != "no"; then

  if test "$PHP_TON_CLIENT_DEBUG" != "no"; then
    PHP_TON_CLIENT=$PHP_TON_CLIENT_DEBUG
  fi

  if test -r $PHP_TON_CLIENT/include/tonclient.h; then
    TON_CLIENT_DIR=$PHP_TON_CLIENT
  else
    AC_MSG_CHECKING(for ton_client in default path)
    for i in /usr/local /usr $(pwd); do
      if test -r $i/include/tonclient.h; then
        TON_CLIENT_DIR=$i
        AC_MSG_RESULT(found in $i)
        break
      fi
    done
  fi

  if test -z "$TON_CLIENT_DIR"; then
    AC_MSG_RESULT(not found)
    AC_MSG_ERROR(Please point to the correct ton_client distribution)
  fi

  PHP_CHECK_LIBRARY(ton_client,tc_create_context,
  [
    PHP_ADD_INCLUDE($TON_CLIENT_DIR/include)
    PHP_ADD_LIBRARY_WITH_PATH(ton_client, $TON_CLIENT_DIR/$PHP_LIBDIR, EXTRA_CFLAGS)
    PHP_SUBST(EXTRA_CFLAGS)
    if test "$PHP_TON_CLIENT_DEBUG" != "no"; then
      AC_DEFINE(TON_DEBUG, 1, [ Enable TON Client debug output ])
    fi
    AC_DEFINE(HAVE_TON_CLIENT, 1, [ Have TON Client support ])
  ],[
    AC_MSG_ERROR([ton lib not found])
  ],[
    -L$TON_CLIENT_DIR/$PHP_LIBDIR
  ])

  PHP_NEW_EXTENSION(ton_client, ton_client.c rpa_queue.c, $ext_shared)
fi
