// vi: ts=8 sts=4 sw=4 
/* This file is part of the KDE libraries
   Copyright (C) 1998 Pietro Iglio <iglio@fub.it>
   Copyright (C) 1999,2000 Geert Jansen <jansen@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/
#ifndef __KPassDlg_h_included__
#define __KPassDlg_h_included__

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

class KPasswordEdit
    : public QLineEdit
{
    Q_OBJECT

public:
    enum EchoModes { OneStar, ThreeStars, NoEcho };

    /**
     * Constructs a password input widget using the user's global "echo mode" setting.
     */
    KPasswordEdit(QWidget *parent=0, const char *name=0);
    /**
     * Constructs a password input widget using echoMode as "echo mode".
     */
    KPasswordEdit(EchoMode echoMode, QWidget *parent, const char *name);
    /**
     * @deprecated
     * Creates a password input widget using echoMode as "echo mode".
     */
    KPasswordEdit(QWidget *parent, const char *name, int echoMode);
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
 * This dialog asks the user to enter a password. The functions you're 
 * probably interested in are the static methods, @ref #getPassword() and
 * @ref #getNewPassword().
 *
 * @sect Usage example
 *
 * <pre>
 * QCString password;
 * int result = KPasswordDialog::getPassword(password, i18n("Password"));
 * if (result == KPasswordDialog::Accepted)
 *     use(password);
 * </pre>
 *
 * @sect Security notes
 *
 * Keeping passwords in memory can be a potential security hole. You should
 * handle this situation with care.
 *
 * @li You may want to use @ref #disableCoreDump() to disable core dumps. 
 * Core dumps are dangerous because they are an image of the process memory, 
 * and thus include any passwords that were in memory.
 *
 * @li You should delete passwords as soon as they are not needed anymore.
 * The functions @ref #getPassword() and @ref #getNewPassword() return the 
 * password as a QCString. I believe this is safer than a QString. A QString 
 * stores its characters internally as 16-bit wide values, so conversions are 
 * needed, both for creating the QString and by using it. The temporary 
 * memory used for these conversion is probably not erased. This could lead 
 * to stray  passwords in memory, even if you think you erased all of them.
 *
 * @short A password input dialog.
 * @author Geert Jansen <jansen@kde.org>
 */

class KPasswordDialog
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
     * @param extraBttn: allows to show additional buttons, @ref KDialogBase.
     */
    KPasswordDialog(Types type, bool enableKeep, int extraBttn,
                    QWidget *parent=0, const char *name=0);
    /**
     * @deprecated Variant of the previous constructor without the
     * possibility to specify a parent. May be removed in KDE 4.0
     */
    KPasswordDialog(int type, QString prompt, bool enableKeep=false, 
	    int extraBttn=0);
    /**
     * Destructs the password dialog.
     */
    virtual ~KPasswordDialog();

    /**
     * Sets the password prompt.
     */
    void setPrompt(QString prompt);
    /**
     * Returns the password prompt.
     */
    QString prompt() const;

    /**
     * Adds a line of information to the dialog.
     */
    void addLine(QString key, QString value);
    /** 
     * Returns the password entered. The memory is freed in the destructor,
     * so you should make a copy.
     */
    const char *password() const { return m_pEdit->password(); }

    /** Returns true if the user wants to keep the password. */
    bool keep() const { return m_Keep; }

    /**
     * Pops up the dialog, asks the user for a password, and returns it.
     *
     * @param password The password is returned in this reference parameter.
     * @param prompt A prompt for the password. This can be a few lines of
     * information. The text is word broken to fit nicely in the dialog.
     * @param keep Enable/disable a checkbox controlling password keeping.
     * If you pass a null pointer, or a pointer to the value 0, the checkbox 
     * is not shown. If you pass a pointer to a nonzero value, the checkbox 
     * is shown and the result is stored in *keep.
     * @return Result code: Accepted or Rejected.
     */
    static int getPassword(QCString &password, QString prompt, int *keep=0L);

    /** 
     * Pops up the dialog, asks the user for a password and returns it. The
     * user has to enter the password twice to make sure it was entered
     * correctly.
     *
     * @param password The password is returned in this reference parameter.
     * @param prompt A prompt for the password. This can be a few lines of
     * information. The text is word broken to fit nicely in the dialog.
     * @return Result code: Accepted or Rejected.
     */
    static int getNewPassword(QCString &password, QString prompt);

    /** Static helper funtion that disables core dumps. */
    static void disableCoreDumps();

protected slots:
    void slotOk();
    void slotCancel();
    void slotKeep(bool);

protected:

    /** Virtual function that can be overridden to provide password
     * checking in derived classes. It should return @p true if the
     * password is valid, @p false otherwise. */
    virtual bool checkPassword(const char *) { return true; }

private:
    void init();
    void erase();

    int m_Keep, m_Type, m_Row;
    QLabel *m_pHelpLbl;
    QGridLayout *m_pGrid;
    QWidget *m_pMain;
    KPasswordEdit *m_pEdit, *m_pEdit2;

    class KPasswordDialogPrivate;
    KPasswordDialogPrivate *d;
};


#endif // __KPassDlg_h_included__
