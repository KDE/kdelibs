/* This file is part of the KDE project
   Copyright (C) 1999 David Faure <faure@kde.org>

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
#ifndef _KBUGREPORT_H__
#define _KBUGREPORT_H__

#include <kdialogbase.h>

class QMultiLineEdit;
class QLineEdit;
class QHButtonGroup;
class KProcess;
class KAboutData;
class KBugReportPrivate;

class KBugReport : public KDialogBase
{
  Q_OBJECT
public:
  KBugReport( QWidget * parent = 0L, bool modal=true );
  virtual ~KBugReport();

protected slots:
  virtual void slotConfigureEmail();
  virtual void slotSetFrom();
  virtual void slotUrlClicked(const QString &);
  virtual void slotOk( void );

protected:
  QString text();
  bool sendBugReport();

  KProcess * m_process;
  const KAboutData * m_aboutData;

  QMultiLineEdit * m_lineedit;
  QLineEdit * m_subject;
  QLabel * m_from;
  QLabel * m_version;
  QString m_strVersion;
  QHButtonGroup * m_bgSeverity;

private:
  KBugReportPrivate *d;
};

#endif

