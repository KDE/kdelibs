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
#ifndef _KMESSAGEBOX_H_
#define _KMESSAGEBOX_H_

#include <qstring.h>

#include <kguiitem.h>
#include <kstdguiitem.h>

class QWidget;
class QStringList;

 /**
  * Easy message dialog box.
  *
  * Provides convenience functions for some i18n'ed standard dialogs.
  *
  * The text in message boxes is wrapped automatically. The text may either
  * be plaintext or richtext. If the text is plaintext, a newline-character
  * may be used to indicate the end of a paragraph.
  *
  * @author Waldo Bastian (bastian@kde.org)
  * @version $Id$
  */
class KMessageBox
{
public:
  /**
   * Button types.
   **/
 enum ButtonCode
 {
   Ok = 1,
   Cancel = 2,
   Yes = 3,
   No = 4,
   Continue = 5
 };

 enum DialogType
 {
   QuestionYesNo = 1,
   WarningYesNo = 2,
   WarningContinueCancel = 3,
   WarningYesNoCancel = 4,
   Information = 5,
   // Reserved for: SSLMessageBox = 6
   Sorry = 7,
   Error = 8,
   QuestionYesNoCancel = 9
 };

 /**
  * @li Notify Emit a KNotifyClient event (Not yet implemented)
  * @li AllowLink The message may contain links.
  */
 enum OptionsType
 {
   Notify = 1,
   AllowLink = 2
 };

 /**
  * Display a simple "question" dialog.
  *
  * @param parent  If @p parent is 0, then the message box becomes an
  *                application-global modal dialog box. If @p parent is a
  *                widget, the message box becomes modal relative to parent.
  * @param text    Message string.
  * @param caption Message box title. The application name is added to
  *                the title. The default title is i18n("Question").
  * @param buttonYes The text for the first button.
  *                  The default is i18n("&Yes").
  * @param buttonNo  The text for the second button.
  *                  The default is i18n("&No").
  * @param dontAskAgainName If provided, a checkbox is added with which
  *                further confirmation can be turned off.
  *                The string is used to lookup and store the setting
  *                in the applications config file.
  * @param options  see @ref OptionsType
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
                          const KGuiItem &buttonYes = KStdGuiItem::yes(),
                          const KGuiItem &buttonNo =  KStdGuiItem::no(),
                          const QString &dontAskAgainName = QString::null,
                          int options = Notify);
 /**
  * Display a simple "question" dialog.
  *
  * @param parent  If @p parent is 0, then the message box becomes an
  *                application-global modal dialog box. If @p parent is a
  *                widget, the message box becomes modal relative to parent.
  * @param text    Message string.
  * @param caption Message box title. The application name is added to
  *                the title. The default title is i18n("Question").
  * @param buttonYes The text for the first button.
  *                  The default is i18n("&Yes").
  * @param buttonNo  The text for the second button.
  *                  The default is i18n("&No").
  * @param dontAskAgainName If provided, a checkbox is added with which
  *                further confirmation can be turned off.
  *                The string is used to lookup and store the setting
  *                in the applications config file.
  * @param options  see @ref OptionsType
  *
  * @return  'Yes' is returned if the Yes-button is pressed. 'No' is returned
  *          if the No-button is pressed.
  *
  * To be used for questions like "Do you want to discard the message or save it for later?",
  *
  * The default button is "Yes". Pressing "Esc" selects "Cancel".
  *
  * NOTE: The cancel button will always have the i18n'ed text '&Cancel'.
  */

  static int questionYesNoCancel(QWidget *parent,
                          const QString &text,
                          const QString &caption = QString::null,
                          const KGuiItem &buttonYes = KStdGuiItem::yes(),
                          const KGuiItem &buttonNo  = KStdGuiItem::no(),
                          const QString &dontAskAgainName = QString::null,
                          int options = Notify);

