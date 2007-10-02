/* This file is part of the KDE libraries
   Copyright (C) 1997 David Sweet <dsweet@kde.org>

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
#ifndef K3PROCIO_H
#define K3PROCIO_H

#include <kde3support_export.h>
#include <k3process.h>

#include <QtCore/QString>

class KProcIOPrivate;
class QTextCodec;

/**
 * @obsolete Use KProcess and KPtyProcess instead.
 *
 * This class provides a slightly simpler interface to the communication
 *  functions provided by K3Process.  The simplifications are:
 *    @li The buffer for a write is copied to an internal K3ProcIO
 *        buffer and maintained/freed appropriately.  There is no need
 *        to be concerned with wroteStdin() signals _at_all_.
 *    @li readln() reads a line of data and buffers any leftovers.
 *    @li Conversion from/to unicode.
 *
 * Basically, K3ProcIO gives you buffered I/O similar to fgets()/fputs().
 *
 * Aside from these, and the fact that start() takes different
 * parameters, use this class just like K3Process.
 *
 * @author David Sweet
 * @short A slightly simpler interface to K3Process
 **/


class KDE3SUPPORT_EXPORT_DEPRECATED K3ProcIO : public K3Process
{
  Q_OBJECT

public:
  /**
   * Constructor
   */
  explicit K3ProcIO ( QTextCodec *codec = 0 );

  /**
   * Destructor
   */
  ~K3ProcIO();

  /**
   * Sets the communication mode to be passed to K3Process::start()
   * by start(). The default communication mode is K3Process::All.
   * You probably want to use this function in conjunction with
   * K3Process::setUsePty().
   * @param comm the communication mode
   */
  void setComm (Communication comm);

  /**
   *  Starts the process. It will fail in the following cases:
   *  @li The process is already running.
   *  @li The command line argument list is empty.
   *  @li The starting of the process failed (could not fork).
   *  @li The executable was not found.
   *
   *  @param runmode For a detailed description of the
   *  various run modes, have a look at the
   *  general description of the K3Process class.
   *  @param includeStderr If true, data from both stdout and stderr is
   *  listened to. If false, only stdout is listened to.
   *  @return true on success, false on error.
   **/
  bool start (RunMode  runmode = NotifyOnExit, bool includeStderr = false);

  /**
   * Writes text to stdin of the process.
   * @param line Text to write.
   * @param appendnewline if true, a newline '\\n' is appended.
   * @return true if successful, false otherwise
   **/
  bool writeStdin(const QString &line, bool appendnewline=true);

  /**
   * Writes text to stdin of the process.
   * @param line Text to write.
   * @param appendnewline if true, a newline '\\n' is appended.
   * @return true if successful, false otherwise
   **/
  bool writeStdin(const QByteArray &line, bool appendnewline);

  /**
   * Writes data to stdin of the process.
   * @param data Data to write.
   * @return true if successful, false otherwise
   **/
  bool writeStdin(const QByteArray &data);

  /**
   * Closes stdin after all data has been send.
   */
  void closeWhenDone();

  /**
   * Reads a line of text (up to and including '\\n').
   *
   * Use readln() in response to a readReady() signal.
   * You may use it multiple times if more than one line of data is
   *  available.
   * Be sure to use ackRead() when you have finished processing the
   *  readReady() signal.  This informs K3ProcIO that you are ready for
   *  another readReady() signal.
   *
   * readln() never blocks.
   *
   * autoAck==true makes these functions call ackRead() for you.
   *
   * @param line is used to store the line that was read.
   * @param autoAck when true, ackRead() is called for you.
   * @param partial when provided the line is returned
   * even if it does not contain a '\\n'. *partial will be set to
   * false if the line contains a '\\n' and false otherwise.
   * @return the number of characters read, or -1 if no data is available.
   **/
  int readln (QString &line, bool autoAck=true, bool *partial=0);

  /**
   * Reset the class.  Doesn't kill the process.
   **/
  void resetAll ();

  /**
   * Call this after you have finished processing a readReady()
   * signal.  This call need not be made in the slot that was signalled
   * by readReady().  You won't receive any more readReady() signals
   * until you acknowledge with ackRead().  This prevents your slot
   * from being reentered while you are still processing the current
   * data.  If this doesn't matter, then call ackRead() right away in
   * your readReady()-processing slot.
   **/
  void ackRead ();

  /**
   *  Turns readReady() signals on and off.
   *   You can turn this off at will and not worry about losing any data.
   *   (as long as you turn it back on at some point...)
   * @param enable true to turn the signals on, false to turn them off
   */
  void enableReadSignals (bool enable);

Q_SIGNALS:
  /**
   * Emitted when the process is ready for reading.
   * @param pio the process that emitted the signal
   * @see enableReadSignals()
   */
  void readReady(K3ProcIO *pio);

protected:
  void controlledEmission ();

protected Q_SLOTS:
  void received (K3Process *proc, char *buffer, int buflen);
  void sent (K3Process *);

private:
  KProcIOPrivate* const d;
};

#endif // K3PROCIO_H

