/* vi: ts=8 sts=4 sw=4
 *
 * $Id$
 *
 * This file is part of the KDE project, module kdesu.
 * Copyright (C) 1998 Pietro Iglio <iglio@fub.it>
 * Copyright (C) 1999,2000 Geert Jansen <jansen@kde.org>
 */

#ifndef __KPassDlg_h_included__
#define __KPassDlg_h_included__

#include <qstring.h>
#include <qlineedit.h>
#include <qevent.h>

#include <kdialogbase.h>

class QLabel;


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
    KPasswordEdit(QWidget *parent=0, const char *name=0);
    ~KPasswordEdit();

    /** 
     * Returns the password. The memory is freed in the destructor
     * so you should make a copy.
     */
    const char *password() { return m_Password; }

    /** Erase the current password. */
    void erase();

    static const int PassLen = 100;
    enum EchoModes { OneStar, ThreeStars, NoEcho };

protected:
    virtual void keyPressEvent(QKeyEvent *);

private:
    void showPass();

    char *m_Password;
    int m_EchoMode, m_Length;
};


/**
 * This dialog asks the user to enter a password. The functions you're 
 * probably interested in are @ref #getPassword and @ref #getNewPassword.
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
 * @li You may want to use @ref #disableCoreDump to disable core dumps. 
 * Core dumps are dangerous because they are an image of the process memory, 
 * and thus include any passwords that were in memory.
 *
 * @li You should delete passwords as soon as they are not needed anymore.
 * The functions @ref #getPassword and @ref #getNewPassword return the 
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
    KPasswordDialog(int type, QString prompt, QString command=QString::null, 
	    bool enableKeep=false, int extraBttn=0);
    virtual ~KPasswordDialog();

    /** Set the password prompt. */
    void setPrompt(QString prompt);

    /** 
     * Returns the password entered. The memory is freed in the destructor,
     * so you should make a copy.
     */
    const char *password() const { return m_pEdit1->password(); }

    /** Returns true if the user wants to keep the password. */
    bool keep() const { return m_Keep; }

    /**
     * Pops up the dialog, asks the user for a password, and returns it.
     *
     * @param password The password is returned in this reference parameter.
     * @param prompt A prompt for the password. This can be a few lines of
     * information. The text is word broken to fit nicely in the dialog.
     * @param command Extra feedback to the user, telling him the command 
     * the password is intended for.
     * @param keep Enable/disable a checkbox controlling password keeping.
     * If you pass a null pointer, or a pointer to the value 0, the checkbox 
     * is not shown. If you pass a pointer to a nonzero value, the checkbox 
     * is shown and the result is stored in *keep.
     * @return Result code: Accepted or Rejected.
     */
    static int getPassword(QCString &password, QString prompt, 
	    QString command=QString::null, int *keep=0L);

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

    enum Types { Password, newPassword };

protected slots:
    void slotOk();
    void slotCancel();
    void slotKeep(bool);

protected:

    /** Virtual function that can be overridden to provide password
     * checking in derived classes. */
    virtual bool checkPassword(const char *) { return true; }

private:
    void erase();

    int m_Keep, m_Type;
    QString m_Command;
    QLabel *m_pHelpLbl;
    KPasswordEdit *m_pEdit1, *m_pEdit2;
};


#endif // __KPassDlg_h_included__
