/**************************************************************************

    alsaout.cc   - class AlsaOut which represents an alsa client/port pair
    This file is part of LibKMid 0.9.5
    Copyright (C) 2000  Antonio Larrosa Jimenez
    LibKMid's homepage : http://www.arrakis.es/~rlarrosa/libkmid.html

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
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.

    Send comments and bug fixes to Antonio Larrosa <larrosa@kde.org>

***************************************************************************/
#include "alsaout.h"
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include "sndcard.h"
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <sys/param.h>
#include "midispec.h"


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef HAVE_ALSA_ASOUNDLIB_H
#	include <alsa/asoundlib.h>
#elif defined(HAVE_SYS_ASOUNDLIB_H)
#	include <sys/asoundlib.h>
#endif

#ifdef HAVE_LIBASOUND2
#	define HAVE_ALSA_SEQ 1
#	define snd_seq_flush_output(x)	snd_seq_drain_output(x)
#elif defined(HAVE_LIBASOUND)
#	define HAVE_ALSA_SEQ 1
#	include <linux/asequencer.h>
#endif


SEQ_USE_EXTBUF();

class AlsaOut::AlsaOutPrivate
{
public:
#ifdef HAVE_ALSA_SEQ
  AlsaOutPrivate(int _client, int _port, const char *cname,const char *pname)
    {
      handle=0L;
      src=tgt=0L;
      queue=0;
      tPCN=1;
      tgtclient=_client;
      tgtport=_port;
      tgtname=new char[strlen(cname)+strlen(pname)+3];
      strcpy(tgtname, cname);
      strcat(tgtname, "  ");
      strcat(tgtname, pname);
      ev=new snd_seq_event_t;
      timerStarted=false;
    }
#else
  AlsaOutPrivate(int, int, const char *,const char *)
    {
    }
#endif

  ~AlsaOutPrivate()
    {
#ifdef HAVE_ALSA_SEQ
      delete ev;
      delete tgtname;
#endif
    }

#ifdef HAVE_ALSA_SEQ
  snd_seq_t *handle;
  int  client;
  int  queue;
  snd_seq_addr_t *src;
  snd_seq_addr_t *tgt;

  snd_seq_event_t *ev;
  int tPCN;

  int tgtclient;
  int tgtport;
  char *tgtname;

  bool timerStarted;

#endif
};

AlsaOut::AlsaOut(int d,int _client, int _port, const char *cname,const char *pname) : MidiOut (d)
{
  di = new AlsaOutPrivate( _client, _port, cname, pname);
  seqfd = 0;
  devicetype=KMID_ALSA;
  device= d;

  volumepercentage=100;
#ifdef HAVE_ALSA_SEQ
//  printf("%d %d %d (%s)\n",device, di->tgtclient, di->tgtport, di->tgtname);
#endif

  _ok=1;
};

AlsaOut::~AlsaOut()
{
  closeDev();
  delete di;
}

void AlsaOut::openDev (int)
{
#ifndef HAVE_ALSA_SEQ
  return;
#else
  _ok=1;
#ifdef HAVE_LIBASOUND2
  if (snd_seq_open(&di->handle, "hw", SND_SEQ_OPEN_DUPLEX, 0) < 0)
                fprintf(stderr, "Couldn't open sequencer: %s", snd_strerror(errno));
#else
  if (snd_seq_open(&di->handle, SND_SEQ_OPEN) < 0)
                fprintf(stderr, "Couldn't open sequencer: %s", snd_strerror(errno));
#endif

  di->queue = snd_seq_alloc_queue(di->handle);
  if (di->queue < 0) {fprintf(stderr, "Couldn't allocate queue"); return; };
  di->client = snd_seq_client_id(di->handle);
  if (di->client < 0) {fprintf(stderr, "Couldn't get client id"); return; };
  di->tgt = new snd_seq_addr_t;
  di->tgt->client=di->tgtclient;
  di->tgt->port=di->tgtport;

  di->src = new snd_seq_addr_t;
  di->src->client = di->client;
  int port = snd_seq_create_simple_port(di->handle, NULL,
	SND_SEQ_PORT_CAP_WRITE | SND_SEQ_PORT_CAP_SUBS_WRITE
	| SND_SEQ_PORT_CAP_READ, SND_SEQ_PORT_TYPE_MIDI_GENERIC);
  if ( port < 0 )
  {
    delete di->src;
    delete di->tgt;
    di->src=0;
    di->tgt=0;
    _ok=0;
    time=0;
    snd_seq_free_queue(di->handle, di->queue);
    snd_seq_close(di->handle);
    fprintf(stderr, "Cannot connect to %d:%d\n",di->tgtclient,di->tgtport);
    return;
  }
  di->src->port = port;
  

  int r=snd_seq_connect_to(di->handle, di->src->port, di->tgt->client, di->tgt->port);
  if (r < 0) { _ok=0; fprintf(stderr, "Cannot connect to %d:%d\n",di->tgtclient,di->tgtport); }
  time=0;
#endif
}

