#include <qcombo.h>

#include "kcombo.moc"

#include "kcombo.h"

#include <qkeycode.h>


KCombo::KCombo (bool rw, QWidget *parent, const char *name)
  : QComboBox (rw, parent, name)
{
}

KCombo::~KCombo ()
{
}

void KCombo::cursorAtEnd ()
{
  QKeyEvent ev( Event_KeyPress, Key_End, 0, 0 );
  QComboBox::keyPressEvent( &ev );
}

void KCombo::keyPressEvent (QKeyEvent *e)
{
  // if (e->key() == Key_Tab)   // Is there any way to use Tab?
  if ( e->key() == Key_D && e->state() == ControlButton )
    {
      e->accept();
      emit completion ();
      QKeyEvent ev( Event_KeyPress, Key_End, 0, 0 );
      QComboBox::keyPressEvent( &ev );
    }
  else if ( e->key() == Key_S && e->state() == ControlButton )
    {
      e->accept();
      emit rotation ();
      QKeyEvent ev( Event_KeyPress, Key_End, 0, 0 );
      QComboBox::keyPressEvent( &ev );
    }
  else
	QComboBox::keyPressEvent(e);
  return;
}