 /**
  * Display a "question" dialog with a listbox to show information to the user
  *
  * @param parent  If @p parent is 0, then the message box becomes an
  *                application-global modal dialog box. If @p parent is a
  *                widget, the message box becomes modal relative to parent.
  * @param text    Message string.
  * @param strlist List of strings to be written in the listbox. If the list is
  *                empty, it doesn't show any listbox, working as questionYesNo.
  * @param caption Message box title. The application name is added to
  *                the title. The default title is i18n("Question").
  * @param buttonYes The text for the first button.
  *                  The default is i18n("&Yes").
  * @param buttonNo  The text for the second button.
  *                  The default is i18n("&No").
  * @param dontAskAgainName If provided, a checkbox is added with which
  *                further confirmation can be turned off.
  *                The string is used to lookup and store the setting
  *                in the applications config file.
  * @param options  see @ref OptionsType
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
                          const KGuiItem &buttonYes = KStdGuiItem::yes(),
                          const KGuiItem &buttonNo = KStdGuiItem::no(),
                          const QString &dontAskAgainName = QString::null,
                          int options = Notify);
 /**
  * Display a "warning" dialog.
  *
  * @param parent  If @p parent is 0, then the message box becomes an
  *                application-global modal dialog box. If @p parent is a
  *                widget, the message box becomes modal relative to parent.
  * @param text    Message string.
  * @param caption Message box title. The application name is added to
  *                the title. The default title is i18n("Warning").
  * @param buttonYes The text for the first button.
  *                  The default is i18n("&Yes").
  * @param buttonNo  The text for the second button.
  *                  The default is i18n("&No").
  * @param dontAskAgainName If provided, a checkbox is added with which
  *                further confirmation can be turned off.
  *                The string is used to lookup and store the setting
  *                in the applications config file.
  * @param options  see @ref OptionsType
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
                         const KGuiItem &buttonYes = KStdGuiItem::yes(),
                         const KGuiItem &buttonNo = KStdGuiItem::no(),
                         const QString &dontAskAgainName = QString::null,
                         int options = Notify);

 /**
  * Display a "warning" dialog.
  *
  * @param parent  If @p parent is 0, then the message box becomes an
  *                application-global modal dialog box. If @p parent is a
  *                widget, the message box becomes modal relative to parent.
  * @param text    Message string.
  * @param caption Message box title. The application name is added to
  *                the title. The default title is i18n("Warning").
  * @param buttonContinue The text for the first button.
  * @param dontAskAgainName If provided, a checkbox is added with which
  *                further confirmation can be turned off.
  *                The string is used to lookup and store the setting
  *                in the applications config file.
  * @param options  see @ref OptionsType
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
                         const QString &caption = QString::null,
                         const KGuiItem &buttonContinue = KStdGuiItem::cont(),
                         const QString &dontAskAgainName = QString::null,
                         int options = Notify);


 /**
  * Display a "warning" dialog with a listbox to show information to the user.
  *
  * @param parent  If @p parent is 0, then the message box becomes an
  *                application-global modal dialog box. If @p parent is a
  *                widget, the message box becomes modal relative to parent.
  * @param text    Message string.
  * @param strlist List of strings to be written in the listbox. If the
  *                list is empty, it doesn't show any listbox, working
  *                as warningContinueCancel.
  * @param caption Message box title. The application name is added to
  *                the title. The default title is i18n("Warning").
  * @param buttonContinue The text for the first button.
  * @param dontAskAgainName If provided, a checkbox is added with which
  *                further confirmation can be turned off.
  *                The string is used to lookup and store the setting
  *                in the applications config file.
  *                The setting is stored in the "Notification Messages" group.
  *
  * @param options  see @ref OptionsType
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
 static int warningContinueCancelList(QWidget *parent,
                         const QString &text,
                         const QStringList &strlist,
                         const QString &caption = QString::null,
                         const KGuiItem &buttonContinue = KStdGuiItem::cont(),
                         const QString &dontAskAgainName = QString::null,
                         int options = Notify);


 /**
  * Display a Yes/No/Cancel "warning" dialog.
  *
  * @param parent  If @p parent is 0, then the message box becomes an
  *                application-global modal dialog box. If @p parent is a
  *                widget, the message box becomes modal relative to parent.
  * @param text    Message string.
  * @param caption Message box title. The application name is added to
  *                the title. The default title is i18n("Warning").
  * @param buttonYes The text for the first button.
  *                  The default is i18n("&Yes").
  * @param buttonNo  The text for the second button.
  *                  The default is i18n("&No").
  * @param dontAskAgainName If provided, a checkbox is added with which
  *                further questions can be turned off. If turned off
  *                all questions will be automatically answered with the
  *                last answer (either Yes or No).
  *                The string is used to lookup and store the setting
  *                in the applications config file.
  * @param options  see @ref OptionsType
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
                                const KGuiItem &buttonYes = KStdGuiItem::yes(),
                                const KGuiItem &buttonNo = KStdGuiItem::no(),
                                const QString &dontAskAgainName = QString::null,
                                int options = Notify);

 /**
  * Display an "Error" dialog.
  *
  * @param parent  If @p parent is 0, then the message box becomes an
  *                application-global modal dialog box. If @p parent is a
  *                widget, the message box becomes modal relative to parent.
  * @param text    Message string.
  * @param caption Message box title. The application name is added to
  *                the title. The default title is i18n("Error").
  * @param options  see @ref OptionsType
  *
  * Your program messed up and now it's time to inform the user.
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
                    const QString &caption = QString::null,
                    int options = Notify);

 /**
  * Displays an "Error" dialog with a "Details >>" button.
  *
  * @param parent  If @p parent is 0, then the message box becomes an
  *                application-global modal dialog box. If @p parent is a
  *                widget, the message box becomes modal relative to parent.
  * @param text    Message string.
  * @param details Detailed message string.
  * @param caption Message box title. The application name is added to
  *                the title. The default title is i18n("Error").
  * @param options  see @ref OptionsType
  *
  * Your program messed up and now it's time to inform the user.
  * To be used for important things like "Sorry, I deleted your hard disk."
  *
  * The @p details message can conatin additional information about
  * the problem and can be shown on request to advanced/interested users.
  *
  * If your program detects the action specified by the user is somehow
  * not allowed, this should never be reported with error(). Use sorry()
  * instead to explain to the user that this action is not allowed.
  *
  * The default button is "&OK". Pressing "Esc" selects the OK-button.
  *
  * NOTE: The OK button will always have the i18n'ed text '&OK'.
  */
  static void detailedError(QWidget *parent,
                    const QString &text,
                    const QString &details,
                    const QString &caption = QString::null,
                    int options = Notify);