void AlsaOut::closeDev (void)
{
  if (!ok()) return;
#ifdef HAVE_ALSA_SEQ
  if (di->handle)
  {
    if (di->src) 
    {
       snd_seq_delete_simple_port(di->handle,di->src->port);
       delete di->src;
    }
    if (di->tgt) delete di->tgt;
    if (di->queue) 
    {
      snd_seq_free_queue(di->handle, di->queue);
      snd_seq_close(di->handle);
    }
    di->handle=0;
  }

#endif
}

void AlsaOut::initDev (void)
{
#ifdef HAVE_ALSA_SEQ
  int chn;
  if (!ok()) return;
  uchar gm_reset[5]={0x7e, 0x7f, 0x09, 0x01, 0xf7};
  sysex(gm_reset, sizeof(gm_reset));
  for (chn=0;chn<16;chn++)
  {
    chnmute[chn]=0;
    if (chn!=9) chnPatchChange(chn,0);
    chnPressure(chn,64);
    chnPitchBender(chn, 0x00, 0x40);
    chnController(chn, CTL_MAIN_VOLUME,110*volumepercentage);
    chnController(chn, CTL_EXT_EFF_DEPTH, 0);
    chnController(chn, CTL_CHORUS_DEPTH, 0);
    chnController(chn, 0x4a, 127);
  }
#endif
}

#ifdef HAVE_ALSA_SEQ
void AlsaOut::eventInit(snd_seq_event_t *ev)
{
  snd_seq_ev_clear(ev);
  snd_seq_real_time_t tmp;
  tmp.tv_sec=(time)/1000;
  tmp.tv_nsec=(time%1000)*1000000;
//  printf("time : %d %d %d\n",(int)time,(int)tmp.tv_sec, (int)tmp.tv_nsec);
  if (!di->src) fprintf(stderr,"AlsaOut::eventInit : no source\n");
  ev->source = *di->src;
  if (!di->tgt) fprintf(stderr,"AlsaOut::eventInit : no target\n");
  ev->dest = *di->tgt;

  snd_seq_ev_schedule_real(ev, di->queue, 0, &tmp);

}

void AlsaOut::eventSend(snd_seq_event_t *ev)
{
    /*int err = */ snd_seq_event_output(di->handle, ev);
/*        if (err < 0)
                return;
*/
//#ifndef SND_SEQ_IOCTL_GET_CLIENT_POOL
        /*
         * If this is not defined then block mode writes will not be
         * working correctly.  Therefore loop until all events are flushed
         * out.
         */
/*        err = 0;
        do {
                err = snd_seq_flush_output(di->handle);
                if (err > 0)
                        usleep(2000);
        } while (err > 0);

#endif

        return ;
*/
}

void AlsaOut::timerEventSend(int type)
{
  snd_seq_event_t ev;

  ev.queue = di->queue;
  ev.dest.client = SND_SEQ_CLIENT_SYSTEM;
  ev.dest.port = SND_SEQ_PORT_SYSTEM_TIMER;

  ev.data.queue.queue = di->queue;

  ev.flags = SND_SEQ_TIME_STAMP_REAL | SND_SEQ_TIME_MODE_REL;
  ev.time.time.tv_sec = 0;
  ev.time.time.tv_nsec = 0;

  ev.type = type;

  snd_seq_event_output(di->handle, &ev);
  snd_seq_flush_output(di->handle);

}

#endif // HAVE_ALSA_SEQ

