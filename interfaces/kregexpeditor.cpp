#include "kregexpeditor.h"
#include <qdialog.h>
#include <ktrader.h>
#include <kservice.h>
#include <klibloader.h>
#include <kdebug.h>

QWidget* KRegExpEditor::createEditor( QWidget* parent, const char* name )
{
  KTrader::OfferList offers = KTrader::self()->query( "KRegExpEditor/KRegExpEditor" );
  if ( offers.count() < 1 ) {
    qDebug("Couldn't find library");
    return 0;
  }
  
  KService::Ptr service = *offers.begin();
  
  KLibFactory *factory = KLibLoader::self()->factory( service->library().latin1() );
  if ( ! factory ) {
    kdWarning() << "Couldn't find a factory" << endl;
    return 0;
  }
  
  QObject *obj = factory->create( parent, name, "QWidget" );
  return dynamic_cast<QWidget *>( obj );
}

QDialog* KRegExpEditor::createDialog( QWidget* parent, const char* name )
{
  KTrader::OfferList offers = KTrader::self()->query( "KRegExpEditor/KRegExpEditor" );
  if ( offers.count() < 1 ) {
    qDebug("Couldn't find library");
    return 0;
  }
  
  KService::Ptr service = *offers.begin();
  
  KLibFactory *factory = KLibLoader::self()->factory( service->library().latin1() );
  if ( ! factory ) {
    kdWarning() << "Couldn't find a factory" << endl;
    return 0;
  }
  
  QObject *obj = factory->create( parent, name, "QDialog" );
  return dynamic_cast<QDialog *>( obj );
}

