#include <kcmoduleproxy.h>
#include <kprintpreview.h>

Q_GLOBAL_STATIC_WITH_ARGS( KCModuleProxy, foo, ( (const char *)0 ) )
Q_GLOBAL_STATIC_WITH_ARGS( KPrintPreview, bar, ( (QPrinter*)0, (QWidget*)0 ) )

void _k__get_dummy_used()
{
    foo();
    bar();
}

#ifdef Q_OS_WIN
__declspec(dllexport) int __dummy_function() { return 5; }
#endif
