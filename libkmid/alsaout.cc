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

#ifdef HAVE_LIBASOUND
#include <linux/asequencer.h>
#include <sys/asoundlib.h>
#endif

//#define HANDLETIMEINDEVICES

#ifdef HANDLETIMEINDEVICES
#include <sys/ioctl.h>
#endif

SEQ_USE_EXTBUF();

class AlsaOut::AlsaOutPrivate
{
public:
#ifdef HAVE_LIBASOUND
  AlsaOutPrivate(int _client, int _port, const char *cname,const char *pname)
    {
      handle=0L;
      tPCN=1;
      tgtclient=_client;
      tgtport=_port;
      tgtname=new char[strlen(cname)+strlen(pname)+3];
      strcpy(tgtname, cname);
      strcat(tgtname, "  ");
      strcat(tgtname, pname);
      ev=new snd_seq_event_t;
      if (ev==NULL) { printf("aaaaaaaargh !\n"); };
    }
#else
  AlsaOutPrivate(int, int, const char *,const char *)
    {
    }
#endif

  ~AlsaOutPrivate()
    {
#ifdef HAVE_LIBASOUND
      delete ev;
      delete tgtname;
#endif
    }

#ifdef HAVE_LIBASOUND
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

  void eventInit(snd_seq_event_t *ev);
  void eventSend(snd_seq_event_t *ep);
  void timerEventSend(int type);
#endif
};

AlsaOut::AlsaOut(int d,int _client, int _port, const char *cname,const char *pname) : MidiOut (d)
{
  di = new AlsaOutPrivate( _client, _port, cname, pname);
  seqfd = 0;
  devicetype=KMID_ALSA;
  device= d;

#ifdef HANDLETIMEINDEVICES
  count=0.0;
  lastcount=0.0;
  m_rate=100;
  convertrate=10;
#endif
  volumepercentage=100;
  printf("%d %d %d %s\n",device, di->tgtclient, di->tgtport, di->tgtname);

  map=new MidiMapper(NULL);

  if (map==NULL) { printfdebug("ERROR : alsaOut : Map is NULL\n"); return; };
  _ok=1;
};

AlsaOut::~AlsaOut()
{
  delete map;
  closeDev();
  delete di;
}

void AlsaOut::openDev (int)
{
#ifndef HAVE_LIBASOUND
  return;
#else
  _ok=1;
  if (snd_seq_open(&di->handle, SND_SEQ_OPEN) < 0)
                fprintf(stderr, "Couldn't open sequencer: %s", snd_strerror(errno));

  di->queue = snd_seq_alloc_queue(di->handle);
  di->client = snd_seq_client_id(di->handle);
  di->tgt = new snd_seq_addr_t;
  di->tgt->client=di->tgtclient;
  di->tgt->port=di->tgtport;

  di->src = new snd_seq_addr_t;
  di->src->client = di->client;
  di->src->port = snd_seq_create_simple_port(di->handle, NULL,
	SND_SEQ_PORT_CAP_WRITE | SND_SEQ_PORT_CAP_SUBS_WRITE
	| SND_SEQ_PORT_CAP_READ, SND_SEQ_PORT_TYPE_MIDI_GENERIC);

  int r=snd_seq_connect_to(di->handle, di->src->port, di->tgt->client, di->tgt->port);
  if (r < 0) { _ok=0; fprintf(stderr, "Cannot connect to %d:%d\n",di->tgtclient,di->tgtport); }
  time=0;
#endif
}

void AlsaOut::closeDev (void)
{
  if (!ok()) return;

#ifdef HAVE_LIBASOUND
  snd_seq_delete_simple_port(di->handle,di->src->port);
  snd_seq_free_queue(di->handle, di->queue);
  snd_seq_close(di->handle);
#endif
}

