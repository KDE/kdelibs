/*  This file is part of the KDE libraries
    Copyright (C) 1999 Waldo Bastian (bastian@kde.org)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; version 2 
    of the License.

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
 * Revision 1.2  1999/08/15 10:50:30  kulow
 * adding KMessageBox::about which uses the KDE icon instead of the Qt Information
 * icon - very cool! :)
 *
 * Revision 1.1  1999/07/25 19:38:58  waba
 * WABA: Added some i18n'ed qmessagebox'es for convenience
 *
 *
 */

#ifndef _KMESSAGEBOX_H_
#define _KMESSAGEBOX_H_

#include <qstring.h>

class QWidget;

 /** 
  * Easy MessageBox Dialog. 
  *
  * Provides convenience functions for some i18n'ed standard dialogs.
  *
  * @author Waldo Bastian (bastian@kde.org)
  * @version $Id$
  */
class KMessageBox
{
public:
 /**
  * Displays a simple "question" dialog. 
  *
  * @param parent  If parent is 0, then the message box becomes an 
  *                application-global modal dialog box. If parent is a
  *                widget, the message box becomes modal relative to parent.
  * @param text    Message string. May contain newlines.
  * @param caption Message box title. The application name is added to
  *                the title. The default title is i18n("Question").
  * @param buttonYes The text for the first button. 
  *                  The default is i18n("&Yes").
  * @param buttonNo  The text for the second button. 
  *                  The default is i18n("&No").
  *
  * @return  0 is returned if the Yes-button is pressed. 1 is returned
  *          if the No-button is pressed.
  * 
  * To be used for conmfirmation on questions like "Shall I do xyz?"
  *
  * The default button is "Yes". Pressing "Esc" selects "No".
  */
  
 static int questionYesNo(QWidget *parent,
                          const QString &text,
                          const QString &caption = QString::null,
                          const QString &buttonYes = QString::null,  
                          const QString &buttonNo = QString::null);

 /**
  * Displays a "warning" dialog. 
  *
  * @param parent  If parent is 0, then the message box becomes an 
  *                application-global modal dialog box. If parent is a
  *                widget, the message box becomes modal relative to parent.
  * @param text    Message string. May contain newlines.
  * @param caption Message box title. The application name is added to
  *                the title. The default title is i18n("Question").
  * @param buttonYes The text for the first button. 
  *                  The default is i18n("&Yes").
  * @param buttonNo  The text for the second button. 
  *                  The default is i18n("&No").
  *
  * @return  0 is returned if the Yes-button is pressed. 1 is returned
  *          if the No-button is pressed.
  *
  * To be used for questions "Are you sure you want to do this?"
  * The text should explain the implication of choosing 'Yes'.
  *
  * The default button is "No". Pressing "Esc" selects "No".
  */
 static int warningYesNo(QWidget *parent, 
                         const QString &text,
                         const QString &caption = QString::null,
                         const QString &buttonYes = QString::null,  
                         const QString &buttonNo = QString::null);

 /**
  * Displays a Yes/No/Cancel "warning" dialog. 
  *
  * @param parent  If parent is 0, then the message box becomes an 
  *                application-global modal dialog box. If parent is a
  *                widget, the message box becomes modal relative to parent.
  * @param text    Message string. May contain newlines.
  * @param caption Message box title. The application name is added to
  *                the title. The default title is i18n("Warning").
  * @param buttonYes The text for the first button. 
  *                  The default is i18n("&Yes").
  * @param buttonNo  The text for the second button. 
  *                  The default is i18n("&No").
  *
  * @return  0 is returned if the Yes-button is pressed. 1 is returned
  *          if the No-button is pressed. 2 is retunred if the Cancel-
  *          button is pressed.
  *
  * To be used for questions "Do you want to save your changes?"
  * The text should explain the implication of choosing 'No'.
  *
  * The default button is "Yes". Pressing "Esc" selects "Cancel"
  *
  * NOTE: The cancel button will always have the i18n'ed text '&Cancel'.
  */

  static int warningYesNoCancel(QWidget *parent,
                                const QString &text,
                                const QString &caption = QString::null,
                                const QString &buttonYes = QString::null,  
                                const QString &buttonNo = QString::null);

 /**
  * Displays an "Error" dialog. 
  *
  * @param parent  If parent is 0, then the message box becomes an 
  *                application-global modal dialog box. If parent is a
  *                widget, the message box becomes modal relative to parent.
  * @param text    Message string. May contain newlines.
  * @param caption Message box title. The application name is added to
  *                the title. The default title is i18n("Error").
  *
   * Your program fucked up and now it's time to inform the user.
   * To be used for important things like "Sorry, I deleted your hard disk."
   * 
   * If your program detects the action specified by the user is somehow
   * not allowed, this should never be reported as with error(). Use sorry()
   * instead to explain to the user that is action is not allowed. 
   *
   * The default button is "&OK". Pressing "Esc" selects the OK-button.
   *
   * NOTE: The ok button will always have the i18n'ed text '&OK'.
   */
   
  static void error(QWidget *parent, 
                    const QString &text, 
                    const QString &caption = QString::null);

 /**
  * Displays an "Sorry" dialog. 
  *
  * @param parent  If parent is 0, then the message box becomes an 
  *                application-global modal dialog box. If parent is a
  *                widget, the message box becomes modal relative to parent.
  * @param text    Message string. May contain newlines.
  * @param caption Message box title. The application name is added to
  *                the title. The default title is i18n("Sorry").
  *
  * Either your program fucked up and asks for understanding
  * or your user did something stupid.
  *
  * To be used for small problems like 
  * "Sorry, I can't find the file you specified.."
  *
  * The default button is "&OK". Pressing "Esc" selects the OK-button.
  *
  * NOTE: The ok button will always have the i18n'ed text '&OK'.
  */
  
  static void sorry(QWidget *parent, 
                    const QString &text,
                    const QString &caption = QString::null);

 /**
  * Displays an "Information" dialog. 
  *
  * @param parent  If parent is 0, then the message box becomes an 
  *                application-global modal dialog box. If parent is a
  *                widget, the message box becomes modal relative to parent.
  * @param text    Message string. May contain newlines.
  * @param caption Message box title. The application name is added to
  *                the title. The default title is i18n("Information").
  *
  *
  * Your program wants to tell the user something.
  * To be used for things like 
  * "Your bookmarks have been rearranged."
  *
  * The default button is "&OK". Pressing "Esc" selects the OK-button.
  *
  *  NOTE: The ok button will always have the i18n'ed text '&OK'.
  */
   
  static void information(QWidget *parent, 
                          const QString &text, 
                          const QString &caption = QString::null);

  
  /**
   * Displays an "About" dialog. 
   *
   * @param parent  If parent is 0, then the message box becomes an 
   *                application-global modal dialog box. If parent is a
   *                widget, the message box becomes modal relative to parent.
   * @param text    Message string. May contain newlines.
   * @param caption Message box title. The application name is added to
   *                the title. The default title is i18n("About <appname>").
   *
   *
   * Your program wants to show some general informations about the application
   * like the authors's names and email addresses.
   *
   * The default button is "&OK". 
   *
   *  NOTE: The ok button will always have the i18n'ed text '&OK'.
   */
  static void about(QWidget *parent,
		    const QString& text,
		    const QString& caption = QString::null);
};

#endif
