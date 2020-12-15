#ifndef TON_DBG_MSG
#if defined(TON_DEBUG)
    #define VA_ARGS(...) , ##__VA_ARGS__
    #define TON_DBG_MSG(fmt, ...) fprintf(stderr, fmt VA_ARGS(__VA_ARGS__))
#else
    #define TON_DBG_MSG(fmt, ...)
#endif
#endif