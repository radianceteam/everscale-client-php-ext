PHP_ARG_WITH([ton_client],
   [for ton_client support],
   [AS_HELP_STRING([--with-ton_client],
     [Include ton_client support])])

if test "$PHP_TON_CLIENT" != "no"; then
  AC_DEFINE(HAVE_TON_CLIENT, 1, [ Have TON Client support ])
  PHP_NEW_EXTENSION(ton_client, ton_client.c rpa_queue.c, $ext_shared)
fi
