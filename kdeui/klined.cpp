// This was taken from filentry.cpp
// fileentry.cpp is part of KFM II, by Torben Weis


#include <qlined.h>
#include "klined.moc"

#include <qkeycode.h>


KLined::KLined (QWidget *parent, const char *name)
  : QLineEdit (parent, name)
{
  installEventFilter (this);
}

KLined::~KLined ()
{
  removeEventFilter (this);
}

void KLined::cursorAtEnd ()
{
  QKeyEvent ev( Event_KeyPress, Key_End, 0, 0 );
  QLineEdit::keyPressEvent( &ev );
}


bool KLined::eventFilter (QObject *, QEvent *e)
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
			  QLineEdit::keyPressEvent( &ev );
			  return TRUE;
			}
		  if (k->key() == Key_D)
			{
			  emit completion ();
			  QKeyEvent ev( Event_KeyPress, Key_End, 0, 0 );
			  QLineEdit::keyPressEvent( &ev );
			  return TRUE;
			}
		}
    }
  return FALSE;
}