void AlsaOut::initDev (void)
{
#ifdef HAVE_LIBASOUND
  int chn;
  if (!ok()) return;
#ifdef HANDLETIMEINDEVICES
  count=0.0;
  lastcount=0.0;
#endif
  uchar gm_reset[5]={0x7e, 0x7f, 0x09, 0x01, 0xf7};
  sysex(gm_reset, sizeof(gm_reset));
  for (chn=0;chn<16;chn++)
  {
    chnmute[chn]=0;
    chnPatchChange(chn,0);
    chnPressure(chn,64);
    chnPitchBender(chn, 0x00, 0x40);
    chnController(chn, CTL_MAIN_VOLUME,110*volumepercentage);
    chnController(chn, CTL_EXT_EFF_DEPTH, 0);
    chnController(chn, CTL_CHORUS_DEPTH, 0);
    chnController(chn, 0x4a, 127);
  }
#endif
}

#ifdef HAVE_LIBASOUND
void AlsaOut::AlsaOutPrivate::eventInit(snd_seq_event_t *ev)
{
  snd_seq_ev_clear(ev);
  snd_seq_real_time_t tmp;
  tmp.tv_sec=(time)/1000;
  tmp.tv_nsec=(time%1000)*1000000;
//  printf("time : %d %d %d\n",(int)time,(int)tmp.tv_sec, (int)tmp.tv_nsec);
  ev->source = *src;
  ev->dest = *tgt;

  snd_seq_ev_schedule_real(ev, queue, 0, &tmp);

}

void AlsaOut::AlsaOutPrivate::eventSend(snd_seq_event_t *ev)
{
    int err = snd_seq_event_output(handle, ev);
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
                err = snd_seq_flush_output(handle);
                if (err > 0)
                        usleep(2000);
        } while (err > 0);

#endif

        return ;
*/
}

void AlsaOut::AlsaOutPrivate::timerEventSend(int type)
{
  snd_seq_event_t ev;

  ev.queue = queue;
  ev.dest.client = SND_SEQ_CLIENT_SYSTEM;
  ev.dest.port = SND_SEQ_PORT_SYSTEM_TIMER;

  ev.data.queue.queue = queue;

  ev.flags = SND_SEQ_TIME_STAMP_REAL | SND_SEQ_TIME_MODE_REL;
  ev.time.time.tv_sec = 0;
  ev.time.time.tv_nsec = 0;

  ev.type = type;

  snd_seq_event_output(handle, &ev);
  snd_seq_flush_output(handle);

}

#endif // HAVE_LIBASOUND

