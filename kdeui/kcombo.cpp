#include "kcombo.h"

#include <qlineedit.h>

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

void KCombo::setText( const QString& text)
{
    setCurrentItem(0);
    if (!set_text_called) {
	set_text_called = true;
	insertItem(text, 0);
    } else {
        changeItem(text, 0);
    }
}

#include "kcombo.moc"

