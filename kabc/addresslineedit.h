/*
    This file is part of libkabc.
    Copyright (c) 2002 Helge Deller <deller@gmx.de>
                  2002 Lubos Lunak <llunak@suse.cz>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#ifndef KABC_ADDRESSLINEEDIT_H
#define KABC_ADDRESSLINEEDIT_H
// $Id$

#include <qobject.h>
#include <qptrlist.h>
#include <qtimer.h>

#include "klineedit.h"
#include "kcompletion.h"

namespace KABC {

class LdapSearch;

/**
 * A lineedit with LDAP and kabc completion
 * 
 * This lineedit is supposed to be used wherever the user types email addresses
 * and might want a completion. You can simply use it as a replacement for 
 * @ref KLineEdit or @ref QLineEdit.
 *
 * You can enable or disable the lineedit at any time.
 *
 * @see AddressLineEdit::enableCompletion()
 */
class AddressLineEdit : public KLineEdit
{
  Q_OBJECT
public:
  AddressLineEdit(QWidget* parent, bool useCompletion = true, 
		const char *name = 0L);
  virtual ~AddressLineEdit();

  /**
   * Reimplented for internal reasons.
   * @ see KLineEdit::setFont()
   */
  virtual void setFont( const QFont& );

public slots:
  void undo();
  /**
   * Set cursor to end of line.
   */
  void cursorAtEnd();
  /**
   * Toggle completion.
   */
  void enableCompletion( bool enable );

protected:
  /**
   * Always call AddressLineEdit::loadAddresses() as the first thing.
   * Use addAddress() to add addresses.
   */
  virtual void loadAddresses();
  void addAddress( const QString& );
  virtual void keyPressEvent(QKeyEvent*);
  virtual void dropEvent(QDropEvent *e);
  virtual void paste();
  virtual void insert(const QString &t);
  virtual void mouseReleaseEvent( QMouseEvent * e );
  void doCompletion(bool ctrlT);
  void init();

private slots:
  void slotCompletion() { doCompletion(false); }
  void slotPopupCompletion( const QString& );
  void slotStartLDAPLookup();
  void slotLDAPSearchData( const QStringList& );

private:
  void startLoadingLDAPEntries();
  void stopLDAPLookup();
  QStringList addresses();
  QStringList removeMailDupes( const QStringList& adrs );

  QString m_previousAddresses;
  bool m_useCompletion;
  bool m_completionInitialized;
  bool m_smartPaste;
  QString m_typedText;

  static bool s_addressesDirty;
  static KCompletion *s_completion;
  static QTimer *s_LDAPTimer;
  static LdapSearch *s_LDAPSearch;
  static QString *s_LDAPText;
  static AddressLineEdit *s_LDAPLineEdit;
private:
  class AddressLineEditPrivate* d;
};

}

#endif		/* KABC_ADDRESSLINEEDIT_H */
