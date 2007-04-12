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

#include "k3procio.h"

#include <config.h>
#include <stdio.h>

#include <kdebug.h>
#include <QtCore/QTextCodec>

class KProcIOPrivate
{
public:
    KProcIOPrivate( QTextCodec* c )
        : codec( c ),
          rbi( 0 ),
          readsignalon( true ),
          writeready( true ),
          comm(K3Process::All)
    {
    }

    QList<QByteArray *> outbuffer;
    QByteArray recvbuffer;
    QTextCodec *codec;
    int rbi;
    bool needreadsignal;
    bool readsignalon;
    bool writeready;
    K3Process::Communication comm;
};

K3ProcIO::K3ProcIO ( QTextCodec *_codec)
  : d( new KProcIOPrivate( _codec ) )
{
    if ( !d->codec ) {
        d->codec = QTextCodec::codecForName( "ISO 8859-1" );
        if ( !d->codec ) {
            kError( 174 ) << "Can't create ISO 8859-1 codec!" << endl;
        }
    }
}

K3ProcIO::~K3ProcIO()
{
    qDeleteAll( d->outbuffer );
    delete d;
}

void
K3ProcIO::resetAll ()
{
    if (isRunning()) {
        kill();
    }

    clearArguments();
    d->rbi = 0;
    d->readsignalon = true;
    d->writeready = true;

  disconnect (this, SIGNAL (receivedStdout (K3Process *, char *, int)),
	   this, SLOT (received (K3Process *, char *, int)));

  disconnect (this, SIGNAL (receivedStderr (K3Process *, char *, int)),
	   this, SLOT (received (K3Process *, char *, int)));

  disconnect (this, SIGNAL (wroteStdin(K3Process *)),
	   this, SLOT (sent (K3Process *)));

  qDeleteAll( d->outbuffer );
  d->outbuffer.clear();
}

void K3ProcIO::setComm (Communication comm)
{
  d->comm = comm;
}

bool K3ProcIO::start (RunMode runmode, bool includeStderr)
{
  connect (this, SIGNAL (receivedStdout (K3Process *, char *, int)),
	   this, SLOT (received (K3Process *, char *, int)));

  if (includeStderr)
  {
     connect (this, SIGNAL (receivedStderr (K3Process *, char *, int)),
              this, SLOT (received (K3Process *, char *, int)));
  }

  connect (this, SIGNAL (wroteStdin(K3Process *)),
	   this, SLOT (sent (K3Process *)));

  return K3Process::start (runmode, d->comm);
}

bool K3ProcIO::writeStdin (const QString &line, bool appendnewline)
{
    return writeStdin( d->codec->fromUnicode( line ), appendnewline );
}

bool K3ProcIO::writeStdin (const QByteArray &line, bool appendnewline)
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
        return K3Process::writeStdin( b->data(), b->size() );
    }

    return true;
}

bool K3ProcIO::writeStdin(const QByteArray &data)
{
    if (!data.size()) {
        return true;
    }
    QByteArray *b = new QByteArray(data);
    d->outbuffer.append(b);

    if ( d->writeready ) {
        d->writeready=false;
        return K3Process::writeStdin( b->data(), b->size() );
    }

    return true;
}

void K3ProcIO::closeWhenDone()
{
    if (d->writeready) {
        closeStdin();
        return;
    }
    d->outbuffer.append(0);

    return;
}

void K3ProcIO::sent(K3Process *)
{
    d->outbuffer.removeFirst();

    if ( d->outbuffer.count() == 0 ) {
        d->writeready = true;
    } else {
        QByteArray *b = d->outbuffer.first();
        if (!b) {
            closeStdin();
        } else {
            K3Process::writeStdin(b->data(), b->size());
        }
    }
}

void K3ProcIO::received (K3Process *, char *buffer, int buflen)
{
  d->recvbuffer += QByteArray(buffer, buflen);

  controlledEmission();
}

void K3ProcIO::ackRead ()
{
    d->readsignalon = true;
    if ( d->needreadsignal || d->recvbuffer.length() != 0 ) {
        controlledEmission();
    }
}

void K3ProcIO::controlledEmission ()
{
    if ( d->readsignalon ) {
        d->needreadsignal = false;
        d->readsignalon = false; //will stay off until read is acknowledged
        emit readReady (this);
    } else {
        d->needreadsignal = true;
    }
}

void K3ProcIO::enableReadSignals (bool enable)
{
    d->readsignalon = enable;

    if ( enable && d->needreadsignal ) {
        emit readReady(this);
    }
}

int K3ProcIO::readln (QString &line, bool autoAck, bool *partial)
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

#include "k3procio.moc"

