// interface.h -*- C++ -*-
// Copyright (C)  2002  Dominique Devriese <devriese@kde.org>
// Copyright (C)  2005  Peter Rockai <me@mornfall.net>

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

#ifndef KDELIBS_TERMINAL_INTERFACE_H
#define KDELIBS_TERMINAL_INTERFACE_H

class QString;
class QStrList;

#include <kdemacros.h>

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
 *  void processExited( KProcess *process );
 *  void receivedData( const QString& s );
 * See the example code below for how to connect to these..
 * 
 * The process provided by processExited() is obviously exited,
 * and is only guaranteed to be valid until you return from the
 * slot connected to it! 
 *
 * Use it like this:
 * \code
 *  // fetch the Library..
 *  KLibFactory* factory = KLibLoader::self()->factory( "libkonsolepart" );
 *  if ( factory == 0L )
 *  {
 *    // inform the user that he should install konsole..
 *    return;
 *  };
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
 *  };
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
 * @since 3.2
 *
 * @author Dominique Devriese <devriese@kde.org>
 */
class KDE_EXPORT TerminalInterface
{
public:
  /**
   * This starts @p program, with arguments @p args
   */
  virtual void startProgram( const QString& program,
                             const QStrList& args ) = 0;
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

/**
   This class is used analogically to TerminalInterface (see it's
   documentation), but provides 2 further methods to change
   konsole's behaviour.

   For KDE 4, this class will be dropped again and the functionality
   merged into TerminalInterface. Only use this if you really need
   it for 3.5...

   @see TerminalInterface

   @since 3.5
*/

class KDE_EXPORT ExtTerminalInterface
{
public:
  /**
   * This starts @p program, with arguments @p args
   */
  virtual void startProgram( const QString& program,
                             const QStrList& args ) = 0;
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

  /**
     Call this to disable the automatic shell that
     is normally loaded when konsolePart is instantiated;

     You must call this function immediately after creating
     the part! The shell is otherwise started as soon as the Qt
     event loop is entered.
  */       
  virtual void setAutoStartShell(bool enabled) = 0;

  /**
     If set to true (which is default), konsolePart will destroy itself
     as soon as the running program terminates. If false, you can
     start another program instead or close it yourself.
  */
  virtual void setAutoDestroy(bool enabled) = 0;
};

#endif
