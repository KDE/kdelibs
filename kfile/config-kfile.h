#ifndef CONFIG_KFILE_H
#define CONFIG_KFILE_H


#define COOLOS 1

#if defined(COOLOS)
#define debugC debug
#else
inline void debugC(const char *,...) {};
#endif

#define DefaultViewStyle QString::fromLatin1("SimpleView")
#define DefaultPannerPosition 40
#define DefaultMixDirsAndFiles true
#define DefaultShowStatusLine false
#define DefaultShowHidden false
#define DefaultFindCommand QString::fromLatin1("kfind")
#define ConfigGroup QString::fromLatin1("KFileDialog Settings")
#endif
