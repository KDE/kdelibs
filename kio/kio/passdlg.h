/* This file is part of the KDE libraries
   Copyright (C) 2000 David Faure <faure@kde.org>
   Copyright (C) 2000 Dawit Alemayehu <adawit@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with this library; see the file COPYING.LIB.  If
   not, write to the Free Software Foundation, Inc., 51 Franklin Street,
   Fifth Floor, Boston, MA 02110-1301, USA.
*/

#ifndef __kio_pass_dlg_h__
#define __kio_pass_dlg_h__

#include <kdialogbase.h>

class QGridLayout;

namespace KIO {

/**
 * A dialog for requesting a login and a password from the end user.
 *
 * KIO-Slave authors are encouraged to use SlaveBase::openPassDlg
 * instead of directly instantiating this dialog.
 * @short dialog for requesting login and password from the end user
 */
class KIO_EXPORT PasswordDialog : public KDialogBase
{
    Q_OBJECT

public:
    /**
     * Create a password dialog.
     *
     * @param prompt        instructional text to be shown.
     * @param user          username, if known initially.
     * @param enableKeep    if true, shows checkbox that makes password persistent until KDE is shutdown.
     * @param modal         if true, the dialog will be modal (default:true).
     * @param parent        the parent widget (default:NULL).
     * @param name          the dialog name (default:NULL).
     */
    PasswordDialog( const QString& prompt, const QString& user,
                    bool enableKeep = false, bool modal=true,
                    QWidget* parent=0, const char* name=0 );

    /**
     * Destructor
     */
    ~PasswordDialog();

    /**
     * Sets the prompt to show to the user.
     * @param prompt        instructional text to be shown.
     */
    void setPrompt( const QString& prompt );

    /**
     * Adds a comment line to the dialog.
     *
     * This function allows you to add one additional comment
     * line to this widget.  Calling this function after a
     * comment has already been added will not have any effect.
     *
     * @param label       label for comment (ex:"Command:")
     * @param comment     the actual comment text.
     */
    void addCommentLine( const QString& label, const QString comment );

    /**
     * Returns the password entered by the user.
     * @return the password
     */
    QString password() const;

    /**
     * Returns the username entered by the user.
     * @return the user name
     */
    QString username() const;

    /**
     * Determines whether supplied authorization should
     * persist even after the application has been closed.
     * @return true to keep the password
     */
    bool keepPassword() const;

    /**
     * Check or uncheck the "keep password" checkbox.
     * This can be used to check it before showing the dialog, to tell
     * the user that the password is stored already (e.g. in the wallet).
     * enableKeep must have been set to true in the constructor.
     */
    void setKeepPassword( bool b );

    /**
     * Sets the username field read-only and sets the
     * focus to the password field.
     *
     * @param readOnly true to set the user field to read-only
     */
    void setUserReadOnly( bool readOnly );

    /**
     * Presets the password.
     * @param password the password to set
     * @since 3.1
     */
    void setPassword( const QString& password );

    /**
     * Presets a number of login+password pairs that the user can choose from.
     * The passwords can be empty if you simply want to offer usernames to choose from.
     * This is incompatible with setUserReadOnly(true).
     * @param knownLogins map of known logins: the keys are usernames, the values are passwords.
     * @since 3.4
     */
    void setKnownLogins( const QMap<QString, QString>& knownLogins );

    /**
     * A convienence static method for obtaining authorization
     * information from the end user.
     *
     *
     * @param user          username
     * @param pass          password
     * @param keep          pointer to flag that indicates whether to keep password (can be null)
     * @param prompt        text to display to user.
     * @param readOnly      make the username field read-only.
     * @param caption       set the title bar to given text.
     * @param comment       extra comment to display to user.
     * @param label         optinal label for extra comment.
     *
     * @return Accepted/Rejected based on the user choice.
     */
    static int getNameAndPassword( QString& user, QString& pass, bool* keep,
                                   const QString& prompt = QString(),
                                   bool readOnly = false,
                                   const QString& caption = QString(),
                                   const QString& comment = QString(),
                                   const QString& label = QString() );

private slots:
    void slotKeep( bool );
    void slotActivated( const QString& userName );

private:
    void init( const QString&, const QString&, bool );

protected:
    virtual void virtual_hook( int id, void* data );
private:
    struct PasswordDialogPrivate;
    PasswordDialogPrivate* const d;
};

}

#endif
