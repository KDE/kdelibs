/* This file is part of the KDE libraries
   Copyright (C) 2000 David Faure <faure@kde.org>

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
#ifndef __kio_pass_dlg_h__
#define __kio_pass_dlg_h__

#include <qstring.h>
#include <qlineedit.h>

#include <kdialogbase.h>

namespace KIO {

  /**
   * A dialog for requesting a login and a password to the user.
   * KIO-Slave authors should have a look at @ref SlaveBase::openPassDlg instead.
   */
  class PassDlg : public KDialog {
    Q_OBJECT
  public:
    /**
     * Create a password dialog
     * @param parent the parent widget
     * @param name the dialog name
     * @param modal if true, the dialog will be modal
     * @param wflags window flags, passed to QDialog
     * @param head the text to show in the dialog, on top of the two fields
     * @param user user name, if known initially
     * @param pass password, if known initially
     */
    PassDlg( QWidget* parent, const char* name, bool modal, WFlags wflags,
	         const QString& head, const QString& user, const QString& pass );

    /**
     * @return the password entered by the user
     */
    QString password() { return m_pPass->text(); }

    /**
     * @return the login entered by the user
     */
    QString user() { return m_pUser->text(); }

    /**
     * Enables/disables the username field.
     */
    void setEnableUserField( bool, bool gainsFocus = false );

  private:
    QLineEdit* m_pPass;
    QLineEdit* m_pUser;

  };

};

#endif


