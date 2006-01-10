/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you wish to add, delete or rename functions or slots use
** Qt Designer which will update this file, preserving your code. Create an
** init() function in place of a constructor, and a destroy() function in
** place of a destructor.
*****************************************************************************/


void KJSErrorDlg::init()
{
    _errorText->setText("<qt>");
}

void KJSErrorDlg::addError( const QString & error )
{
    // Argh why doesn't QText* interpret html tags unless I use setText()?
    // This is really inefficient!
    _errorText->setText(_errorText->text() + "<br>\n" + error);
}

void KJSErrorDlg::setURL( const QString & url )
{
    _url->setText(url);
}

void KJSErrorDlg::clear()
{
    _errorText->clear();
    _errorText->insert("<qt>");
}
