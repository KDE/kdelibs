/*  This file is part of the KDE Libraries
 *  Copyright (C) 1998 Thomas Tanghus (tanghus@earthling.net)
 *  Additions 1999-2000 by Espen Sand (espen@kde.org) 
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 */  

#ifndef _KDIALOG_H_
#define _KDIALOG_H_

class QLayoutItem;

#include <qdialog.h>

/**
 * Dialog with extended modeless support and methods for KDE standard
 * compliance.
 *
 * If the dialog is modeless and has a parent, the default keybindings 
 * (@p escape = @p reject(), @p enter = @p accept(), etc.) are disabled.
 *
 * The @ref marginHint() and @ref spacingHint() sizes shall be used
 * whenever you layout the interior of a dialog. One special note. If
 * you make your own action buttons (OK, Cancel etc), the space
 * beteween the buttons shall be @ref spacingHint(), whereas the space
 * above, below, to the right and to the left shall be @ref marginHint().
 * If you add a separator line above the buttons, there shall be a
 * @ref marginHint() between the buttons and the separator and a
 * @ref marginHint() above the separator as well.
 *
 * @author Thomas Tanghus <tanghus@earthling.net>, Espen Sand <espensa@online.no>
 * @version 0.1.2
*/
class KDialog : public QDialog
{
  Q_OBJECT

  public:

    /**
     * Constructor. 
     *
     * Takes the same arguments as @ref QDialog.
     */
    KDialog(QWidget *parent = 0, const char *name = 0, 
	    bool modal = false, WFlags f = 0);

    /**
     * Return the number of pixels you shall use between a 
     * dialog edge and the outermost widget(s) according to the KDE standard.
     **/
    static int marginHint();

    /**
     * Return the number of pixels you shall use between 
     * widgets inside a dialog according to the KDE standard.
     */
    static int spacingHint();

    /**
     * Resize every layout manager used in @p widget and its nested children.
     *
     * @param margin The new layout margin.
     * @param margin The new layout spacing.
     */
    static void resizeLayout( QWidget *widget, int margin, int spacing );

    /**
     * Reszie every layout associated with @p lay and its children. 
     *
     * @param margin The new layout margin
     * @param margin The new layout spacing
     */
    static void resizeLayout( QLayoutItem *lay, int margin, int spacing );

  public slots:
    /**
     * Make a KDE compliant caption.
     * 
     * @param caption Your caption. Do @p not include the application name
     * in this string. It will be added automatically according to the KDE
     * standard.
     */
    virtual void setCaption( const QString &caption );

    /**
     * Make a plain caption without any modifications.
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
     * Emitted when the margin size and/or spacing size 
     * have changed.
     *
     * Use @ref marginHint() and  @ref spacingHint() in your slot
     * to get the new values.
     */
    void layoutHintChanged();

  private:
    static int mMarginSize;
    static int mSpacingSize;

    class KDialogPrivate;
    KDialogPrivate *d;

};

#endif // __KDIALOG_H
