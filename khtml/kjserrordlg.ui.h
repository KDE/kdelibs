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
    _errorText->setTextFormat(QTextEdit::LogText);
    _errorText->setMaxLogLines(2048);//Limit to about 2K errors so we don't use much CPU
}

void KJSErrorDlg::addError( const QString & error )
{
    
    _errorText->append(error);
}

void KJSErrorDlg::setURL( const QString & url )
{
    _url->setText(url);
}

void KJSErrorDlg::clear()
{
    _errorText->clear();
    init();
}
