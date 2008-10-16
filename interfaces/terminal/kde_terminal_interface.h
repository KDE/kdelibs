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
 * These are the signals you can connect to:
 *  void processExited( int status );
 *  void receivedData( const QString& s );
 * See the example code below for how to connect to these..
 *
 * Use it like this:
 * \code
 *  // fetch the Library..
 *  KLibFactory* factory = KLibLoader::self()->factory( "libkonsolepart" );
 *  if ( factory == 0L )
 *  {
 *    // inform the user that he should install konsole..
 *    return;
 *  }
 *  // fetch the part..
 *  KParts::Part* p = static_cast<KParts::Part*>(
 *      factory->create( this, "tralala", "QObject",
 *                       "KParts::ReadOnlyPart" ) );
 *  assert( p );
 *  setCentralWidget( p->widget() );
 *
 *  // cast the part to the TerminalInterface..
 *  TerminalInterface* t = static_cast<TerminalInterface*>( p->qt_cast( "TerminalInterface" ) );
 *  if( ! t )
 *  {
 *    // This probably happens because the konsole that is installed
 *    // comes from before KDE 3.2 , and the TerminalInterface is not
 *    // available..  What you can do here is either inform the user
 *    // that he needs a more recent konsole, or try to deliver the
 *    // functionality in some other way...
 *    return;
 *  }
 *  // now use the interface in all sorts of ways, e.g.
 *  //    t->showShellInDir( QDir::home().path() );
 *  // or:
 *  //    QStrList l;
 *  //    l.append( "python" );
 *  //    t->startProgram( QString::fromUtf8( "/usr/bin/python" ), l);
 *  // or connect to one of the signals.  Connect to the Part object,
 *  // not to the TerminalInterface, since the latter is no QObject,
 *  // and as such cannot have signals..:
 *  //    connect( p, SIGNAL( processExited( int ) ),
 *  //             this, SLOT( shellExited( int ) ) );
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
   * If a shell is currently shown, this sends it a cd
   * command. Otherwise, this starts a shell, and sends it a cd
   * command too...
   */
  virtual void showShellInDir( const QString& dir ) = 0;

  /**
   * This sends @param text as input to the currently running
   * program..
   */
  virtual void sendInput( const QString& text ) = 0;

};

Q_DECLARE_INTERFACE(TerminalInterface, "org.kde.TerminalInterface")

#endif
