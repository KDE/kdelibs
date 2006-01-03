// vi: ts=8 sts=4 sw=4
/* This file is part of the KDE libraries
   Copyright (C) 1998 Pietro Iglio <iglio@fub.it>
   Copyright (C) 1999,2000 Geert Jansen <jansen@kde.org>
   Copyright (C) 2004,2005 Andrew Coles <andrew_coles@yahoo.co.uk>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/
#ifndef KPASSDLG_H
#define KPASSDLG_H

#include <qstring.h>
#include <qlineedit.h>
#include <kdialogbase.h>

class QLabel;
class QGridLayout;
class QWidget;

/**
 * @short A safe password input widget.
 * @author Geert Jansen <geertj@kde.org>
 *
 * The widget uses the user's global "echo mode" setting.
 */

class KDEUI_EXPORT KPasswordEdit
    : public QLineEdit
{
    Q_OBJECT

public:
    enum EchoModes { OneStar, ThreeStars, NoEcho };

    /**
     * Constructs a password input widget using the user's global "echo mode" setting.
     */
    KPasswordEdit(QWidget *parent=0);
    // KDE4: either of the two must go! add default values for parameters

    /**
     * Constructs a password input widget using echoMode as "echo mode".
     * Note that echoMode is a QLineEdit::EchoMode.
     * @since 3.0
     */
    KPasswordEdit(EchoMode echoMode, QWidget *parent);

    /**
     * Constructs a password input widget using echoMode as "echo mode".
     * Note that echoMode is a KPasswordEdit::EchoModes.
     * @since 3.2
     */
    KPasswordEdit(EchoModes echoMode, QWidget *parent);

    /**
     * Destructs the widget.
     */
    ~KPasswordEdit();

    /**
     * Returns the password. The memory is freed in the destructor
     * so you should make a copy.
     */
    const char *password() const { return m_Password; }

    /**
     * Erases the current password.
     */
    void erase();

    static const int PassLen;

    /**
     * Set the current maximum password length.  If a password longer than the limit
     * specified is currently entered, it is truncated accordingly.
     *
     * The length is capped to lie between 0 and 199 inclusive.
     *
     * @param newLength: The new maximum password length
     * @since 3.4
     */
    void setMaxPasswordLength(int newLength);

    /**
     * Returns the current maximum password length.
     * @since 3.4
     */
    int maxPasswordLength() const;

public slots:
    /**
     * Reimplementation
     */
    virtual void insert( const QString &);

protected:
    virtual void keyPressEvent(QKeyEvent *);
    virtual void focusInEvent(QFocusEvent *e);
    virtual bool event(QEvent *e);

private:
    void init();
    void showPass();

    char *m_Password;
    int m_EchoMode, m_Length;
};


/**
 * @short A password input dialog.
 *
 * This dialog asks the user to enter a password. The functions you're
 * probably interested in are the static methods, getPassword() and
 * getNewPassword().
 *
 * <b>Usage example</b>\n
 *
 * \code
 * QCString password;
 * int result = KPasswordDialog::getPassword(parent, password, i18n("Prompt message"));
 * if (result == KPasswordDialog::Accepted)
 *     use(password);
 * \endcode
 *
 * \image html kpassworddialog.png "KDE Password Dialog"
 *
 * <b>Security notes:</b>\n
 *
 * Keeping passwords in memory can be a potential security hole. You should
 * handle this situation with care.
 *
 * @li You may want to use disableCoreDump() to disable core dumps.
 * Core dumps are dangerous because they are an image of the process memory,
 * and thus include any passwords that were in memory.
 *
 * @li You should delete passwords as soon as they are not needed anymore.
 * The functions getPassword() and getNewPassword() return the
 * password as a QCString. I believe this is safer than a QString. A QString
 * stores its characters internally as 16-bit wide values, so conversions are
 * needed, both for creating the QString and by using it. The temporary
 * memory used for these conversion is probably not erased. This could lead
 * to stray  passwords in memory, even if you think you erased all of them.
 *
 * @author Geert Jansen <jansen@kde.org>
 */

