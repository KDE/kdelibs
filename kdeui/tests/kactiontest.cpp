
#include <qpointer.h>

#include <kapplication.h>
#include <kaction.h>

#include <assert.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>

int main( int argc, char **argv )
{
    KAboutData aboutData( "kactiontest", "kactiontest", "1.0" );
    KCmdLineArgs::init(argc, argv, &aboutData);
    KApplication app;

    KActionCollection coll( static_cast<QObject *>( 0 ) );

    QPointer<KAction> action1 = new KRadioAction("test",0, &coll);
    QPointer<KAction> action2 = new KRadioAction("test",0, &coll);
    QPointer<KAction> action3 = new KRadioAction("test",0, &coll);
    QPointer<KAction> action4 = new KRadioAction("test",0, &coll);
    QPointer<KAction> action5 = new KRadioAction("test",0, &coll);
    QPointer<KAction> action6 = new KRadioAction("test",0, &coll);
    QPointer<KAction> action7 = new KRadioAction("test",0, &coll);

    coll.clear();
    assert( coll.isEmpty() );

    assert( action1.isNull() );
    assert( action2.isNull() );
    assert( action3.isNull() );
    assert( action4.isNull() );
    assert( action5.isNull() );
    assert( action6.isNull() );
    assert( action7.isNull() );

    return 0;
}

/* vim: et sw=4 ts=4
 */