  /**
   * Like @ref detailedError
   *
   * This function will return immediately, the messagebox will be shown
   * once the application enters an event loop and no other messagebox
   * is being shown.
   *
   * Note that if the parent gets deleted, the messagebox will not be
   * shown.
   */
  static void queuedDetailedError( QWidget *parent,
                    const QString &text,
                    const QString &details,
                    const QString &caption = QString::null);

 /**
  * Display an "Sorry" dialog.
  *
  * @param parent  If @p parent is 0, then the message box becomes an
  *                application-global modal dialog box. If @p parent is a
  *                widget, the message box becomes modal relative to parent.
  * @param text    Message string.
  * @param caption Message box title. The application name is added to
  *                the title. The default title is i18n("Sorry").
  * @param options  see @ref OptionsType
  *
  * Either your program messed up and asks for understanding
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
                    const QString &caption = QString::null,
                    int options = Notify);

 /**
  * Displays a "Sorry" dialog with a "Details >>" button.
  *
  * @param parent  If @p parent is 0, then the message box becomes an
  *                application-global modal dialog box. If @p parent is a
  *                widget, the message box becomes modal relative to parent.
  * @param text    Message string.
  * @param details Detailed message string.
  * @param caption Message box title. The application name is added to
  *                the title. The default title is i18n("Sorry").
  * @param options  see @ref OptionsType
  *
  * Either your program messed up and asks for understanding
  * or your user did something stupid.
  *
  * To be used for small problems like
  * "Sorry, I can't find the file you specified."
  *
  * And then @p details can contain something like
  * "foobar.txt was not found in any of
  *  the following directories:
  *  /usr/bin,/usr/local/bin,/usr/sbin"
  *
  * The default button is "&OK". Pressing "Esc" selects the OK-button.
  *
  * NOTE: The ok button will always have the i18n'ed text '&OK'.
  */