class KDEUI_EXPORT KPasswordDialog
    : public KDialogBase
{
    Q_OBJECT

public:
    /**
     * This enum distinguishes the two operation modes of this dialog:
     */
    enum Types {
        /**
         * The user is asked to enter a password.
         */
        Password,

        /**
         * The user is asked to enter a password and to confirm it
         * a second time. This is usually used when the user
         * changes his password.
         */
        NewPassword
    };

    /**
     * Constructs a password dialog.
     *
     * @param type: if NewPassword is given here, the dialog contains two
     *        input fields, so that the user must confirm his password
     *        and possible typos are detected immediately.
     * @param enableKeep: if true, a check box is shown in the dialog
     *        which allows the user to keep his password input for later.
     * @param extraBttn: allows to show additional buttons, KDialogBase.
     * @param parent Passed to lower level constructor.
     *
     * @since 3.0
     */
    KPasswordDialog(Types type, bool enableKeep, QFlags<ButtonCode> extraBttn, QWidget *parent=0);


    /**
     * Construct a password dialog. Essentially the same as above but allows the
     * icon in the password dialog to be set via @p iconName.
     * @param type if NewPassword is given here, the dialog contains two
     * input fields, so that the user must confirm his password
     * and possible typos are detected immediately
     * @param enableKeep: if true, a check box is shown in the dialog
     *        which allows the user to keep his password input for later.
     * @param extraBttn: allows to show additional buttons.
     * @param iconName the name of the icon to be shown in the dialog. If empty,
     * a default icon is used
     * @param parent Passed to lower level constructor.
     * @since 3.3
     */
	KPasswordDialog(Types type, bool enableKeep, QFlags<ButtonCode> extraBttn, const QString& iconName,
                    QWidget *parent = 0);

    /**
     * Destructs the password dialog.
     */
    virtual ~KPasswordDialog();

    /**
     * Sets the password prompt.
     */
    void setPrompt(const QString &prompt);

    /**
     * Returns the password prompt.
     */
    QString prompt() const;

    /**
     * Adds a line of information to the dialog.
     */
    void addLine(const QString &key, const QString &value);

    /**
     * Allow empty passwords? - Default: false
     * @since 3.4
     */
    void setAllowEmptyPasswords(bool allowed);

    /**
     * Allow empty passwords?
     * @since 3.4
     */
    bool allowEmptyPasswords() const;

    /**
     * Minimum acceptable password length.
     * Default: If empty passwords are forbidden, 1;
     *          Otherwise, 0.
     *
     * @param minLength: The new minimum password length
     * @since 3.4
     */
    void setMinimumPasswordLength(int minLength);

    /**
     * Minimum acceptable password length.
     * @since 3.4
     */
    int minimumPasswordLength() const;

    /**
     * Maximum acceptable password length.  Limited to 199.
     * Default: No limit, i.e. -1
     *
     * @param maxLength: The new maximum password length.
     * @since 3.4
     */
    void setMaximumPasswordLength(int maxLength);

    /**
     * Maximum acceptable password length.
     * @since 3.4
     */
    int maximumPasswordLength() const;

    /**
     * Password length that is expected to be reasonably safe.
     *
     * Default: 8 - the standard UNIX password length
     *
     * @param reasonableLength: The new reasonable password length.
     * @since 3.4
     */
    void setReasonablePasswordLength(int reasonableLength);

    /**
     * Password length that is expected to be reasonably safe.
     * @since 3.4
     */
    int reasonablePasswordLength() const;

    /**
     * Set the password strength level below which a warning is given
     * Value is in the range 0 to 99. Empty passwords score 0;
     * non-empty passwords score up to 100, depending on their length and whether they
     * contain numbers, mixed case letters and punctuation.
     *
     * Default: 1 - warn if the password has no discernable strength whatsoever
     * @param warningLevel: The level below which a warning should be given.
     * @since 3.4
     */
    void setPasswordStrengthWarningLevel(int warningLevel);

    /**
     * Password strength level below which a warning is given
     * @since 3.4
     */
    int passwordStrengthWarningLevel() const;

    /**
     * Returns the password entered. The memory is freed in the destructor,
     * so you should make a copy.
     */
    const char *password() const { return m_pEdit->password(); }

    /**
     * Clears the password input field. You might want to use this after the
     * user failed to enter the correct password.
     * @since 3.3
     */
    void clearPassword();

    /**
     * Returns true if the user wants to keep the password.
     */
    bool keep() const { return m_Keep; }

    /**
     * Pops up the dialog, asks the user for a password, and returns it.
     *
     * @param parent The widget the dialog belongs too
     * @param password The password is returned in this reference parameter.
     * @param prompt A prompt for the password. This can be a few lines of
     * information. The text is word broken to fit nicely in the dialog.
     * @param keep Enable/disable a checkbox controlling password keeping.
     * If you pass a null pointer, or a pointer to the value 0, the checkbox
     * is not shown. If you pass a pointer to a nonzero value, the checkbox
     * is shown and the result is stored in *keep.
     * @return Result code: Accepted or Rejected.
     */
    static int getPassword(QWidget *parent,QByteArray &password, const QString &prompt, int *keep=0L);

    /**
     * Pops up the dialog, asks the user for a password and returns it. The
     * user has to enter the password twice to make sure it was entered
     * correctly.
     *
     * @param parent The widget the dialog belongs too
     * @param password The password is returned in this reference parameter.
     * @param prompt A prompt for the password. This can be a few lines of
     * information. The text is word broken to fit nicely in the dialog.
     * @return Result code: Accepted or Rejected.
     */
    static int getNewPassword(QWidget *parent,QByteArray &password, const QString &prompt);

    /**
     * Static helper function that disables core dumps.
     */
    static void disableCoreDumps();
	
	virtual void accept();

protected slots:
    void slotKeep(bool);

protected:

    /**
     * Virtual function that can be overridden to provide password
     * checking in derived classes. It should return @p true if the
     * password is valid, @p false otherwise.
     */
    virtual bool checkPassword(const char *) { return true; }


private slots:
  void enableOkBtn();

private:
    void init();
    void erase();

    int m_Keep, m_Type, m_Row;
    QLabel *m_pHelpLbl;
    QGridLayout *m_pGrid;
    QWidget *m_pMain;
    KPasswordEdit *m_pEdit, *m_pEdit2;

protected:
    virtual void virtual_hook( int id, void* data );
private:
    class KPasswordDialogPrivate;
    KPasswordDialogPrivate* const d;
};

#endif // KPASSDLG_H

