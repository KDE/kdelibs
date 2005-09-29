/* This file is part of the KDE libraries

   Copyright (C) 1999 Preston Brown <pbrown@kde.org>

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
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/
#ifndef __klineeditdlg_h__
#define __klineeditdlg_h__

class KLineEdit;
class QValidator;

#include <kdialogbase.h>

/**
 * @deprecated
 * Please use KInputDialog instead.
 *
 * Dialog for user to enter a single line of text.
 *
 * @version $Id$
 * @author David Faure <faure@kde.org>, layout management by Preston Brown <pbrown@kde.org>
 */

class KDEUI_EXPORT_DEPRECATED KLineEditDlg : public KDialogBase
{
  Q_OBJECT
public:
  /**
   * Create a dialog that asks for a single line of text. _value is
   * the initial value of the line. _text appears as label on top of
   * the entry box. If the internal line edit has an associated
   * QValidator set, the OK button is disabled as long as the
   * validator doesn't return Acceptable. If there's no validator, the
   * OK button is enabled whenever the line edit isn't empty.
   *
   * If you want to accept empty input, make a trivial QValidator that
   * always returns Acceptable, e.g. QRegExpValidator with a
   * regexp of ".*".
   *
   * @param _text      Text of the label
   * @param _value     Initial value of the inputline
   * @param parent     Parent widget for the line edit dialog
   */
  KLineEditDlg( const QString& _text, const QString& _value, QWidget *parent ) KDE_DEPRECATED;
  virtual ~KLineEditDlg();

  /**
   * @return the value the user entered
   */
  QString text() const;

  /**
   * @return the line edit widget
   */
  KLineEdit *lineEdit() const { return edit; }

  /**
   * Static convenience function to get a textual input from the user.
   *
   * @param text      Text of the label
   * @param value     Initial value of the inputline
   * @param ok         this bool will be set to true if user pressed "Ok"
   * @param validator Validator to be stuffed into the line edit.
   * @param parent    The parent widget
   */
  static QString getText(const QString &text, const QString& value,
		 bool *ok, QWidget *parent, QValidator *validator=0 ) KDE_DEPRECATED;

  /**
   * Static convenience function to get a textual input from the user.
   * This method includes a caption, and has (almost) the same API as QInputDialog::getText
   * (no echo mode, we have KPasswordDialog).
   *
   * @param caption   Caption of the dialog
   * @param text      Text of the label
   * @param value     Initial value of the inputline
   * @param ok         this bool will be set to true if user pressed "Ok"
   * @param parent    The parent widget for this text input dialog
   * @param validator Validator to be stuffed into the line edit.
   */
  static QString getText(const QString &caption, const QString &text,
                         const QString& value=QString::null,
                         bool *ok=0, QWidget *parent=0,
			 QValidator *validator=0) KDE_DEPRECATED;

public slots:
  /**
   * Clears the edit widget
   */
  void slotClear();

protected slots:
  /**
   * Enables and disables the OK button depending on the state
   * returned by the lineedit's QValidator.
   */
  void slotTextChanged( const QString& );

protected:
  /**
   * The line edit widget
   */
  KLineEdit *edit;
protected:
  virtual void virtual_hook( int id, void* data );
private:
  class KLineEditDlgPrivate;
  KLineEditDlgPrivate* d;
};

#endif
