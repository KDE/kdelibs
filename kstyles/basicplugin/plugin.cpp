#include <kthemestyle.h>
#include <klocale.h>
#include <qstring.h>

extern "C" {
    KStyle* allocate();
    KStyle* allocateCustomTheme(QString &configFile);
    int minor_version();
    int major_version();
    const char *description();
}

KStyle* allocate()
{
    return(new KThemeStyle());
}

KStyle* allocateCustomTheme(QString &configFile)
{
    return(new KThemeStyle(configFile));
}

int minor_version()
{
    return(0);
}

int major_version()
{
    return(1);
}

const char *description()
{
    return(i18n("Basic themed plugin"));
}
