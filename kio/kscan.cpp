#include <klocale.h>
#include <ktrader.h>

#include "kscan.h"

// static factory method
KScanDialog * KScanDialog::getScanDialog( QWidget *parent, const char *name,
					  bool modal )
{
    KTrader::OfferList offers = KTrader::self()->query("KScan/KScanDialog");
    if ( offers.isEmpty() )
	return 0L;
	
    KService::Ptr ptr = *(offers.begin());
    KLibFactory *factory = KLibLoader::self()->factory( ptr->library().latin1() );
    
    if ( !factory )
        return 0;

    QStringList args;
    args << QString::number( (int)modal );

    QObject *res = factory->create( parent, name, "KScanDialog", args ); 

    return dynamic_cast<KScanDialog *>( res );
}


KScanDialog::KScanDialog( int dialogFace, int buttonMask,
			  QWidget *parent, const char *name, bool modal )
    : KDialogBase( dialogFace, i18n("Acquire image"), buttonMask, Close,
		   parent, name, modal, true ),
      m_currentId( 1 )
{

}

KScanDialog::~KScanDialog()
{
}


///////////////////////////////////////////////////////////////////


KScanDialogFactory::KScanDialogFactory( QObject *parent, const char *name )
    : KLibFactory( parent, name ),
      m_instance( 0L )
{
}

KScanDialogFactory::~KScanDialogFactory()
{
    delete m_instance;
}

QObject *KScanDialogFactory::createObject( QObject *parent, const char *name,
                                           const char *classname,
                                           const QStringList &args )
{
    if ( strcmp( classname, "KScanDialog" ) != 0 )
        return 0;

    if ( !parent->isWidgetType() )
       return 0;

    bool modal = false;
    
    if ( args.count() == 1 )
        modal = (bool)args[ 0 ].toInt();

    return createDialog( static_cast<QWidget *>( parent ), name, modal );
}

#include "kscan.moc"
