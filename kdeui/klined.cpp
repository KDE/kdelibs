// This was taken from filentry.cpp
// fileentry.cpp is part of KFM II, by Torben Weis


#include <qlined.h>
#include "klined.moc"

#include <qkeycode.h>


KLined::KLined (QWidget *parent, const char *name)
  : QLineEdit (parent, name)
{
  // setFocusPolicy(ClickFocus); // this doesn't enable tab (grin!)
}

KLined::~KLined ()
{
}

void KLined::cursorAtEnd ()
{
  QKeyEvent ev( Event_KeyPress, Key_End, 0, 0 );
  QLineEdit::keyPressEvent( &ev );
}

void KLined::keyPressEvent (QKeyEvent *e)
{
  // if (e->key() == Key_Tab)   // Is there any way to use Tab?
  if ( e->key() == Key_D && e->state() == ControlButton )
    {
      e->accept();
      emit completion ();
      QKeyEvent ev( Event_KeyPress, Key_End, 0, 0 );
      QLineEdit::keyPressEvent( &ev );
    }
  else if ( e->key() == Key_S && e->state() == ControlButton )
    {
      e->accept();
      emit rotation ();
      QKeyEvent ev( Event_KeyPress, Key_End, 0, 0 );
      QLineEdit::keyPressEvent( &ev );
    }
  else
	QLineEdit::keyPressEvent(e);
  return;
}
