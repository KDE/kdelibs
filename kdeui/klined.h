// This was taken from filentry.h
// fileentry.h is part of KFM II, by Torben Weis



#ifndef _KLINED_H
#define _KLINED_H
#include <qlined.h>

/**
 * This widget has the same behaviour as QLineEditor, but emits
 * signals for two more key-events: @ref completion when Ctrl-D is
 * pressed and  @ref rotation when Ctrl-S is pressed. This
 * class is inspired by Torben Weis' fileentry.cpp for KFM II
 * @short KDE Line input widget
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
	* Connect to this signal to receive Ctrl-D
	*/
  void completion ();
    
  /**
	* Connect to this signal to receive Ctrl-S
	*/
  void rotation ();
    
 protected:
     bool eventFilter (QObject *, QEvent *);
 };

#endif
