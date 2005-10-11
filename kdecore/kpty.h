/* This file is part of the KDE libraries

    Copyright (C) 1997-2002 The Konsole Developers
    Copyright (C) 2002 Waldo Bastian <bastian@kde.org>
    Copyright (C) 2002-2003 Oswald Buddenhagen <ossi@kde.org>

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

#ifndef kpty_h
#define kpty_h

#include <qglobal.h>
#include <kdelibs_export.h>

#ifdef Q_OS_UNIX

struct KPtyPrivate;

/**
 * Provides a high level representation of a pseudo tty pair, including
 * utmp support.
 * ...
 *
 * @since 3.2
 **/
class KDECORE_EXPORT KPty {

public:

  /**
   * Constructor
   */
  KPty();

  /**
   * Destructor:
   *
   *  If the pty is still open, it will be closed. Note, however, that
   *  an utmp registration is @em not undone.
  */
  ~KPty();

  /**
   * Create a pty master/slave pair.
   *
   * @return true if a pty pair was successfully opened
   */
  bool open();

  /**
   * Close the pty master/slave pair.
   */
  void close();

  /**
   * Creates a new session and process group and makes this pty the
   * controlling tty.
   */
  void setCTty();

  /**
   * Creates an utmp entry for the tty.
   * This function must be called after calling setCTty and
   * making this pty the stdin.
   * @param user the user to be logged on
   * @param remotehost the host from which the login is coming. This is
   *  @em not the local host. For remote logins it should be the hostname
   *  of the client. For local logins from inside an X session it should
   *  be the name of the X display. Otherwise it should be empty.
   */
  void login(const char *user = 0, const char *remotehost = 0);

  /**
   * Removes the utmp entry for this tty.
   */
  void logout();

  /**
   * Change the logical (screen) size of the pty.
   * The default is 24 lines by 80 columns.
   *
   * @param lines the number of rows
   * @param columns the number of columns
   */
  void setWinSize(int lines, int columns);

  /**
   * Set whether the pty should honor Xon/Xoff flow control.
   *
   * Xon/Xoff flow control is off by default.
   *
   * @param useXonXoff true if Xon/Xoff flow control should be used.
   */
  void setXonXoff(bool useXonXoff);

  /**
   * Set the pty in utf8 mode on systems that support it.
   * 
   * See the man page of "stty iutf8" for more info.
   *
   * @since 3.4
   */
  void setUtf8Mode(bool useUtf8);


  /**
   * @return the name of the slave pty device.
   *
   * This function should be called only while the pty is open.
   */
  const char *ttyName() const;

  /**
   * @return the file descriptor of the master pty
   *
   * This function should be called only while the pty is open.
   */
  int masterFd() const;

  /**
   * @return the file descriptor of the slave pty
   *
   * This function should be called only while the pty is open.
   */
  int slaveFd() const;

private:
  bool chownpty(bool grant);

  KPtyPrivate *d;
};

#endif //Q_OS_UNIX
#endif

