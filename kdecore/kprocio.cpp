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

// $Id$

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>

#include "kprocio.h"

#include <kdebug.h>
#include <qtextcodec.h>

KProcIO::KProcIO ( QTextCodec *_codec)
  : codec(_codec)
{
  rbi=0;
  readsignalon=writeready=TRUE;
  outbuffer.setAutoDelete(true);

  if (!codec)
  {
     codec = QTextCodec::codecForName("ISO 8859-1");
     if (!codec)
     {
        kdError(174) << "Can't create ISO 8859-1 codec!" << endl;
     }
  }
}

KProcIO::~KProcIO()
{
}

void
KProcIO::resetAll ()
{
  if (isRunning())
     kill();

  clearArguments();
  rbi=0;
  readsignalon=writeready=TRUE;

  disconnect (this, SIGNAL (receivedStdout (KProcess *, char *, int)),
	   this, SLOT (received (KProcess *, char *, int)));

  disconnect (this, SIGNAL (receivedStderr (KProcess *, char *, int)),
	   this, SLOT (received (KProcess *, char *, int)));

  disconnect (this, SIGNAL (wroteStdin(KProcess *)),
	   this, SLOT (sent (KProcess *)));

  outbuffer.clear();

}

bool KProcIO::start (RunMode runmode, bool includeStderr)
{
  connect (this, SIGNAL (receivedStdout (KProcess *, char *, int)),
	   this, SLOT (received (KProcess *, char *, int)));
  
  if (includeStderr)
  {
     connect (this, SIGNAL (receivedStderr (KProcess *, char *, int)),
              this, SLOT (received (KProcess *, char *, int)));
  }

  connect (this, SIGNAL (wroteStdin(KProcess *)),
	   this, SLOT (sent (KProcess *)));
           
  return KProcess::start (runmode, KProcess::All);
}

bool KProcIO::writeStdin (const QString &line, bool appendnewline)
{
  return writeStdin(codec->fromUnicode(line), appendnewline);
}

bool KProcIO::writeStdin (const QCString &line, bool appendnewline)
{
  QCString *qs = new QCString(line);
  
  if (appendnewline)
  {
     *qs += '\n';
  }
    
  int l = qs->length();
  if (!l) 
  {
     delete qs;
     return true;
  }

  QByteArray *b = (QByteArray *) qs;
  b->truncate(l); // Strip trailing null
  
  outbuffer.append(b);

  if (writeready)
  {
     writeready=FALSE;
     return KProcess::writeStdin( b->data(), b->size() );
  }
  return true;
}

bool KProcIO::writeStdin(const QByteArray &data)
{
  if (!data.size())
     return true;
  QByteArray *b = new QByteArray(data);
  outbuffer.append(b);
  
  if (writeready)
  {
     writeready=FALSE;
     return KProcess::writeStdin( b->data(), b->size() );
  }
  return true;
}

void KProcIO::closeWhenDone()
{
  if (writeready)
  {
     closeStdin();
     return;
  }
  outbuffer.append(0);
  
  return;
}

void KProcIO::sent(KProcess *)
{
  outbuffer.removeFirst();

  if (outbuffer.count()==0)
  {
     kdDebug(174) << "Empty" << endl;
     writeready=TRUE;
  }
  else
  {
     QByteArray *b = outbuffer.first();
     if (!b)
     {
        kdDebug(174) << "Closing" << endl;
        closeStdin();
     }
     else
     {
        kdDebug(174) << "Sending [" << b->size() << "]" << endl;
        KProcess::writeStdin(b->data(), b->size());
     }
  }

}

void KProcIO::received (KProcess *, char *buffer, int buflen)
{
  recvbuffer += QCString(buffer, buflen+1);

  controlledEmission();
}

void KProcIO::ackRead ()
{
  readsignalon=TRUE;
  if (needreadsignal || recvbuffer.length()!=0)
     controlledEmission();
}

void KProcIO::controlledEmission ()
{
  if (readsignalon)
  {
     needreadsignal=FALSE;
     readsignalon=FALSE; //will stay off until read is acknowledged
     emit readReady (this);
  }
  else
  {
    needreadsignal=TRUE;
  }
}

void KProcIO::enableReadSignals (bool enable)
{
  readsignalon=enable;

  if (enable && needreadsignal)
     emit readReady (this);
}

int KProcIO::readln (QString &line, bool autoAck, bool *partial)
{
  int len;

  if (autoAck)
     readsignalon=TRUE;

  //need to reduce the size of recvbuffer at some point...

  len=recvbuffer.find ('\n',rbi)-rbi;

  //kdDebug(174) << "KPIO::readln" << endl;

  //in case there's no '\n' at the end of the buffer
  if ((len<0) && 
      ((unsigned int)rbi<recvbuffer.length()))
  {
     recvbuffer=recvbuffer.mid (rbi,recvbuffer.length()-rbi);
     rbi=0;
     if (partial)
     {
        len = recvbuffer.length();
        line = recvbuffer;
        recvbuffer = "";
        *partial = true;
        return len;
     }
     return -1;
  }

  if (len>=0)
  {
     line = codec->toUnicode(recvbuffer.mid(rbi,len), len);
     rbi += len+1;
     if (partial)
        *partial = false;
     return len;
  }

  recvbuffer="";
  rbi=0;

  //-1 on return signals "no more data" not error
  return -1;

}

void KProcIO::virtual_hook( int id, void* data )
{ KProcess::virtual_hook( id, data ); }

#include "kprocio.moc"

