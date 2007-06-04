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

#ifndef kio_pass_word_dialog_h
#define kio_pass_word_dialog_h

#include <kio/kio_export.h>
#include <kpassworddialog.h>


namespace KIO {

/**
 * @deprecated  use KPasswordDialog
 * A dialog for requesting a login and a password from the end user.
 *
 * KIO-Slave authors are encouraged to use SlaveBase::openPassDlg
 * instead of directly instantiating this dialog.
 * @short dialog for requesting login and password from the end user
 */
class KIO_EXPORT_DEPRECATED PasswordDialog : public KPasswordDialog
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
     */
    PasswordDialog( const QString& prompt, const QString& user,
                    bool enableKeep = false, bool modal=true,
                    QWidget* parent=0 );

    /**
     * Destructor
     */
    ~PasswordDialog();


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

};

}

#endif
