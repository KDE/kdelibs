#include "kjserrordlg.h"

KJSErrorDlg::KJSErrorDlg( QWidget *parent )
  : QDialog( parent )
{
  setupUi( this );

  init();
}

void KJSErrorDlg::addError( const QString & error )
{
  _errorText->append(error);
}

void KJSErrorDlg::setURL( const QString & url )
{
  _url->setPlainText(url);
}

void KJSErrorDlg::clear()
{
  _errorText->clear();
  init();
}

void KJSErrorDlg::init()
{
  _errorText->setAcceptRichText(false);
}
