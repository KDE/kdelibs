#include "kbetterthankdialog.h"

KBetterThanKDialog::KBetterThanKDialog( QWidget *parent )
  : QDialog( parent )
{
  setupUi( this );
}

void KBetterThanKDialog::init()
{
    _allowOnce->setFocus();
}

void KBetterThanKDialog::setLabel( const QString & label )
{
    _label->setPlainText(label);
}

void KBetterThanKDialog::clicked()
{
    if (sender() == _allowOnce) {
        done(0);
    } else if (sender() == _allowAlways) {
        done(1);
    } else if (sender() == _deny) {
        done(2);
    } else if (sender() == _denyForever) {
        done(3);
    }
}

void KBetterThanKDialog::accept()
{
    setResult(0);
}

void KBetterThanKDialog::reject()
{
    QDialog::reject();
    setResult(2);
}

#include "kbetterthankdialog.moc"