void AlsaOut::noteOn  (uchar chn, uchar note, uchar vel)
{
#ifdef HAVE_LIBASOUND
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

void AlsaOut::noteOff (uchar chn, uchar note, uchar vel)
{
#ifdef HAVE_LIBASOUND
  eventInit(di->ev);
  snd_seq_ev_set_noteoff(di->ev,map->channel(chn), map->key(chn,chnpatch[chn],note), vel);
  eventSend(di->ev);
#endif
#ifdef MIDIOUTDEBUG
  printfdebug("Note OFF >\t chn : %d\tnote : %d\tvel: %d\n",chn,note,vel);
#endif
}

void AlsaOut::keyPressure (uchar chn, uchar note, uchar vel)
{
#ifdef HAVE_LIBASOUND
  eventInit(di->ev);
  snd_seq_ev_set_keypress(di->ev,map->channel(chn), map->key(chn,chnpatch[chn],note), vel);
  eventSend(di->ev);
#endif
}

void AlsaOut::chnPatchChange (uchar chn, uchar patch)
{
#ifdef MIDIOUTDEBUG
  printfdebug("PATCHCHANGE [%d->%d] %d -> %d\n",
      chn,map->channel(chn),patch,map->patch(chn,patch));
#endif
#ifdef HAVE_LIBASOUND
  eventInit(di->ev);
  snd_seq_ev_set_pgmchange(di->ev,map->channel(chn), map->patch(chn,patch));
  eventSend(di->ev);
  chnpatch[chn]=patch;
#endif
}

void AlsaOut::chnPressure (uchar chn, uchar vel)
{
#ifdef HAVE_LIBASOUND
  eventInit(di->ev);
  snd_seq_ev_set_chanpress(di->ev,map->channel(chn), vel);
  eventSend(di->ev);

  chnpressure[chn]=vel;
#endif
}

void AlsaOut::chnPitchBender(uchar chn,uchar lsb, uchar msb)
{
#ifdef HAVE_LIBASOUND
  map->pitchBender(chn,lsb,msb);
  chnbender[chn]=(msb << 8) | (lsb & 0xFF);

  eventInit(di->ev);
  snd_seq_ev_set_pitchbend(di->ev,map->channel(chn), chnbender[chn]);
  eventSend(di->ev);
#endif
}

void AlsaOut::chnController (uchar chn, uchar ctl, uchar v)
{
#ifdef HAVE_LIBASOUND
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

void AlsaOut::sysex(uchar *data, ulong size)
{
#ifdef HAVE_LIBASOUND
  eventInit(di->ev);
  snd_seq_ev_set_sysex(di->ev, size, data);
  eventSend(di->ev);
#endif

#ifdef MIDIOUTDEBUG
  printfdebug("sysex\n");
#endif
}

void AlsaOut::channelSilence (uchar chn)
{
#ifdef HAVE_LIBASOUND
  uchar i;
  for ( i=0; i<127; i++)
  {
    noteOff(chn,i,0);
  }
#endif
}

void AlsaOut::channelMute(uchar chn, int a)
{
#ifdef HAVE_LIBASOUND
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

void AlsaOut::tmrSetTempo(int v)
{
/*  eventInit(ev);
  ev->type = SND_SEQ_EVENT_TEMPO;
  ev->data.queue.queue = queue;
  ev->data.queue.param.value = v;
printf("tempo _ : _ : _ : %d\n",v);
  ev->dest.client = SND_SEQ_CLIENT_SYSTEM;
  ev->dest.port = SND_SEQ_PORT_SYSTEM_TIMER;
  eventSend(ev);
*/
#ifdef MIDIOUTDEBUG
  printfdebug("SETTEMPO  >\t tempo: %d\n",v);
#endif
#ifdef HAVE_LIBASOUND
  snd_seq_queue_tempo_t ev;
  ev.queue=queue;
  ev.tempo=v;
  ev.ppq=di->tPCN;
  snd_seq_set_queue_tempo(di->handle,queue,&ev);
#endif
}

void AlsaOut::sync(int i)
{
//#ifdef MIDIOUTDEBUG
  printf("Alsa Sync %d\n",i);
//#endif
#ifdef HAVE_LIBASOUND
  if (i==1)
  {
    snd_seq_flush_output(di->handle);
    snd_seq_drain_output(di->handle);
  }

  printf("Alsa 1\n");

  eventInit(di->ev);
  di->ev->dest = *di->src;
  eventSend(ev);
  snd_seq_flush_output(di->handle);
  printf("Alsa 2\n");
  snd_seq_event_input(di->handle,&di->ev);

  printf("Alsa 3\n");
#endif
}

void AlsaOut::tmrStart(int tpcn)
{
#ifdef HAVE_LIBASOUND
  snd_seq_queue_tempo_t queuetempo;
  int  ret;

  memset(&queuetempo, 0, sizeof(queuetempo));
  queuetempo.queue = queue;
  queuetempo.ppq = tpcn;
  di->tPCN=tpcn;
  queuetempo.tempo = 60*1000000/120;

  ret = snd_seq_set_queue_tempo(di->handle, queue, &queuetempo);

  timerEventSend(SND_SEQ_EVENT_START);
  snd_seq_start_queue(di->handle,queue,NULL);
#endif
}

void AlsaOut::tmrStop(void)
{
#ifdef HAVE_LIBASOUND
  timerEventSend(SND_SEQ_EVENT_STOP);
#endif
}

void AlsaOut::tmrContinue(void)
{
  printf("Is this used ?\n");
}


const char * AlsaOut::deviceName(void) const
{
#ifdef HAVE_LIBASOUND
  return di->tgtname;
#else
  return 0L;
#endif
}
