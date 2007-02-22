#ifdef KDE_NO_COMPAT
#error ksimpleconfig.h does not exist anymore. The KSimpleConfig class has been removed. Instead use KConfig with the OnlyLocal flag to prevent config file merging (including kdeglobals). See the KDE4 Porting Guide for more information.
#else
#include <kconfig.h>
#endif
