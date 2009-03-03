
#include <kcharsets.h>
#include <QtCore/QString>

#include <assert.h>

int main()
{
    // Test that toLocal8Bit works even without a QCoreApplication,
    // thanks to the static initializer in KCatalog.
    // Do NOT move this code to a QTestLib unit test ;-)
    QString one = QString::fromUtf8("é");
    QByteArray one8bit = one.toLocal8Bit();
    Q_ASSERT(one8bit.length() == 2);


    QString input( "&lt;Hello &amp;World&gt;" );
    QString output = KCharsets::resolveEntities( input );
    assert( output == "<Hello &World>" );
    return 0;
}
