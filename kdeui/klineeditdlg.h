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
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/
#ifndef __klineeditdlg_h__
#define __klineeditdlg_h__

#include <qlineedit.h>

#include <kdialogbase.h>
#include "kurlcompletion.h"

/**
 * Dialog for user to enter a single line of text.
 * Includes support for URL completion with @ref KURLCompletion.
 *
 * @version $Id$
 * @author David Faure <faure@kde.org>, layout management by Preston Brown <pbrown@kde.org>
 */

class KLineEditDlg : public KDialogBase
{
  Q_OBJECT
public:
  /**
   * Create a dialog that asks for a single line of text. _value is the initial
   * value of the line. _text appears as label on top of the entry box.
   * 
   * @param _file_mode if set to TRUE, the editor widget will provide command
   *                   completion ( Ctrl-S and Ctrl-D ), and a browse button
   *                   for the local file system.
   */
  KLineEditDlg( const QString&_text, const QString&_value, 
		QWidget *parent, bool _file_mode = FALSE );
  virtual ~KLineEditDlg();

  /**
   * @return the value the user entered
   */
  QString text() { return edit->text(); }
    
public slots:
  /**
   * Clears the edit widget
   */
  void slotClear();

protected slots:
  /**
   * use a QFileDialog to find a file to view.
   */
  void slotBrowse();

protected:
  /**
   * The line edit widget
   */
  QLineEdit *edit;

  /**
   * Completion helper ..
   */
  KURLCompletion * completion;
};

#endif
