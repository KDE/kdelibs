#ifndef KEMOTICONS_EXPORT_H
#define KEMOTICONS_EXPORT_H

/* needed for KDE_EXPORT and KDE_IMPORT macros */
#include <kdemacros.h>

#ifndef KEMOTICONS_EXPORT
#ifdef MAKE_KEMOTICONS_LIB)
   /* We are building this library */
#  define KEMOTICONS_EXPORT KDE_EXPORT
# else
   /* We are using this library */
#  define KEMOTICONS_EXPORT KDE_IMPORT
# endif

#endif
