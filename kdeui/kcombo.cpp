#include <qpainter.h>
#include <qdrawutl.h>
#include <qkeycode.h>
#include "kcombo.h"
#include "kcombo.moc"
#include <qaccel.h>

KCombo::KCombo( QWidget* parent, const char* name, WFlags ) :
	QComboBox( parent, name)
{
    set_text_called = false;
    connect(this, SIGNAL(activated(int)),
	    SLOT(reset_text_called()));
    connect(this, SIGNAL(activated(const char*)),
	    SLOT(reset_text_called()));
}

KCombo::KCombo( bool readWrite, QWidget* parent, const char* name, WFlags ) :
	QComboBox( readWrite, parent, name)
{
    set_text_called = false;
    connect(this, SIGNAL(activated(int)),
	    SLOT(reset_text_called()));
    connect(this, SIGNAL(activated(const char*)),
	    SLOT(reset_text_called()));
}

void KCombo::reset_text_called()
{
    set_text_called = false;
}

void KCombo::setText( const char *text)
{
  if (!set_text_called){
    set_text_called = true;
    insertItem(text, 0);
    setCurrentItem(0);
  } 
  changeItem(text, 0);
}

