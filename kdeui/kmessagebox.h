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
 * Revision 1.9  1999/12/11 17:20:36  antlarr
 * Changed the QStringList questionYesNo method name to questionYesNoList
 * Also reimplemented the original questionYesNo to call questionYesNoList with
 * an empty list which reduces the sources and keeps the look
 * I've changed the only two places that used this to reflect the changes:
 * the KMessageBox test and Konqueror
 *
 * Revision 1.8  1999/11/27 21:30:47  antlarr
 * Added a new questionYesNo which displays a "question" dialog with a listbox
 * to show information to the user (see the mail in kde-core-devel for more info)
 *
 * Revision 1.8  1999/11/27 22:20:41  antlarr
 * Added a new questionYesNo which displays a "question" dialog with a listbox 
 * to show information to the user (see the docs for more info)
 *
 * Revision 1.7  1999/11/11 15:03:41  waba
 * WABA:
 * * Uses KDialogBase for implementation
 * * Some additions according to the KDE Style Guide
 *
 * Revision 1.6  1999/09/12 13:35:04  espensa
 * I have had problems getting the action button to be properly underlined.
 * The changes should fix this once and for all. There were errors in the
 * "box->setButtonText()" (index errors)
 *
 * One minor typo removed in kmessagebox.h as well.
 *
 * Revision 1.5  1999/08/24 13:16:17  waba
 * WABA: Adding enums, fixing default for warningYesNo
 *
 * Revision 1.4  1999/08/15 15:41:47  porten
 * fixed documentation typos
 *
 * Revision 1.3  1999/08/15 14:03:00  porten
 * capitalize "OK" in documentation as well.
 *
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
class QStringList;

 /** 
  * Easy message dialog box. 
  *
  * Provides convenience functions for some i18n'ed standard dialogs.
  *
  * @author Waldo Bastian (bastian@kde.org)
  * @version $Id$
  */
class KMessageBox
{
public:
 enum { Ok = 1, Cancel = 2, Yes = 3, No = 4, Continue = 5 };
 /**
  * Display a simple "question" dialog. 
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
  * @return  'Yes' is returned if the Yes-button is pressed. 'No' is returned
  *          if the No-button is pressed.
  * 
  * To be used for questions like "Do you have a printer?"
  *
  * The default button is "Yes". Pressing "Esc" selects "No".
  */
  
 static int questionYesNo(QWidget *parent,
                          const QString &text,
                          const QString &caption = QString::null,
                          const QString &buttonYes = QString::null,  
                          const QString &buttonNo = QString::null);
 /**
  * Display a "question" dialog with a listbox to show information to the user 
  *
  * @param parent  If parent is 0, then the message box becomes an 
  *                application-global modal dialog box. If parent is a
  *                widget, the message box becomes modal relative to parent.
  * @param text    Message string. May contain newlines.
  * @param strlist List of strings to be written in the listbox. If the list is
  *                empty, it doesn't show any listbox, working as questionYesNo.
  * @param caption Message box title. The application name is added to
  *                the title. The default title is i18n("Question").
  * @param buttonYes The text for the first button. 
  *                  The default is i18n("&Yes").
  * @param buttonNo  The text for the second button. 
  *                  The default is i18n("&No").
  *
  * @return  'Yes' is returned if the Yes-button is pressed. 'No' is returned
  *          if the No-button is pressed.
  * 
  * To be used for questions like "Do you really want to delete these files?"
  * And show the user exactly which files are going to be deleted in case
  * he presses "Yes"
  *
  * The default button is "Yes". Pressing "Esc" selects "No".
  */
 
