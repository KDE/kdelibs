// interface.h
// Copyright (C)  2002  Dominique Devriese <devriese@kde.org>

// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.

// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.

// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
// 02110-1301  USA

#ifndef KDELIBS_KDE_TERMINAL_INTERFACE_H
#define KDELIBS_KDE_TERMINAL_INTERFACE_H

#include <QtCore/QObject>

class QStringList;

/**
 * TerminalInterface is an interface implemented by KonsolePart to
 * allow developers access to the KonsolePart in ways that are not
 * possible through the normal KPart interface.
 *
 * Note that besides the functions below here, KonsolePart also has
 * some signals you can connect to.  They aren't in this class cause
 * we can't have signals without having a QObject, which
 * TerminalInterface is not.
 *
 * These are some signals you can connect to:
 *  void currentDirectoryChanged(const QString& dir);
 *
 * See the example code below for how to connect to these.
 *
 * Use it like this:
 * \code
 *  //query the .desktop file to get service information about konsolepart
 *  KService::Ptr service = KService::serviceByDesktopName("konsolepart");
 *
 *  if (!service)
 *  {
 *      QMessageBox::critical(this, tr("Konsole not installed"), tr("Please install the kde konsole and try again!"), QMessageBox::Ok);
 *      return ;
 *  }
 *
 *  // create one instance of konsolepart
 *  KParts::ReadOnlyPart* p = service->createInstance<KParts::ReadOnlyPart>(parent, parentWidget, QVariantList());
 *
 *  if (!p)
 *  {
 *      return;
 *  }
 *
 *  // cast the konsolepart to the TerminalInterface..
 *  TerminalInterface* t = qobject_cast<TerminalInterface*>(p);
 *
 *  if (!t)
 *  {
 *      return;
 *  }
 *
 *  // now use the interface in all sorts of ways, e.g.
 *  //    t->showShellInDir( QDir::home().path() );
 *  // or:
 *  //    QStringList l;
 *  //    l.append( "python" );
 *  //    t->startProgram( QString::fromUtf8( "/usr/bin/python" ), l);
 *  // or connect to one of the signals.  Connect to the Part object,
 *  // not to the TerminalInterface, since the latter is no QObject,
 *  // and as such cannot have signals..:
 *  //    connect(p, SIGNAL(currentDirectoryChanged(QString)),
 *  //             this, SLOT(currentDirectoryChanged(QString)));
 *  // etc.
 *
 * \endcode
 *
 * @author Dominique Devriese <devriese@kde.org>
 */
class TerminalInterface
{
public:
  virtual ~TerminalInterface(){}
  /**
   * This starts @p program, with arguments @p args
   */
  virtual void startProgram( const QString& program,
                             const QStringList& args ) = 0;
  /**
   * If no shell is running, this starts a shell with the
   * @dir as the starting directory.
   * If a shell is already running, nothing is done.
   */
  virtual void showShellInDir( const QString& dir ) = 0;

  /**
   * This sends @param text as input to the currently running
   * program..
   */
  virtual void sendInput( const QString& text ) = 0;

  /**
   * Return terminal PID
   */
  virtual int terminalProcessId() = 0;

  /**
   * Return foregound PID, If there is no foreground process running, returns -1
   */
  virtual int foregroundProcessId() = 0;

  /**
   * Returns sub process name. If there is no sub process running, returns empty QString
   */
  virtual QString foregroundProcessName() = 0;

};

Q_DECLARE_INTERFACE(TerminalInterface, "org.kde.TerminalInterface")

#endif
