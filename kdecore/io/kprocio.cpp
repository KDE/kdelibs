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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>

#include "kprocio.h"

#include <kdebug.h>
#include <qtextcodec.h>

class KProcIOPrivate
{
public:
    KProcIOPrivate( QTextCodec* c )
        : codec( c ),
          rbi( 0 ),
          readsignalon( true ),
          writeready( true ),
          comm(KProcess::All)
    {
    }

    QList<QByteArray *> outbuffer;
    QByteArray recvbuffer;
    QTextCodec *codec;
    int rbi;
    bool needreadsignal;
    bool readsignalon;
    bool writeready;
    KProcess::Communication comm;
};

KProcIO::KProcIO ( QTextCodec *_codec)
  : d( new KProcIOPrivate( _codec ) )
{
    if ( !d->codec ) {
        d->codec = QTextCodec::codecForName( "ISO 8859-1" );
        if ( !d->codec ) {
            kError( 174 ) << "Can't create ISO 8859-1 codec!" << endl;
        }
    }
}

KProcIO::~KProcIO()
{
    qDeleteAll( d->outbuffer );
    delete d;
}

void
KProcIO::resetAll ()
{
    if (isRunning()) {
        kill();
    }

    clearArguments();
    d->rbi = 0;
    d->readsignalon = true;
    d->writeready = true;

  disconnect (this, SIGNAL (receivedStdout (KProcess *, char *, int)),
	   this, SLOT (received (KProcess *, char *, int)));

  disconnect (this, SIGNAL (receivedStderr (KProcess *, char *, int)),
	   this, SLOT (received (KProcess *, char *, int)));

  disconnect (this, SIGNAL (wroteStdin(KProcess *)),
	   this, SLOT (sent (KProcess *)));

  qDeleteAll( d->outbuffer );
  d->outbuffer.clear();
}

void KProcIO::setComm (Communication comm)
{
  d->comm = comm;
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
           
  return KProcess::start (runmode, d->comm);
}

bool KProcIO::writeStdin (const QString &line, bool appendnewline)
{
    return writeStdin( d->codec->fromUnicode( line ), appendnewline );
}

bool KProcIO::writeStdin (const QByteArray &line, bool appendnewline)
{
  QByteArray *qs = new QByteArray(line);

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

    d->outbuffer.append(b);

    if ( d->writeready ) {
        d->writeready = false;
        return KProcess::writeStdin( b->data(), b->size() );
    }

    return true;
}

bool KProcIO::writeStdin(const QByteArray &data)
{
    if (!data.size()) {
        return true;
    }
    QByteArray *b = new QByteArray(data);
    d->outbuffer.append(b);

    if ( d->writeready ) {
        d->writeready=false;
        return KProcess::writeStdin( b->data(), b->size() );
    }

    return true;
}

void KProcIO::closeWhenDone()
{
    if (d->writeready) {
        closeStdin();
        return;
    }
    d->outbuffer.append(0);

    return;
}

void KProcIO::sent(KProcess *)
{
    d->outbuffer.removeFirst();

    if ( d->outbuffer.count() == 0 ) {
        d->writeready = true;
    } else {
        QByteArray *b = d->outbuffer.first();
        if (!b) {
            closeStdin();
        } else {
            KProcess::writeStdin(b->data(), b->size());
        }
    }
}

void KProcIO::received (KProcess *, char *buffer, int buflen)
{
  d->recvbuffer += QByteArray(buffer, buflen);

  controlledEmission();
}

void KProcIO::ackRead ()
{
    d->readsignalon = true;
    if ( d->needreadsignal || d->recvbuffer.length() != 0 ) {
        controlledEmission();
    }
}

void KProcIO::controlledEmission ()
{
    if ( d->readsignalon ) {
        d->needreadsignal = false;
        d->readsignalon = false; //will stay off until read is acknowledged
        emit readReady (this);
    } else {
        d->needreadsignal = true;
    }
}

void KProcIO::enableReadSignals (bool enable)
{
    d->readsignalon = enable;

    if ( enable && d->needreadsignal ) {
        emit readReady(this);
    }
}

int KProcIO::readln (QString &line, bool autoAck, bool *partial)
{
  int len;

  if ( autoAck ) {
     d->readsignalon=true;
  }

  //need to reduce the size of recvbuffer at some point...

  len = d->recvbuffer.indexOf('\n', d->rbi) - d->rbi;

  //kDebug(174) << "KPIO::readln" << endl;

  //in case there's no '\n' at the end of the buffer
  if ( ( len < 0 ) && 
       ( d->rbi < d->recvbuffer.length() ) ) {
     d->recvbuffer = d->recvbuffer.mid( d->rbi );
     d->rbi = 0;
     if (partial)
     {
        len = d->recvbuffer.length();
        line = d->recvbuffer;
        d->recvbuffer = "";
        *partial = true;
        return len;
     }
     return -1;
  }

  if (len>=0)
  {
     line = d->codec->toUnicode( d->recvbuffer.mid( d->rbi, len ) );
     d->rbi += len + 1;
     if (partial)
        *partial = false;
     return len;
  }

  d->recvbuffer = "";
  d->rbi = 0;

  //-1 on return signals "no more data" not error
  return -1;

}

#include "kprocio.moc"