#ifndef HAVE_ALSA_SEQ
void AlsaOut::noteOn  (uchar , uchar , uchar )
{
#else
void AlsaOut::noteOn  (uchar chn, uchar note, uchar vel)
{
  if (vel==0)
  {
    noteOff(chn,note,vel);
  }
  else
  {
    eventInit(di->ev);
    snd_seq_ev_set_noteon(di->ev,map->channel(chn), map->key(chn,chnpatch[chn],note), vel);
    eventSend(di->ev);
  }
#endif
#ifdef MIDIOUTDEBUG
  printfdebug("Note ON >\t chn : %d\tnote : %d\tvel: %d\n",chn,note,vel);
#endif
}

#ifndef HAVE_ALSA_SEQ
void AlsaOut::noteOff (uchar , uchar , uchar )
{
#else
void AlsaOut::noteOff (uchar chn, uchar note, uchar vel)
{
  eventInit(di->ev);
  snd_seq_ev_set_noteoff(di->ev,map->channel(chn), map->key(chn,chnpatch[chn],note), vel);
  eventSend(di->ev);
#endif
#ifdef MIDIOUTDEBUG
  printfdebug("Note OFF >\t chn : %d\tnote : %d\tvel: %d\n",chn,note,vel);
#endif
}

#ifndef HAVE_ALSA_SEQ
void AlsaOut::keyPressure (uchar , uchar , uchar )
{
#else
void AlsaOut::keyPressure (uchar chn, uchar note, uchar vel)
{
  eventInit(di->ev);
  snd_seq_ev_set_keypress(di->ev,map->channel(chn), map->key(chn,chnpatch[chn],note), vel);
  eventSend(di->ev);
#endif
}

#ifndef HAVE_ALSA_SEQ
void AlsaOut::chnPatchChange (uchar , uchar )
{
#else
void AlsaOut::chnPatchChange (uchar chn, uchar patch)
{
#ifdef MIDIOUTDEBUG
  printfdebug("PATCHCHANGE [%d->%d] %d -> %d\n",
      chn,map->channel(chn),patch,map->patch(chn,patch));
#endif
  eventInit(di->ev);
  snd_seq_ev_set_pgmchange(di->ev,map->channel(chn), map->patch(chn,patch));
  eventSend(di->ev);
  chnpatch[chn]=patch;
#endif
}

#ifndef HAVE_ALSA_SEQ
void AlsaOut::chnPressure (uchar , uchar )
{
#else
void AlsaOut::chnPressure (uchar chn, uchar vel)
{
  eventInit(di->ev);
  snd_seq_ev_set_chanpress(di->ev,map->channel(chn), vel);
  eventSend(di->ev);

  chnpressure[chn]=vel;
#endif
}

#ifndef HAVE_ALSA_SEQ
void AlsaOut::chnPitchBender(uchar ,uchar , uchar )
{
#else
void AlsaOut::chnPitchBender(uchar chn,uchar lsb, uchar msb)
{
  map->pitchBender(chn,lsb,msb);
  chnbender[chn]=((short)msb<<7) | (lsb & 0x7F);
  chnbender[chn]=chnbender[chn]-0x2000;

  eventInit(di->ev);
  snd_seq_ev_set_pitchbend(di->ev,map->channel(chn), chnbender[chn]);
  eventSend(di->ev);
#endif
}

#ifndef HAVE_ALSA_SEQ
void AlsaOut::chnController (uchar , uchar , uchar )
{
#else
void AlsaOut::chnController (uchar chn, uchar ctl, uchar v)
{
  map->controller(chn,ctl,v);
  if ((ctl==11)||(ctl==7))
  {
    v=(v*volumepercentage)/100;
    if (v>127) v=127;
  }

  eventInit(di->ev);
  snd_seq_ev_set_controller(di->ev,map->channel(chn), ctl, v);
  eventSend(di->ev);

  chncontroller[chn][ctl]=v;
#endif
}

#ifndef HAVE_ALSA_SEQ
void AlsaOut::sysex(uchar *, ulong )
{
#else
void AlsaOut::sysex(uchar *data, ulong size)
{
  eventInit(di->ev);
  snd_seq_ev_set_sysex(di->ev, size, data);
  eventSend(di->ev);
#endif

#ifdef MIDIOUTDEBUG
  printfdebug("sysex\n");
#endif
}

#ifndef HAVE_ALSA_SEQ
void AlsaOut::channelSilence (uchar )
{
#else
void AlsaOut::channelSilence (uchar chn)
{
  uchar i;
  for ( i=0; i<127; i++)
  {
    noteOff(chn,i,0);
  }
#endif
}

#ifndef HAVE_ALSA_SEQ
void AlsaOut::channelMute(uchar , int )
{
#else
void AlsaOut::channelMute(uchar chn, int a)
{
  if (a==1)
  {
    chnmute[chn]=a;
    channelSilence(chn);
  }
  else if (a==0)
  {
    chnmute[chn]=a;
  }
  /*  else ignore the call to this procedure */
#endif
}

void AlsaOut::seqbuf_dump (void)
{
  printf("You shouldn't be here.\n");
}

void AlsaOut::seqbuf_clean(void)
{
  printf("You shouldn't be here neither.\n");
}

void AlsaOut::wait(double ticks)
{
//  SEQ_WAIT_TIME(((int)(ticks/convertrate)));
  time=(long int)ticks;

#ifdef MIDIOUTDEBUG
  printfdebug("Wait  >\t ticks: %g\n",ticks);
#endif
}

#ifndef HAVE_ALSA_SEQ
void AlsaOut::tmrSetTempo(int )
{
#else
void AlsaOut::tmrSetTempo(int v)
{
  eventInit(di->ev);
  di->ev->type = SND_SEQ_EVENT_TEMPO;
  snd_seq_ev_set_direct(di->ev);
  di->ev->data.queue.queue = di->queue;
  di->ev->data.queue.param.value = v;
  di->ev->dest.client = SND_SEQ_CLIENT_SYSTEM;
  di->ev->dest.port = SND_SEQ_PORT_SYSTEM_TIMER;
  snd_seq_event_output_direct(di->handle, di->ev);
#ifdef MIDIOUTDEBUG
  printfdebug("SETTEMPO  >\t tempo: %d\n",v);
#endif
#endif
}

#ifndef HAVE_ALSA_SEQ
void AlsaOut::sync(int )
{
#else
void AlsaOut::sync(int i)
{
  if (i==1)
  {
    snd_seq_flush_output(di->handle);
  }

  if (di->timerStarted) 
  {
    eventInit(di->ev);
    di->ev->dest = *di->src;
    eventSend(di->ev);
    snd_seq_flush_output(di->handle);
    snd_seq_event_input(di->handle,&di->ev);
  }

#endif
}

#ifndef HAVE_ALSA_SEQ
void AlsaOut::tmrStart(int )
{
#else
void AlsaOut::tmrStart(int tpcn)
{
  int  ret;
  di->timerStarted=true;
  di->tPCN=tpcn;

#ifdef HAVE_LIBASOUND2
  snd_seq_queue_tempo_t *queuetempo;
  snd_seq_queue_tempo_alloca(&queuetempo);
  snd_seq_queue_tempo_set_ppq(queuetempo, tpcn);
  snd_seq_queue_tempo_set_tempo(queuetempo, 60*1000000/120);
  ret = snd_seq_set_queue_tempo(di->handle, di->queue, queuetempo);
#else
  snd_seq_queue_tempo_t queuetempo;
  memset(&queuetempo, 0, sizeof(queuetempo));
  queuetempo.queue = di->queue;
  queuetempo.ppq = tpcn;
  queuetempo.tempo = 60*1000000/120;
  ret = snd_seq_set_queue_tempo(di->handle, di->queue, &queuetempo);
#endif

  timerEventSend(SND_SEQ_EVENT_START);
  snd_seq_start_queue(di->handle,di->queue,NULL);
#endif
}

void AlsaOut::tmrStop(void)
{
#ifdef HAVE_LIBASOUND
  di->timerStarted=false;
  timerEventSend(SND_SEQ_EVENT_STOP);
#endif
}

void AlsaOut::tmrContinue(void)
{
}

const char * AlsaOut::deviceName(void) const
{
#ifdef HAVE_LIBASOUND
  return di->tgtname;
#else
  return 0L;
#endif
}
