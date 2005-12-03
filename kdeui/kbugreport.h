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
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/
#ifndef KBUGREPORT_H
#define KBUGREPORT_H

#include <kdialogbase.h>

class QTextEdit;
class QLineEdit;
class QGroupBox;
class KProcess;
class KAboutData;
class KBugReportPrivate;

/**
 * @short A dialog box for sending bug reports.
 *
 * All the information needed by the dialog box
 * (program name, version, bug-report address, etc.)
 * comes from the KAboutData class.
 * Make sure you create an instance of KAboutData and pass it
 * to KCmdLineArgs.
 *
 * @author David Faure <faure@kde.org>
 */
class KDEUI_EXPORT KBugReport : public KDialogBase
{
  Q_OBJECT
public:
  /**
   * Creates a bug-report dialog.
   * Note that you shouldn't have to do this manually,
   * since KHelpMenu takes care of the menu item
   * for "Report Bug..." and of creating a KBugReport dialog.
   */
  KBugReport( QWidget * parent = 0L, bool modal=true, const KAboutData *aboutData = 0L );
  /**
   * Destructor
   */
  virtual ~KBugReport();

protected slots:
  /**
   * "Configure email" has been clicked - this calls kcmshell System/email
   */
  virtual void slotConfigureEmail();
  /**
   * Sets the "From" field from the e-mail configuration
   * Called at creation time, but also after "Configure email" is closed.
   */
  virtual void slotSetFrom();
  /**
   * OK has been clicked
   */
  virtual void slotOk( void );
  /**
   * Cancel has been clicked
   */
  virtual void slotCancel();

  /**
   * Application combo selection changed (and was activated)
   */
  void appChanged(int);
  /**
   * Update the url to match the current os, compiler, selected app, etc
   */
  void updateURL();

protected:
  /**
   * A complete copy of the bug report
   * @return QString copy of the bug report.
   */
  QString text() const;
  /**
   * Attempt to e-mail the bug report.
   * @return true on success
   */
  bool sendBugReport();

  KProcess * m_process;
  const KAboutData * m_aboutData;

  QTextEdit * m_lineedit;
  QLineEdit * m_subject;
  QLabel * m_from;
  QLabel * m_version;
  QString m_strVersion;
  QGroupBox * m_bgSeverity;
  QPushButton * m_configureEmail;

protected:
  virtual void virtual_hook( int id, void* data );
private:
  KBugReportPrivate *d;
};

#endif

