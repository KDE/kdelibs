#include <qpainter.h>
#include <qdrawutil.h>
#include <qkeycode.h>
#include "kcombo.h"
#include "kcombo.h"
#include <qaccel.h>

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
    setCurrentItem(0);
    if (!set_text_called) {
	set_text_called = true;
	insertItem(text, 0);
    } 
    changeItem(text, 0);
}

#include "kcombo.moc"

