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
 * Revision 1.7  1998/01/03 19:47:30  kulow
 * changed the defaults for yesNo and co. It's no longer "Yes" and co, but
 * 0 to enable translation in case.
 * This is source and binary compatible. It's just, that it will behave
 * differently after recompiling (as far I understood the C++ language ;)
 *
 * Revision 1.6  1997/11/04 17:48:51  kulow
 * hmm, gcc accepts QPixmap & icon = 0, Sparc CC won't. So I made it .. = QPixmap()
 *
 * Revision 1.5  1997/10/21 20:45:03  kulow
 * removed all NULLs and replaced it with 0L or "".
 * There are some left in mediatool, but this is not C++
 *
 * Revision 1.4  1997/10/16 11:15:27  torben
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


 /** 
  * Enhanced MessageBox Dialog. 
  *
  * Provides a message box with icons and up to four configurable
  * buttons and internationalized button text.
  *
  * Several static functions that perform common message box functions
  * are also provided for convenience.
  *
  * @author Alexander Sanda (alex@darkstar.ping.at)
  * @version $Id$
  */
class KMsgBox : public QDialog
{
    Q_OBJECT

public:

    /**
    * Icon styles for message box customization. One of these should be
    * passed in the type parameter of the message box constructor
    * and static functions.
    *
    * @see KMsgBox::KMsgBox
    */
    enum IconStyle { INFORMATION = 1, EXCLAMATION = 2, 
		STOP = 4, QUESTION = 8 };

    /**
    * Possible options for the default button. The first button
    * is set to the default if no button is explicitly selected.
    *
    * @see KMsgBox::KMsgBox
    */
    enum DefaultButton { DB_FIRST = 16, DB_SECOND = 32, 
    		DB_THIRD = 64, DB_FOURTH = 128};
    
    /** 
     * The generic constructor for a KMsgBox widget. 
     *
     * @param parent 	Parent widget
     * @param caption	Message box title
     * @param message	Message string, center aligned by default. 
     *			May contain newlines.
     * @param type	Selects message box options. This value is 
     *			a constant from @ref ::DefaultButton and/or
     *			a constant from @ref ::IconStyle or-ed together.
     * @param b1text..	Button captions. Up to four may be specified, only
     *			specified buttons will be displayed.
     *
     * @see ::DefaultButton ::IconStyle
     */
    KMsgBox ( QWidget *parent = 0, const char *caption = 0, 
		    const char *message = 0, int type = INFORMATION,
		    const char *b1text = 0, const char *b2text = 0, 
		    const char *b3text = 0, const char *b4text = 0);

    /**
    * Destructor.
    */
    ~KMsgBox();

    /** 
     * Displays a modal yes/no message box. 
     * The icon is set to a question mark.
     *
     * @param parent 	the parent widget
     * @param caption	the message box title
     * @param message	the message in the dialog (eg the question the 
     *			user is to be asked)
     * @param type	Selects message box options. This value is 
     *			a constant from @ref ::DefaultButton and/or
     *			a constant from @ref ::IconStyle or-ed together.
     * @param yes	the text for the "Yes" button. defaults to "Yes", or
     *			its translation in the current user's locale if
     *			available.
     * @param no	the text for the "No" button. defaults to "No", or
     *			its translation in the current user's locale if
     *			available.
     *
     * @return 1 if yes is clicked, 0 otherwise.
     */
    static int yesNo( QWidget *parent = 0, const char *caption = 0, 
		    const char *message = 0, int type = 0,
		    const char *yes = 0, const char *no = 0);

    /** 
     * Displays a modal yes/no/cancel message box.
     * The icon is set to a question mark.
     *
     * @param parent 	the parent widget
     * @param caption	the message box title
     * @param message	the message in the dialog (eg the question the user is 
     *			to be asked)
     * @param type	Selects message box options. This value is 
     *			a constant from @ref ::DefaultButton and/or
     *			a constant from @ref ::IconStyle or-ed together.
     * @param yes	the text for the "Yes" button. defaults to "Yes", or
     *			its translation in the current user's locale if
     *			available.
     * @param no	the text for the "No" button. defaults to "No", or
     *			its translation in the current user's locale if
     *			available.
     * @param cancel	the text for the "Cancel" button. defaults to "Cancel", 
     *			or its translation in the current user's locale
     *			if available.
     *
     * @return 1, 2 or 3 if yes, no or cancel are clicked respectively.
     */
    static int yesNoCancel( QWidget *parent = 0, 
    		const char *caption = 0, const char *message = 0, 
		int type = 0, const char *yes = 0, const char *no = 0, 
		const char *cancel = 0);

    /** 
     * Displays a modal message box with one button. An "Information"
     * icon is displayed.
     *
     * @param parent 	the parent widget
     * @param caption	the message box title
     * @param message	the message in the dialog (eg the question the user is 
     *			to be asked)
     * @param type	Selects message box options. This value is 
     *			a constant from @ref ::DefaultButton and/or
     *			a constant from @ref ::IconStyle or-ed together.
     * @param btext	the text for the "Ok" button. defaults to "Ok", or
     *			its translation in the current user's locale if
     *			available.
     * @return		1 if the Ok button is clicked
     */
    static int message( QWidget *parent = 0, const char *caption = 0, 
		    const char *message = 0, int type = 0,
		    const char *btext = 0);

private:

    enum {B_SPACING = 10, B_WIDTH = 80};

    QLabel      *msg, *picture;
    QPushButton *b1, *b2, *b3, *b4;
    QFrame      *f1;

    int		nr_buttons;
    int         w, h, h1, text_offset;

    void        calcOptimalSize();

public slots:

    void        b1Pressed();
    void        b2Pressed();
    void        b3Pressed();
    void        b4Pressed();
};

#endif

