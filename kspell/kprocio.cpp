// $Id$

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>

#include "kprocio.h"

#include <kdebug.h>

KProcIO::KProcIO (void)
{
  rbi=0;
  readsignalon=writeready=TRUE;
}

void
KProcIO::resetAll (void)
{
  if (isRunning())
    kill();

  clearArguments();
  rbi=0;
  readsignalon=writeready=TRUE;

  disconnect (this, SIGNAL (receivedStdout (KProcess *, char *, int)),
	   this, SLOT (received (KProcess *, char *, int)));

  disconnect (this, SIGNAL (wroteStdin(KProcess *)),
	   this, SLOT (sent (KProcess *)));

  qlist.clear();

}

bool KProcIO::start (RunMode runmode)
{
  connect (this, SIGNAL (receivedStdout (KProcess *, char *, int)),
	   this, SLOT (received (KProcess *, char *, int)));

  connect (this, SIGNAL (wroteStdin(KProcess *)),
	   this, SLOT (sent (KProcess *)));
           
  qlist.setAutoDelete (TRUE);

  return KProcess::start (runmode, KProcess::All);
}

bool KProcIO::writeStdin (const char *buffer, bool appendnewline)
{
  QString qs (buffer);
  if (appendnewline)
    qs+='\n';

  qlist.append (qs.ascii());

  kdebug(KDEBUG_INFO, 750, "KPIO::write [%s],[%s]", buffer, qlist.current());

  if (writeready)
    {
      kdebug(KDEBUG_INFO, 750, "really writing");
      writeready=FALSE;
      return KProcess::writeStdin (qlist.current(),
				   strlen (qlist.current()));
    }
  kdebug(KDEBUG_INFO, 750, "NOT really writing");
  return TRUE;
}

void KProcIO::sent (KProcess *)
{
  kdebug(KDEBUG_INFO, 750, "KP::sent [%s]",qlist.first());

  qlist.removeFirst();

  if (qlist.count()==0)
    {
      kdebug(KDEBUG_INFO, 750, "Empty");
      writeready=TRUE;
    }
  else
    {
      kdebug(KDEBUG_INFO, 750, "Sending [%s]", qlist.first());
	      KProcess::writeStdin (qlist.first(), strlen (qlist.first()));
    }

}

void KProcIO::received (KProcess *, char *buffer, int buflen)
{
  int i;

  buffer [buflen]='\0';

  kdebug(KDEBUG_INFO, 750, "KPIO: recv'd [%s]",buffer);

  for (i=0;i<buflen;i++)
    recvbuffer+=buffer [i];

  controlledEmission();
}

void KProcIO::ackRead (void)
{
  readsignalon=TRUE;
  if (needreadsignal || recvbuffer.length()!=0)
    controlledEmission();
}

void KProcIO::controlledEmission (void)
{
  //  if (readsignalon)
    {
      needreadsignal=FALSE;
      readsignalon=FALSE; //will stay off until read is acknowledged
      emit readReady (this);
    }
    //  else
    //    needreadsignal=TRUE;

}

void KProcIO::enableReadSignals (bool enable)
{
  readsignalon=enable;

  if (enable && needreadsignal)
	emit readReady (this);
}

int KProcIO::readln (char *buffer, int max, bool autoAck)
{
  int len;

  if (autoAck)
    readsignalon=TRUE;

  //need to reduce the size of recvbuffer at some point...

  len=recvbuffer.find ('\n',rbi)-rbi;

  kdebug(KDEBUG_INFO, 750, "KPIO::readln\n");

  //in case there's no '\n' at the end of the buffer
  if (len<0 && (unsigned)rbi<recvbuffer.length())
    {
      QString qs=recvbuffer.mid (rbi,recvbuffer.length()-rbi);
      recvbuffer=qs;
      rbi=0;
      return -1;
    }

  if (len>=0)
    {
    if (len-rbi<max)
      {
	strcpy (buffer, recvbuffer.mid (rbi,len).ascii());
	buffer [len]='\0';
	rbi+=len+1;
	return len;
      }
    else
      {
	strcpy (buffer, recvbuffer.mid (rbi,max).ascii());
	buffer [max]='\0';
	rbi+=max+1;
	return max;
      }
    }
  
  recvbuffer="";
  rbi=0;

  //-1 on return signals "no more data" not error
  return -1;
    
}
#include "kprocio.moc"

