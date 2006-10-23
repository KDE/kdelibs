#include "kbetterthankdialog.h"

KBetterThanKDialog::KBetterThanKDialog( QWidget *parent )
  : QDialog( parent )
{
  setupUi( this );
  connect(_allowOnce, SIGNAL(clicked()), this, SLOT(allowOnceClicked()));
  connect(_allowAlways, SIGNAL(clicked()), this, SLOT(allowAlwaysClicked()));
  connect(_deny, SIGNAL(clicked()), this, SLOT(denyClicked()));
  connect(_denyForever, SIGNAL(clicked()), this, SLOT(denyForeverClicked()));
}

void KBetterThanKDialog::init()
{
    _allowOnce->setFocus();
}

void KBetterThanKDialog::setLabel( const QString & label )
{
    _label->setText(label);
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

void KBetterThanKDialog::allowOnceClicked()
{
    done(0);
}

void KBetterThanKDialog::allowAlwaysClicked()
{
    done(1);
}

void KBetterThanKDialog::denyClicked()
{
    done(2);
}

void KBetterThanKDialog::denyForeverClicked()
{
    done(3);
}

#include "kbetterthankdialog.moc"
