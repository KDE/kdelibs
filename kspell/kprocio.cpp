#include <stdio.h>
#include "kprocio.moc"

//#define __KPIODEBUG

#ifndef __GNUC__
#define inline
#endif

#ifdef __KPIODEBUG
#define dsdebug printf
#else
inline void dsdebug (const char *, ...)  {}
#endif

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

  qlist.append (qs);

#ifdef __KPIODEBUG
  dsdebug ("KPIO::write [%s],[%s]\n",buffer,qlist.current());
#endif

  if (writeready)
    {
#ifdef __KPIODEBUG
      dsdebug ("really writing\n");
#endif
      writeready=FALSE;
      return KProcess::writeStdin (qlist.current(),
				   strlen (qlist.current()));
    }
#ifdef __KPIODEBUG
  dsdebug ("NOT really writing\n");
#endif
  return TRUE;
}

void KProcIO::sent (KProcess *)
{
#ifdef __KPIODEBUG
  dsdebug ("KP::sent  [%s]\n",qlist.first());
#endif

  qlist.removeFirst();

  if (qlist.count()==0)
    {
#ifdef __KPIODEBUG
      dsdebug ("Empty\n");
#endif
      writeready=TRUE;
    }
  else
    {
#ifdef __KPIODEBUG
      dsdebug ("Sending [%s]\n",qlist.first());
#endif
	      KProcess::writeStdin (qlist.first(),
				    strlen (qlist.first()));
    }

}

void KProcIO::received (KProcess *, char *buffer, int buflen)
{
  int i;

  buffer [buflen]='\0';

#ifdef __KPIODEBUG
  dsdebug ("KPIO: recv'd [%s]\n",buffer);
#endif


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

#ifdef __KPIODEBUG
  dsdebug ("KPIO::readln\n");
#endif

  //in case there's no '\n' at the end of the buffer
  if (len<0 && (unsigned)rbi<recvbuffer.length())
    {
      QString qs=recvbuffer.mid (rbi,recvbuffer.length()-rbi).data();
      recvbuffer=(const char*)qs;
      rbi=0;
      return -1;
    }

  if (len>=0)
    {
    if (len-rbi<max)
      {
	strcpy (buffer, recvbuffer.mid (rbi,len));
	buffer [len]='\0';
	rbi+=len+1;
	return len;
      }
    else
      {
	strcpy (buffer, recvbuffer.mid (rbi,max));
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
