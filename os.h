#if defined(__FreeBSD__) || defined(__FreeBSD)
#define TON_FREE_BSD
#elif defined(__linux) || defined(__linux__) || defined(linux)
#define TON_LINUX
#elif defined(_WIN32) || defined(__WIN32__) || defined(WIN32)
#define TON_WINDOWS
#elif defined(__APPLE__)
#define TON_APPLE
#endif
