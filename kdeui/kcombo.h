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
    * more signals @ref completion (Ctrl-D pressed) and @ref rotation
    * (Ctrl-S pressed).
    * @short KDE combo-box Widget
    * @author sven.radej@iname.com
    */
   
  KCombo ( bool _rw, QWidget *_parent, const char *name );
  ~KCombo ();

  /**
    * This puts cursor at and of string. When using out of toolbar,
    * call this in your slot connected to signal @ref completion
    * or @ref rotation
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
