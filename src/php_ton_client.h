/* ton_client extension for PHP */

#ifndef PHP_TON_CLIENT_H
# define PHP_TON_CLIENT_H

extern zend_module_entry ton_client_module_entry;
# define phpext_ton_client_ptr &ton_client_module_entry

# define PHP_TON_CLIENT_VERSION "1.27.0"

# if defined(ZTS) && defined(COMPILE_DL_TON_CLIENT)
ZEND_TSRMLS_CACHE_EXTERN()
# endif

#endif	/* PHP_TON_CLIENT_H */