  static void detailedSorry(QWidget *parent,
                    const QString &text,
                    const QString &details,
                    const QString &caption = QString::null,
                    int options = Notify);

 /**
  * Display an "Information" dialog.
  *
  * @param parent  If @p parent is 0, then the message box becomes an
  *                application-global modal dialog box. If @p parent is a
  *                widget, the message box becomes modal relative to parent.
  * @param text    Message string.
  * @param caption Message box title. The application name is added to
  *                the title. The default title is i18n("Information").
  * @param dontShowAgainName If provided, a checkbox is added with which
  *                further notifications can be turned off.
  *                The string is used to lookup and store the setting
  *                in the applications config file.
  *                The setting is stored in the "Notification Messages" group.
  * @param options  see @ref OptionsType
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
                          const QString &dontShowAgainName = QString::null,
                          int options = Notify);

 /**
  * Display an "Information" dialog with a listbox.
  *
  * @param parent  If @p parent is 0, then the message box becomes an 
  *                application-global modal dialog box. If @p parent is a
  *                widget, the message box becomes modal relative to parent.
  * @param text    Message string.
  * @param strlist List of strings to be written in the listbox. If the 
  *                list is empty, it doesn't show any listbox, working 
  *                as information.
  * @param caption Message box title. The application name is added to
  *                the title. The default title is i18n("Information").
  * @param dontShowAgainName If provided, a checkbox is added with which
  *                further notifications can be turned off.
  *                The string is used to lookup and store the setting
  *                in the applications config file.
  *                The setting is stored in the "Notification Messages" group.
  * @param options  see @ref OptionsType
  *
  *
  * Your program wants to tell the user something.
  * To be used for things like:
  * "The following bookmarks have been rearranged:"
  *
  * The default button is "&OK". Pressing "Esc" selects the OK-button.
  *
  *  NOTE: The OK button will always have the i18n'ed text '&OK'.
  */
   
  static void informationList(QWidget *parent, 
			      const QString &text, 
			      const QStringList & strlist,
			      const QString &caption = QString::null,
			      const QString &dontShowAgainName = QString::null, 
			      int options = Notify);

  /**
   * Enable all messages which have been turned off with the
   * @p dontShowAgainName feature of the information dialog.
   */
  static void enableAllMessages();

  /**
   * Display an "About" dialog.
   *
   * @param parent  If @p parent is 0, then the message box becomes an
   *                application-global modal dialog box. If @p parent is a
   *                widget, the message box becomes modal relative to parent.
   * @param text    Message string.
   * @param caption Message box title. The application name is added to
   *                the title. The default title is i18n("About <appname>").
   * @param options  see @ref OptionsType
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
		    const QString& caption = QString::null,
                    int options = Notify);

    /**
     * Alternate method to show a messagebox:
     *
     * @param parent  If @p parent is 0, then the message box becomes an
     *                application-global modal dialog box. If @p parent is a
     *                widget, the message box becomes modal relative to parent.
     * @param type type of message box: QuestionYesNo, WarningYesNo, WarningContinueCancel...
     * @param text Message string.
     * @param caption Message box title.
     * @param buttonYes The text for the first button.
     *                  The default is i18n("&Yes").
     * @param buttonNo  The text for the second button.
     *                  The default is i18n("&No").
     * @param options  see @ref OptionsType
     * Note: for ContinueCancel, buttonYes is the continue button and buttonNo is unused.
     *       and for Information, none is used.
     * @return a button code, as defined in KMessageBox.
     */
    static int messageBox( QWidget *parent, DialogType type, const QString &text,
                    const QString &caption = QString::null,
                    const KGuiItem &buttonYes = KStdGuiItem::yes(),
                    const KGuiItem &buttonNo = KStdGuiItem::no(),
                    int options = Notify);

    /*
     * Like @ref messageBox
     *
     * Only for message boxes of type Information, Sorry or Error.
     *
     * This function will return immediately, the messagebox will be shown
     * once the application enters an event loop and no other messagebox
     * is being shown.
     *
     * Note that if the parent gets deleted, the messagebox will not be
     * shown.
     */
    static void queuedMessageBox( QWidget *parent,
                    DialogType type, const QString &text,
                    const QString &caption = QString::null );

};

#endif
