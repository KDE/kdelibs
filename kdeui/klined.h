// This was taken from filentry.h
// fileentry.h is part of KFM II, by Torben Weis



#ifndef _KLINED_H
#define _KLINED_H
#include <qlined.h>

/** A new line editor for the KDE project.
  */
class KLined : public QLineEdit
{
  Q_OBJECT
        
public:
  KLined ( QWidget *_parent, const char *_name );
  ~KLined ();

  /** Put the cursor at the end of the string.
	* This puts cursor at and of string. When using out of toolbar,
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
