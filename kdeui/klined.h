// This was taken from filentry.h
// fileentry.h is part of KFM II, by Torben Weis



#ifndef _KLINED_H
#define _KLINED_H
#include <qlined.h>

/**
 * This widget has the same behaviour as QLineEditor, but has
 * signals two more key-events: ctrl-s and ctrl-d. This
 * class is first written by Torben Weis for KFMII as
 * fileentry.cpp
 */

class KLined : public QLineEdit
{
  Q_OBJECT
        
public:
  KLined ( QWidget *_parent, const char *_name );
  ~KLined ();

  /**
    * This puts cursor at and of string. When using out of toolbar,
    * call this in your slot connected to signal completion.
    */
  void cursorAtEnd();

  signals:

  /**
	* Connect to this signal to receive ctrl-D
	*/
  void completion ();
    
  /**
	* Connect to this signal to receive Ctrl-S
	*/
  void rotation ();
    
protected:
  virtual void keyPressEvent( QKeyEvent *_ev );

};

#endif
