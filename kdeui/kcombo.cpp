#include <qpainter.h>
#include <qdrawutl.h>
#include <qkeycode.h>
#include "kcombo.h"
#include "kcombo.moc"


KCombo::KCombo( QWidget* parent, const char* name, WFlags ) :
	QComboBox( parent, name)
{
  set_text_called = false;
}

KCombo::KCombo( bool readWrite, QWidget* parent, const char* name, WFlags ) :
	QComboBox( readWrite, parent, name)
{
  set_text_called = false;

}

void KCombo::setText( const char *text)
{
  if (!set_text_called){
    set_text_called = true;
    insertItem(text, 0);
  }
  changeItem(text, 0);
}

