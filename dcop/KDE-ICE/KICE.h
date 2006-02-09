#ifndef KICE_H
#define KICE_H

#include <kdelibs_export.h>

#ifndef KICE_EXPORT
# if defined(MAKE_DCOP_LIB) || defined(MAKE_KICE_LIB)
# define KICE_EXPORT KDE_EXPORT
# else
# define KICE_EXPORT KDE_IMPORT
# endif 
#endif 

#endif
