PHP_ARG_WITH([ton_client],
   [for ton_client support],
   [AS_HELP_STRING([--with-ton_client],
     [Include ton_client support])])

if test "$PHP_TON_CLIENT" != "no"; then
  LIBNAME=ton_client
  LIBSYMBOL=tc_create_context
  LIB_PATH=../deps/lib/x64
  PHP_CHECK_LIBRARY($LIBNAME,$LIBSYMBOL,
  [
    PHP_ADD_LIBRARY_WITH_PATH($LIBNAME, $LIB_PATH, TON_CLIENT_DIR_SHARED_LIBADD)
    AC_DEFINE(HAVE_TON_CLIENTLIB, 1 ,[ ])
  ],[
    AC_MSG_ERROR([wrong $LIBNAME lib version or lib not found])
  ],[
    -L$LIB_PATH -ldl
  ])
  AC_DEFINE(HAVE_TON_CLIENT, 1, [ Have TON Client support ])
  PHP_NEW_EXTENSION(ton_client, ton_client.c rpa_queue.c, $ext_shared)
fi
