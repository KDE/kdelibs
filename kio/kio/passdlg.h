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
   not, write to the Free Software Foundation, Inc., 59 Temple
   Place - Suite 330, Boston, MA 02111-1307, USA.
*/

#ifndef __kio_pass_dlg_h__
#define __kio_pass_dlg_h__

#include <kdialogbase.h>

class QGridLayout;

namespace KIO {

/**
 * A dialog for requesting a login and a password from the end user.
 *
 * KIO-Slave authors are encouraged to use @ref SlaveBase::openPassDlg
 * instead of directly instantiating this dialog.
 */
class PasswordDialog : public KDialogBase 
{
    Q_OBJECT

public:
    /**
     * Create a password dialog.
     *
     * @param prompt        instructional text to be shown.
     * @param user          username, if known initially.
     * @param enableKeep    if true, makes password persistent until KDE is shutdown.
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
     */
    void setPrompt( const QString& prompt );

    /**
     * Adds a comment line to the dialog.
     *
     * This function allows you to add one additional comment
     * line to this widget.  Calling this fucntion after a
     * comment has already been added will not have any effect.
     *
     * @param label       label for comment (ex:"Command:")
     * @param comment     the actual comment text.
     */
    void addCommentLine( const QString& label, const QString comment );

    /**
     * Returns the password entered by the user.
     */
    QString password() const;
    
    /**
     * Returns the username entered by the user.
     */
    QString username() const;

    /**
     * Determines whether supplied authorization should
     * persist even after the application has been closed.
     */
    bool keepPassword() const;

    /**
     * Sets the username field read-only and sets the
     * focus to the password field.
     *
     * @param readOnly
     */
    void setUserReadOnly( bool readOnly );

    /**
     * @deprecated. Use @ref setUserReadOnly(bool).
     */
    void setEnableUserField( bool enable, bool=false ) {
	setUserReadOnly( !enable );
    };

    /**
     * A convienence static method for obtaining authorization
     * information from the end user.
     *
     *
     * @param user          username
     * @param pass          password
     * @param keep          pointer to flag that indicates
     * @param prompt        text to display to user.
     * @param readOnly      make the username field read-only.
     * @param caption       set the title bar to given text.
     * @param comment       extra comment to display to user.
     * @param label         optinal label for extra comment.
     *
     * @return Accept/Reject based on the user choice.
     */
    static int getNameAndPassword( QString& user, QString& pass, bool* keep,
                                   const QString& prompt = QString::null,
                                   bool readOnly = false,
                                   const QString& caption = QString::null,
                                   const QString& comment = QString::null,
                                   const QString& label = QString::null );

private slots:
    void slotKeep( bool );

private:
    void init( const QString&, const QString&, bool );

protected:
    virtual void virtual_hook( int id, void* data );
private:
    struct PasswordDialogPrivate;
    PasswordDialogPrivate* d;
};

};

#endif
