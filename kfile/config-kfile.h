#ifndef CONFIG_KFILE_H
#define CONFIG_KFILE_H


// #define COOLOS 1

#if defined(COOLOS)
#define debugC debug
#else
inline void debugC(const char *,...) {};
#endif

#define DefaultViewStyle "SimpleView"
#define DefaultSingleClick false
#define DefaultShowListLabels true
#define DefaultShowFilter true
#define DefaultPannerPosition 40
#define DefaultMixDirsAndFiles true
#define DefaultShowStatusLine false
#define DefaultShowHidden false
#define DefaultKeepDirsFirst true
#define DefaultFindCommand "kfind"
#define DefaultShowDirsLeft true


#endif
