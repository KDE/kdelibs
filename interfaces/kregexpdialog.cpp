#include "kregexpdialog.h"
#include <qlineedit.h>
#include <kregexpeditor.h>
#include <klocale.h>
#include <qlayout.h>
#include <qpushbutton.h>

KRegExpDialog::KRegExpDialog( QWidget* parent, const char* name, bool modal, const QString& caption )
  : KDialogBase( KDialogBase::Plain, caption == QString::null ? i18n("Regular Expression Editor") : caption,
                 KDialogBase::Ok | KDialogBase::Cancel | KDialogBase::Help, KDialogBase::Ok, 
                 parent, name ? name : "KRegExpDialog", modal )
{
  QFrame* frame = plainPage();
  QVBoxLayout* layout = new QVBoxLayout( frame );
  layout->setAutoAdd( true );
    
  _guiEditor = KRegExpEditor::createEditor( frame );
  
  if ( _guiEditor ) {
    // Found the rgeaphical editor everythis is OK.
  }
  else {
    QString text =
      i18n("<qt>Here you should normally see a user friendly graphical editor for regular expressions. "
           "The most likely reason that you do not see this editor now is that you do not have "
           "kdeutils installed. If you are a 100 percent sure that you do have kdeutils installed, "
           "then this is a bug that we encourage you to report.</qt>");
    QLabel* label = new QLabel( text, frame );
    _lineEditor = new QLineEdit( frame );
    actionButton( KDialogBase::Help )->setEnabled( false );
  }
}

void KRegExpDialog::slotSetRegExp( const QString& regexp )
{
  if ( _guiEditor )
    _guiEditor->slotSetRegExp( regexp );
  else
    _lineEditor->setText( regexp );
}

QString KRegExpDialog::regexp() const
{
  if ( _guiEditor )
    return _guiEditor->regexp();
  else
    return _lineEditor->text();
}

bool KRegExpDialog::isGuiEditor()
{
  KRegExpEditor* editor = KRegExpEditor::createEditor( 0 );
  bool found = ( editor != 0 );
  delete editor; // Yes it is OK to delete 0 ;-)
  return found;
}

void KRegExpDialog::slotHelp()
{
  kapp->invokeHelp( QString::null, QString::fromLocal8Bit("KRegExpEditor") ); 
}


#include "kregexpdialog.moc"
