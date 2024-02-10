//#undef __linux__
//#define TARGET_FREERTOS
//#define TARGET_8266
#ifdef __linux__
#define TARGET_LINUX
#elif defined(WIN32)
#define TARGET_WIN32
#else
#define TARGET_8266
#define TARGET_FREERTOS
#endif
/* end of target definitions */