#include <klocale.h>
#include <ktrader.h>

#include "kscan.h"

// static factory method
KScanDialog * KScanDialog::getScanDialog( QWidget *parent, const char *name,
					  bool modal )
{
    KScanDialog *dlg = 0L;

    KTrader::OfferList offers = KTrader::self()->query("KScan/KScanDialog");
    if ( offers.isEmpty() )
	return 0L;
	
    KService::Ptr ptr = *(offers.begin());
    KLibFactory *factory = KLibLoader::self()->factory( ptr->library().latin1() );
    if ( factory ) {
	KScanDialogFactory *fac = static_cast<KScanDialogFactory*>( factory );
	dlg = fac->create( parent, name, modal );
    }
	
    return dlg;
}


KScanDialog::KScanDialog( QWidget *parent, const char *name, bool modal )
    : KDialogBase( Tabbed, i18n("Acquire image"), Close|Help, Close,
		   parent, name, modal, true ),
      m_currentId( 1 )
{

}

KScanDialog::~KScanDialog()
{
}


///////////////////////////////////////////////////////////////////


// to be implemented by the library
KScanDialog * KScanDialogFactory::createDialog( QWidget *, const char *, bool )
{
    return 0L;
}

KScanDialogFactory::KScanDialogFactory( QObject *parent, const char *name )
    : KLibFactory( parent, name ),
      m_instance( 0L )
{
}

KScanDialogFactory::~KScanDialogFactory()
{
    delete m_instance;
}

KScanDialog * KScanDialogFactory::create( QWidget *parent,
					  const char *name, bool modal )
{
    KScanDialog *dlg = createDialog( parent, name, modal );
    if ( dlg )
	emit objectCreated( dlg );
    return dlg;
}


#include "kscan.moc"
