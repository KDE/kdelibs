// Idea was taken from filentry.h
// fileentry.h is part of KFM II, by Torben Weis

#ifndef _KCOMBO_H
#define _KCOMBO_H

#include <qcombo.h>

/** A new combobox for the KDE project.
  */
class KCombo : public QComboBox
{
  Q_OBJECT

public:
  /** Constructor
	*/
  KCombo ( bool _rw, QWidget *_parent, const char *name );

  /** Destructor
	*/
  ~KCombo ();

  /** Put the cursor at the end of the string. 
	*  This puts cursor at and of string. When using out of toolbar,
	* call this in your slot connected to signal completion.
	*/
  void cursorAtEnd();

  signals:

  /** Connect to this signal to receive ctrl-D
	*/
  void completion ();

  /** Connect to this signal to receive Ctrl-S
	*/
  void rotation ();

protected:
  virtual void keyPressEvent( QKeyEvent *_ev );

};

#endif
