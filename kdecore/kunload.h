#include <qglobal.h>

#if QT_VERSION < 300

#include <qobjectdict.h>
#include <kapp.h>

#define _UNLOAD(p) \
static void unload_##p (const char **list) \
{ \
    if (!objectDict || !kapp) return; \
    /*qDebug("removing meta classes for %s", #p);*/ \
    const char **n = list; \
    for (; *n; n++) { \
        /*qDebug("  removing metaclass %s", *n);*/ \
	/* Because objectDict was set to AutoDelete the following does the \
           right thing. */ \
        while (objectDict->remove(*n)) ; \
    } \
    /*qDebug("removing done");*/ \
} \
 \
class _KUnloadMetaClass_Helper##p { \
public: \
    _KUnloadMetaClass_Helper##p () {} \
    ~_KUnloadMetaClass_Helper##p () { \
        unload_##p (_metalist_##p); \
    } \
}; \
 \
static _KUnloadMetaClass_Helper##p _helper_object_##p;

#else
#define _UNLOAD(p) 
#endif
