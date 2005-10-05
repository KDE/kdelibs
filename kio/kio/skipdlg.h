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
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef __kio_skip_dlg__
#define __kio_skip_dlg__

#include <kdelibs_export.h>
#include <kdialog.h>

class QPushButton;
class QWidget;

namespace KIO {

  enum SkipDlg_Result { S_SKIP = 1, S_AUTO_SKIP = 2, S_CANCEL = 0 };

  KIO_EXPORT SkipDlg_Result open_SkipDlg( bool _multi, const QString& _error_text = QString::null );

/**
 * @internal
 */
class KIO_EXPORT SkipDlg : public KDialog
{
  Q_OBJECT
public:
  SkipDlg( QWidget *parent, bool _multi, const QString& _error_text, bool _modal = false );
  ~SkipDlg();

protected:
  QPushButton *b0;
  QPushButton *b1;
  QPushButton *b2;

  bool modal;

public slots:
  void b0Pressed();
  void b1Pressed();
  void b2Pressed();

signals:
  void result( SkipDlg *_this, int _button );
};

}
#endif
