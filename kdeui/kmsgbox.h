/* This file is part of the KDE libraries
    Copyright (C) 1997 Alexander Sanda (alex@darkstar.ping.at)

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
/*
 * $Id$
 *
 * $Log$
 * Kalle: Copyright headers
 * kdoctoolbar removed
 *
 * Revision 1.3  1997/07/24 21:06:07  kalle
 * Kalle:
 * KToolBar upgraded to newtoolbar 0.6
 * KTreeList has rubberbanding now
 * Patches for SGI
 *
 * Revision 1.2  1997/05/08 22:53:20  kalle
 * Kalle:
 * KPixmap gone for good
 * Eliminated static objects from KApplication and HTML-Widget
 *
 * Revision 1.1.1.1  1997/04/13 14:42:42  cvsuser
 * Source imported
 *
 * Revision 1.1.1.1  1997/04/09 00:28:09  cvsuser
 * Sources imported
 *
 * Revision 1.1  1997/03/15 22:41:21  kalle
 * Initial revision
 *
 * Revision 1.3.2.1  1997/01/10 19:48:32  alex
 * public release 0.1
 *
 * Revision 1.3  1997/01/10 19:44:33  alex
 * *** empty log message ***
 *
 * Revision 1.2.4.1  1997/01/10 16:46:33  alex
 * rel 0.1a, not public
 *
 * Revision 1.2  1997/01/10 13:05:52  alex
 * *** empty log message ***
 *
 * Revision 1.1.1.1  1997/01/10 13:05:21  alex
 * imported
 *
 */

// kmsgbox.h

#ifndef _KMSGBOX_H_
#define _KMSGBOX_H_

#include <qobjdefs.h>
#include <qdialog.h>
#include <qpushbt.h>
#include <qlabel.h>
#include <qframe.h>
#include <qpixmap.h>


/// KMsgBox
/** A message box API for the KDE project. KMsgBox provides a Windows - lookalike message- or
 error box with icons and up to 4 configurable buttons.
 */

class KMsgBox : public QDialog
{
    Q_OBJECT
public:

    enum {INFORMATION = 1, EXCLAMATION = 2, STOP = 4, QUESTION = 8};
    enum {DB_FIRST = 16, DB_SECOND = 32, DB_THIRD = 64, DB_FOURTH = 128};
    
    /// Constructor
    /** The generic constructor for a KMsgBox Widget. All parameters have a default value of 0.
     @Doc:
     \begin{itemize}
     \item { \tt The parent widget }
     \item { \tt A caption (title) for the message box. }
     \item { \tt a message string. The default alignment is centered. The string may contain
     newline characters.}
     \item { \tt Flags: This parameter is responsible for several behaviour options of the
     message box. See below for valid constans for this parameter.}
     \item { \tt Up to 4 button strings (b1text to b4text). You have to specify at least b1text.
     Unspecified buttons will not appear in the message box, therefore you can control the
     number of buttons in the box.}
     \end{itemize}
     */
    KMsgBox(QWidget *parent = 0, const char *caption = 0, const char *message = 0, int flags = INFORMATION,
            const char *b1text = 0, const char *b2text = 0, const char *b3text = 0, const char *b4text = 0);

    // the default destructor - only for some compilers
    ~KMsgBox();
    /// yesNo
    /** A static member function. Can be used to create simple message boxes with a
     single function call. This box has 2 buttons which default to "Yes" and "No". The icon
     shows a question mark. The appearance can be manipulated with the {\bf type} parameter.
     */
    
    static int yesNo(QWidget *parent = 0, const char *caption = 0, const char *message = 0, int type = 0,
                     const char *yes = "Yes", const char *no = "No");

    /// yesNoCancel
    /** A static member function for creating a three-way message box. The box has three
     buttons (defaulting to "Yes", "No" and "Cancel") and a question-mark icon. The {\bf type}
     Parameter can be used to change the default behaviour.
     */
    
    static int yesNoCancel(QWidget *parent = 0, const char *caption = 0, const char *message = 0, int type = 0,
                           const char *yes = "Yes", const char *no = "No", const char *cancel = "Cancel");

    /// message
    /** A static member function. Creates a simple message box with only one button ("Ok")
     and a information-icon. The icon can be changed by using another value for the {\bf type}
     parameter.
     */
    
    static int message(QWidget *parent = 0, const char *caption = 0, const char *message = 0, int type = 0,
                       const char *btext = "Ok");


private:
    enum {B_SPACING = 10, B_WIDTH = 80};
    QLabel      *msg, *picture;
    QPushButton *b1, *b2, *b3, *b4;
    QFrame      *f1;
    int nr_buttons;
    int         w, h, h1, text_offset;
    void        calcOptimalSize();
    void        resizeEvent(QResizeEvent *);
                       const QPixmap & icon = (char *)NULL);
    void        initMe(const char *caption, const char *message, const char *b1text,
                       const char *b2text, const char *b3text, const char *b4text,
                       const QPixmap & icon = QPixmap());

public slots:
    void        b1Pressed();
    void        b2Pressed();
    void        b3Pressed();
    void        b4Pressed();
};

#endif

