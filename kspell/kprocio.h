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
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/
#ifndef KPROCIO_H_
#define KPROCIO_H_

#include <qstring.h>
#include <kprocess.h>

/**
 * @version $Id$
 * @author David Sweet
 * @short A slightly simpler interface to KProcess
 *
 * KProcIO
 * By David Sweet (LGPL 1997)
 *
 * This class provides a slightly simpler interface to the communication
 *  functions provided by KProcess.  The simplifications are:
 *    1) The buffer for a write is copied to an internal KProcIO
 *        buffer and maintained/freed appropriately.  There is no need
 *        to be concerned with wroteStdin() signals _at_all_.
 *    2) readln() (or fgets()) reads a line of data and buffers any 
 *        leftovers.
 *
 * Added:
 *
 *    3) Conversion from/to unicode.
 *
 * Basically, KProcIO gives you buffered I/O similar to fgets()/fputs().
 *
 * Aside from these, and the fact that start() takes different
 *  parameters, use this class just like KProcess.
 **/

class QTextCodec;

class KProcIO : public KProcess
{
  Q_OBJECT
public:
  KProcIO ( QTextCodec *codec = 0 );
  
  bool start (RunMode  runmode = NotifyOnExit);

  /**
   * The buffer is zero terminated.
   * A deep copy is made of the buffer, so you don't
   * need to bother with that.  A newline ( '\n' ) is appended 
   * unless you specify FALSE as the second parameter.
   * FALSE is returned on an error, or else TRUE is.
   **/
  virtual bool writeStdin(const QString &line, bool AppendNewLine=TRUE);

  //I like fputs better -- it's the same as writeStdin
  //inline
  bool fputs (const QString &line, bool AppendNewLine=TRUE)
    { return writeStdin(line, AppendNewLine); }

  /**
   * readln() reads up to '\n' (or max characters) and
   * returns the number of characters placed in buffer.  Zero is returned
   * if no more data is available.
   *
   * Use readln() in response to a readReady() signal.
   * You may use it multiple times if more than one line of data is
   *  available.
   * Be sure to use ackRead() when you have finished processing the
   *  readReady() signal.  This informs KProcIO that you are ready for
   *  another readReady() signal.
   *
   * readln() never blocks.
   *
   * autoAck==TRUE makes these functions call ackRead() for you.
   **/
  virtual int readln (QString &line, bool autoAck=FALSE);

  int fgets (QString &line, bool autoAck=FALSE)
    { return readln (line, autoAck); }

  /**
   * Reset the class.  Doesn't kill the process.
   **/
   virtual void resetAll ();

  /**
   * Call this after you have finished processing a readReady()
   * signal.  This call need not be made in the slot that was signalled
   * by readReady().  You won't receive any more readReady() signals
   * until you acknowledge with ackRead().  This prevents your slot
   * from being reentered while you are still processesing the current
   * data.  If this doesn't matter, then call ackRead() right away in
   * your readReady()-processing slot.
   **/
  virtual void ackRead ();

  /**
   *  Turns readReady() signals on and off.
   *   You can turn this off at will and not worry about losing any data.
   *   (as long as you turn it back on at some point...)
   */
  void enableReadSignals (bool enable);

signals:
  void readReady(KProcIO *);

protected:
  QStrList qlist;
  QCString recvbuffer;
  QTextCodec *codec;
  int rbi;
  bool needreadsignal, readsignalon, writeready;

  void controlledEmission ();

protected slots:
  void received (KProcess *proc, char *buffer, int buflen);
  void sent (KProcess *);

private:
  class KProcIOPrivate;
  KProcIOPrivate *d;
};

#endif
