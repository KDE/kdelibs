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
  /**
    *
    * Kcombo is normal writable or readonly ComboBox with two
    * more signals completion() (ctrl-d) and rotation (ctrl-s).
    * Unfortunately, ctrl-d combination doesn't work. Use signal
    * rotation() instead.
    * @author sven.radej@iname.com
    */
   
  KCombo ( bool _rw, QWidget *_parent, const char *name );
  ~KCombo ();

  /**
    * This puts cursor at and of string. When using out of toolbar,
    * call this in your slot connected to signal completion.
    *
    */
   
  void cursorAtEnd();

  signals:

  /**
	* Connect to this signal to receive ctrl-D
	* Unfortunately this doesn't work. Use signal rotation() instead.
	*/
     
  void completion ();
  /// Connect to this signal to receive Ctrl-S
  void rotation ();

protected:
  virtual void keyPressEvent( QKeyEvent *_ev );

};

#endif
