#include <qcombo.h>
#include "kcombo.moc"
#include "kcombo.h"
#include <qkeycode.h>

KCombo::KCombo (bool rw, QWidget *parent, const char *name)
  : QComboBox (rw, parent, name)
{
  installEventFilter (this);
}

KCombo::~KCombo ()
{
  removeEventFilter (this);
}

void KCombo::cursorAtEnd ()
{
  QKeyEvent ev( Event_KeyPress, Key_End, 0, 0 );
  QComboBox::keyPressEvent( &ev );
}


bool KCombo::eventFilter (QObject *, QEvent *e)
{
  if (e->type() == Event_KeyPress)
    {
      QKeyEvent *k = (QKeyEvent *) e;
      if (k->state() == ControlButton)
		{
		  if (k->key() == Key_S)
			{
			  emit rotation ();
			  QKeyEvent ev( Event_KeyPress, Key_End, 0, 0 );
			  QComboBox::keyPressEvent( &ev );
			  return TRUE;
			}
		  if (k->key() == Key_D)
			{
			  emit completion ();
			  QKeyEvent ev( Event_KeyPress, Key_End, 0, 0 );
			  QComboBox::keyPressEvent( &ev );
			  return TRUE;
			}
		}
    }
  return FALSE;
}
