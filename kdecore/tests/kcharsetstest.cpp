
#include <kcharsets.h>
#include <kdebug.h>
#include <QtCore/QString>

#include <assert.h>

int main()
{
    // Note: ctest sets LANG=C, while running the test directly uses your real $LANG.
    // And since the static initializer runs before main, we cannot setenv("LANG") here,
    // it's too late.
    //kDebug() << "LANG=" << getenv("LANG");
    //kDebug() << "LC_ALL=" << getenv("LC_ALL");

    // Test that toLocal8Bit works even without a QCoreApplication,
    // thanks to the static initializer in KCatalog.
    // Do NOT move this code to a QTestLib unit test ;-)
    QString one = QString::fromUtf8("é");
    QByteArray one8bit = one.toLocal8Bit();
    if (qgetenv("LANG").endsWith("UTF-8")) {
        kDebug() << one << one8bit;
        Q_ASSERT(one8bit.length() == 2);
    }

    QString input( "&lt;Hello &amp;World&gt;" );
    QString output = KCharsets::resolveEntities( input );
    assert( output == "<Hello &World>" );
    return 0;
}
