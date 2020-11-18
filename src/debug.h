#if defined(TON_DEBUG)
    #define TON_DBG_MSG(fmt, ...) fprintf(stderr, fmt, __VA_ARGS__)
#else
    #define TON_DBG_MSG(fmt, ...)
#endif