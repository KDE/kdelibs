/*  This file is part of the KDE Libraries
    Copyright (C) 1998 Thomas Tanghus (tanghus@earthling.net)
    with addtions by Espen Sand (espensa@online.no)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/  

#ifndef __KDIALOG_H
#define __KDIALOG_H


class QLayoutItem;

#include <qdialog.h>

/**
 * Dialog with extended modeless support and and methods for KDE standard
 * compliance.
 *
 * If the dialog is modeless and has a parent, the default keybindings 
 * (escape = reject(), enter = accept() etc.) are disabled.
 *
 * The marginHint() and spacingHint() sizes shall be used whenever you layout
 * the interior of a dialog. One special note. If you make your own action
 * buttons (Ok, Cancel etc), the space beteween the buttons shall be 
 * spacingHint(), while the space above, below, to right and to the left shall
 * be marginHint(). If you add a separator line above the buttons, there 
 * shall be a marginHint() between the buttons and the separator and a 
 * marginHint() above the separator as well.
 *
 * @author Thomas Tanghus <tanghus@earthling.net>, Espen Sand <espensa@online.no>
 * @version 0.1.2
*/
class KDialog : public QDialog
{
  Q_OBJECT

  public:

    /**
     * Constructor. Takes the same arguments as QDialog.
     */
    KDialog(QWidget *parent = 0, const char *name = 0, 
	    bool modal = false, WFlags f = 0);

    /**
     * Returns the number of pixels you shall use between a 
     * dialog edge and the outermost widget(s) according to the KDE standard.
     */
    int marginHint( void ) const;

    /**
     * Returns the number of pixels you shall use between 
     * widgets inside a dialog according to the KDE standard.
     */
    int spacingHint( void ) const;

    /**
     * Resizes every layout manager used in w and its nested children.  
     *
     * @param margin The new layout margin
     *
     * @param margin The new layout spacing
     *
     */
    static void resizeLayout( QWidget *w, int margin, int spacing );

    /**
     * Reszies every layout associated with 'lay' and its children. 
     *
     * @param margin The new layout margin
     *
     * @param margin The new layout spacing
     *
     */
    static void resizeLayout( QLayoutItem *lay, int margin, int spacing );

  public slots:
    /**
     * Makes a KDE compliant caption.
     * 
     * @param caption Your caption. DO NOT include the application name
     * in this string. It will be added automatically according to the KDE
     * standard.
     */
    virtual void setCaption( const QString &caption );

    /**
     * Makes a plain caption without any modifications.
     * 
     * @param caption Your caption. This is the string that will be 
     * displayed in the window title.
     */
    virtual void setPlainCaption( const QString &caption );


  protected:
    /**
     * @internal
     */
    virtual void keyPressEvent(QKeyEvent*);


   signals:
    /**
     * This signal is emitted when the margin size and/or spacing size 
     * have changed. Use @ref #marginHint and  @ref #spacingHint in your slot
     * to get the new values.
     */
    void layoutHintChanged( void );


  private:
    static int mMarginSize;
    static int mSpacingSize;

};

#endif // __KDIALOG_H


