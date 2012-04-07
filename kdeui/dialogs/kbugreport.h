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

#include <kdialog.h>
#include <kdeui_export.h>

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
 * \image html kbugreport.png "KDE Bug Report Dialog"
 *
 * @author David Faure <faure@kde.org>
 */
class KDEUI_EXPORT KBugReport : public KDialog
{
  Q_OBJECT
  
public:
  /**
   * Creates a bug-report dialog.
   * Note that you shouldn't have to do this manually,
   * since KHelpMenu takes care of the menu item
   * for "Report Bug..." and of creating a KBugReport dialog.
   */
  // ###KDE5: remove modal argument
  explicit KBugReport(QWidget *parent = 0L, bool modal=true, const KAboutData *aboutData = 0L);
  
  /**
   * Destructor
   */
  virtual ~KBugReport();

  /**
   * The message body of the bug report
   * @return The message body of the bug report.
   */
  QString messageBody() const;
  
  /**
   * Sets the message body of the bug report.
   */
  void setMessageBody(const QString &messageBody);

  /**
    * OK has been clicked
   */
  virtual void accept();
  
private:
  /**
   * "Configure email" has been clicked - this calls kcmshell4 System/email
   */
  Q_PRIVATE_SLOT(d, void _k_slotConfigureEmail())
  
  /**
   * Sets the "From" field from the e-mail configuration
   * Called at creation time, but also after "Configure email" is closed.
   */
  Q_PRIVATE_SLOT(d, void _k_slotSetFrom())

  /**
   * Application combo selection changed (and was activated)
   */
  Q_PRIVATE_SLOT(d, void _k_appChanged(int))
  
  /**
   * Update the url to match the current os, compiler, selected app, etc
   */
  Q_PRIVATE_SLOT(d, void _k_updateUrl())

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

  virtual void closeEvent(QCloseEvent *e);
  
private:
  friend class KBugReportPrivate;
  KBugReportPrivate *const d;
  
  Q_DISABLE_COPY(KBugReport)
};

#endif