 static int questionYesNoList(QWidget *parent,
                          const QString &text,
                          const QStringList &strlist,
                          const QString &caption = QString::null,
                          const QString &buttonYes = QString::null,
                          const QString &buttonNo = QString::null);
 /**
  * Display a "warning" dialog. 
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
  * @return  @p Yes is returned if the Yes-button is pressed. @p No is returned
  *          if the No-button is pressed.
  *
  * To be used for questions "Shall I update your configuration?"
  * The text should explain the implication of both options.
  *
  * The default button is "No". Pressing "Esc" selects "No".
  */
 static int warningYesNo(QWidget *parent, 
                         const QString &text,
                         const QString &caption = QString::null,
                         const QString &buttonYes = QString::null,  
                         const QString &buttonNo = QString::null);

 /**
  * Display a "warning" dialog. 
  *
  * @param parent  If parent is 0, then the message box becomes an 
  *                application-global modal dialog box. If parent is a
  *                widget, the message box becomes modal relative to parent.
  * @param text    Message string. May contain newlines.
  * @param caption Message box title. The application name is added to
  *                the title. The default title is i18n("Warning").
  * @param buttonContinue The text for the first button. 
  *
  * The second button always has the text "Cancel".
  *
  * @return  @p Continue is returned if the Continue-button is pressed. 
  *          @p Cancel is returned if the Cancel-button is pressed.
  *
  * To be used for questions like "You are about to Print. Are you sure?"
  * the continueButton should then be labeled "Print".
  *
  * The default button is buttonContinue. Pressing "Esc" selects "Cancel".
  */
 static int warningContinueCancel(QWidget *parent, 
                         const QString &text,
                         const QString &caption,
                         const QString &buttonContinue);

 /**
  * Display a Yes/No/Cancel "warning" dialog. 
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
  * @return  @p Yes is returned if the Yes-button is pressed. @p No is returned
  *          if the No-button is pressed. @p Cancel is retunred if the Cancel-
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
  * Display an "Error" dialog. 
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
   * not allowed, this should never be reported with error(). Use sorry()
   * instead to explain to the user that this action is not allowed. 
   *
   * The default button is "&OK". Pressing "Esc" selects the OK-button.
   *
   * NOTE: The OK button will always have the i18n'ed text '&OK'.
   */
   
  static void error(QWidget *parent, 
                    const QString &text, 
                    const QString &caption = QString::null);

 /**
  * Display an "Sorry" dialog. 
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
  * "Sorry, I can't find the file you specified."
  *
  * The default button is "&OK". Pressing "Esc" selects the OK-button.
  *
  * NOTE: The ok button will always have the i18n'ed text '&OK'.
  */
  
  static void sorry(QWidget *parent, 
                    const QString &text,
                    const QString &caption = QString::null);

 /**
  * Display an "Information" dialog. 
  *
  * @param parent  If parent is 0, then the message box becomes an 
  *                application-global modal dialog box. If parent is a
  *                widget, the message box becomes modal relative to parent.
  * @param text    Message string. May contain newlines.
  * @param caption Message box title. The application name is added to
  *                the title. The default title is i18n("Information").
  * @param dontShowAgainName If provided, a checkbox is added with which
  *                further notifications can be turned off.
  *                The string is used to lookup and store the setting
  *                in the applications config file.
  *
  *
  * Your program wants to tell the user something.
  * To be used for things like:
  * "Your bookmarks have been rearranged."
  *
  * The default button is "&OK". Pressing "Esc" selects the OK-button.
  *
  *  NOTE: The OK button will always have the i18n'ed text '&OK'.
  */
   
  static void information(QWidget *parent, 
                          const QString &text, 
                          const QString &caption = QString::null,
                          const QString &dontShowAgainName = QString::null);

  /**
   * Enable all messages which have been turned off with the
   * @p dontShowAgainName feature of the information dialog.
   */
  static void enableAllMessages();
  
  /**
   * Display an "About" dialog. 
   *
   * @param parent  If parent is 0, then the message box becomes an 
   *                application-global modal dialog box. If parent is a
   *                widget, the message box becomes modal relative to parent.
   * @param text    Message string. May contain newlines.
   * @param caption Message box title. The application name is added to
   *                the title. The default title is i18n("About <appname>").
   *
   *
   * Your program wants to show some general information about the application
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
