#define COOLOS 1

#if defined(COOLOS)
#define debugC debug
#else
inline void debugC(const char *,...) {};
#endif

