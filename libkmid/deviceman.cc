/**************************************************************************

    deviceman.cc  - The device manager, that hides the use of midiOut
    This file is part of LibKMid 0.9.5
    Copyright (C) 1997,98,99,2000  Antonio Larrosa Jimenez
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

    $Id$

    Send comments and bug fixes to Antonio Larrosa <larrosa@kde.org>

***************************************************************************/
#include "deviceman.h"
#include "midiout.h"
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <errno.h>
#include "sndcard.h"
#include "synthout.h"
#include "fmout.h"
#include "gusout.h"
#include "alsaout.h"
#include "midimapper.h"
#include "midispec.h"

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif

#ifdef HAVE_LIBASOUND2
#	define HAVE_ALSA_SUPPORT
#	include <sound/asound.h>
#	include <sound/asequencer.h>
#elif defined(HAVE_LIBASOUND)
#	define HAVE_ALSA_SUPPORT
#	include <linux/asequencer.h>
#endif

#ifdef HAVE_ALSA_ASOUNDLIB_H
#	include <alsa/asoundlib.h>
#elif defined(HAVE_SYS_ASOUNDLIB_H)
#	include <sys/asoundlib.h>
#endif

#if 1
#include <kinstance.h>
#include <kglobal.h>
#include <kconfig.h>
#endif

//#define DEVICEMANDEBUG
//#define GENERAL_DEBUG_MESSAGES

SEQ_DEFINEBUF (4096);

#define CONTROLTIMER

#ifdef GENERAL_DEBUG_MESSAGES
void DEBUGPRINTF(const char *format)
{
  printf(format);
}

void DEBUGPRINTF(const char *format,int i)
{
  printf(format,i);
}

void DEBUGPRINTF(const char *format,const char *s)
{
  printf(format,s);
}

#else

void DEBUGPRINTF(const char *) { }
void DEBUGPRINTF(const char *,int ) { }
void DEBUGPRINTF(const char *,const char * ) { }

#endif


DeviceManager::DeviceManager(int def)
{
#if 1
  if (def==-1)
  {
    KInstance *tmp_instance=0L;
    if (!KGlobal::_instance) tmp_instance=new KInstance("nonKDEapp");
    KConfig *config = new KConfig("kcmmidirc", true);

    config->setGroup("Configuration");
    default_dev=config->readNumEntry("midiDevice",0);
    QString mapurl(config->readEntry("mapFilename",""));
    if ((config->readBoolEntry("useMidiMapper", false))&&(!mapurl.isEmpty()))
    {
      mapper_tmp = new MidiMapper( mapurl.mid(mapurl.find(":")+1 ).local8Bit() );
    }
    else
      mapper_tmp = 0L;

    delete config;
    delete tmp_instance;
  }
  else
#endif
  {
    default_dev = def;
    mapper_tmp = 0L;
  }

  initialized=0;
  _ok=1;
  alsa=false;
  device = 0L;
  m_rate=0;
  convertrate=10;
  seqfd=-1;
  timerstarted=0;
  n_midi=0;
  n_synths=0;
  n_total=0;
  midiinfo=0L;
  synthinfo=0L;
  for (int i=0;i<16;i++) chn2dev[i]=default_dev;
}

DeviceManager::~DeviceManager(void)
{
  closeDev();
  if (device)
  {
    for (int i=0;i<n_total;i++)
      delete device[i];
    delete[] device;
    device=0L;
  }
#ifdef HAVE_OSS_SUPPORT
  delete[] midiinfo;
  delete[] synthinfo;
#endif
}

int DeviceManager::ok(void)
{
  int r=_ok;
  _ok=1;
  return r;
}

int DeviceManager::checkInit(void)
{
  if (initialized==0)
  {
    int r=initManager();
    if (default_dev>=n_total) default_dev=0;
    DEBUGPRINTF("check : %d\n",r);
    return r;
  }
  return 0;
}

void DeviceManager::checkAlsa(void)
{
#ifdef HAVE_SYS_STAT_H
  struct stat buf;
  stat("/proc/asound", &buf);
  if ((stat("/proc/asound", &buf) == 0 ) && (S_ISDIR(buf.st_mode)))
    alsa=true;
  else
    alsa=false;
#else
#warning "ALSA won't be found at runtime"
  alsa=false;
#endif
}

int DeviceManager::initManager(void)
{
  checkAlsa();

  if (!alsa)  // We are using OSS
  {
#ifdef HAVE_OSS_SUPPORT
    n_synths=0;
    n_midi=0;
    n_total=0;

    seqfd = open("/dev/sequencer", O_WRONLY | O_NONBLOCK, 0);
    if (seqfd==-1)
    {
      fprintf(stderr,"ERROR: Couldn't open /dev/sequencer to get some information\n");
      _ok=0;
      return -1;
    }
    ioctl(seqfd,SNDCTL_SEQ_NRSYNTHS,&n_synths);
    ioctl(seqfd,SNDCTL_SEQ_NRMIDIS,&n_midi);
    n_total=n_midi+n_synths;


    if (n_midi==0)
    {
      fprintf(stderr,"ERROR: There's no midi port\n");
      /* This could be a problem if the user don't have a synth neither,
	 but not having any of both things is unusual */
      //    _ok=0;
      //    return 1;
    }

    device=new MidiOut*[n_total];
    midiinfo=new midi_info[n_midi];
    synthinfo=new synth_info[n_synths];

    int i;
    for (i=0;i<n_midi;i++)
    {
      midiinfo[i].device=i;
      if (ioctl(seqfd,SNDCTL_MIDI_INFO,&midiinfo[i])!=-1)
      {
#ifdef GENERAL_DEBUG_MESSAGES
	printf("----\n");
	printf("Device : %d\n",i);
	printf("Name : %s\n",midiinfo[i].name);
	printf("Device type : %d\n",midiinfo[i].dev_type);
#endif
      }
      device[i]=new MidiOut(i);
    }

    for (i=0;i<n_synths;i++)
    {
      synthinfo[i].device=i;
      if (ioctl(seqfd,SNDCTL_SYNTH_INFO,&synthinfo[i])!=-1)
      {
#ifdef GENERAL_DEBUG_MESSAGES
	printf("----\n");
	printf("Device : %d\n",i);
	printf("Name : %s\n",synthinfo[i].name);
	switch (synthinfo[i].synth_type)
	{
	  case (SYNTH_TYPE_FM) : printf("FM\n");break;
	  case (SYNTH_TYPE_SAMPLE) : printf("Sample\n");break;
	  case (SYNTH_TYPE_MIDI) : printf("Midi\n");break;
	  default : printf("default type\n");break;
	};
	switch (synthinfo[i].synth_subtype)
	{
	  case (FM_TYPE_ADLIB) : printf("Adlib\n");break;
	  case (FM_TYPE_OPL3) : printf("Opl3\n");break;
	  case (MIDI_TYPE_MPU401) : printf("Mpu-401\n");break;
	  case (SAMPLE_TYPE_GUS) : printf("Gus\n");break;
	  default : printf("default subtype\n");break;
	}
#endif
	if (synthinfo[i].synth_type==SYNTH_TYPE_FM)
	  device[i+n_midi]=new FMOut(i,synthinfo[i].nr_voices);
	else if ((synthinfo[i].synth_type==SYNTH_TYPE_SAMPLE)&&
	    (synthinfo[i].synth_subtype==SAMPLE_TYPE_GUS))
	  device[i+n_midi]=new GUSOut(i,synthinfo[i].nr_voices);
	else
	  device[i+n_midi]=new SynthOut(i);
      }
    }

    close(seqfd);
#else // There's no OSS support and ALSA wasn't detected
      // It must be one of those systems coolo is using :-)

    n_synths=0;
    n_midi=0;
    n_total=0;
    device=0L;
    midiinfo=0L;
    synthinfo=0L;

#endif

  }
  else
  {  // We are using ALSA

#ifdef HAVE_ALSA_SUPPORT
    int  client, port;
#ifdef HAVE_LIBASOUND2
    snd_seq_t *handle=0;
    snd_seq_client_info_t *clienti;
    snd_seq_client_info_malloc(&clienti);
    snd_seq_port_info_t *porti;
    snd_seq_port_info_malloc(&porti);

    snd_seq_open(&handle, "hw", SND_SEQ_OPEN_DUPLEX, 0);
    if (!handle) { printf("handle==0\n"); return -1; };
    snd_seq_system_info_t *info;
    snd_seq_system_info_malloc(&info);
    if (!info) { printf("info==0\n"); return -1; };
    snd_seq_system_info(handle, info);

    n_total=0;
    n_midi=0;
    n_synths=0;

    device=new MidiOut*[snd_seq_system_info_get_clients(info)*snd_seq_system_info_get_ports(info)];
    unsigned int k=SND_SEQ_PORT_CAP_SUBS_WRITE | SND_SEQ_PORT_CAP_WRITE ;
    for (client=0 ; client<snd_seq_system_info_get_clients(info) ; client++)
    {
      snd_seq_get_any_client_info(handle, client, clienti);
      for (port=0 ; port<snd_seq_client_info_get_num_ports(clienti) ; port++)
      {
        snd_seq_get_any_port_info(handle, client, port, porti);
        if (( snd_seq_port_info_get_capability(porti) & k ) == k)
        {
          device[n_midi]=new AlsaOut(n_midi,client, port, snd_seq_client_info_get_name(clienti), snd_seq_port_info_get_name(porti));
          n_midi++;
        };
      }
    }
    snd_seq_client_info_free(clienti);
    snd_seq_port_info_free(porti);
    snd_seq_system_info_free(info);
#else
    snd_seq_t *handle=0;
    snd_seq_client_info_t clienti;
    snd_seq_port_info_t porti;

    snd_seq_open(&handle, SND_SEQ_OPEN);
    if (!handle) { printf("handle(2)==0\n"); return -1; };

    snd_seq_system_info_t info;
    info.clients=info.ports=0;
    snd_seq_system_info(handle, &info);

    n_total=0;
    n_midi=0;
    n_synths=0;

    device=new MidiOut*[info.clients*info.ports];
    unsigned int k=SND_SEQ_PORT_CAP_SUBS_WRITE | SND_SEQ_PORT_CAP_WRITE ;
    for (client=0 ; client<info.clients ; client++)
    {
      snd_seq_get_any_client_info(handle, client, &clienti);
      for (port=0 ; port<clienti.num_ports ; port++)
      {
	snd_seq_get_any_port_info(handle, client, port, &porti);
	if (( porti.capability & k ) == k)
	{
	  device[n_midi]=new AlsaOut(n_midi,client, port, clienti.name, porti.name);
	  n_midi++;
	};
      }
    }
#endif
    n_total=n_midi;

    snd_seq_close(handle);
#else

    // Note: Please don't add i18n for the text below, thanks :)

    fprintf(stderr,"Sorry, this KMid version was compiled without \n");
    fprintf(stderr,"ALSA support but you're using ALSA . \n");
    fprintf(stderr,"Please compile KMid for yourself or tell the people\n");
    fprintf(stderr,"at your Linux distribution to compile it themselves\n");
#endif
  }

  if (mapper_tmp!=0L) setMidiMap(mapper_tmp);

  initialized=1;

  return 0;
}

void DeviceManager::openDev(void)
{
  if (checkInit()<0)
  {
    DEBUGPRINTF("DeviceManager::openDev : Not initialized\n");
    _ok = 0;
    return;
  }
  _ok=1;

  if (!alsa)
  {
#ifdef HAVE_OSS_SUPPORT
    seqfd = open("/dev/sequencer", O_WRONLY | O_NONBLOCK, 0);
    if (seqfd==-1)
    {
      fprintf(stderr,"Couldn't open the MIDI sequencer device (/dev/sequencer)\n");
      _ok=0;
      return;
    }
    _seqbufptr = 0;
    ioctl(seqfd,SNDCTL_SEQ_RESET);
    //ioctl(seqfd,SNDCTL_SEQ_PANIC);
    m_rate=0;
    int r=ioctl(seqfd,SNDCTL_SEQ_CTRLRATE,&m_rate);
    if ((r==-1)||(m_rate<=0)) m_rate=HZ;
    
    convertrate=1000/m_rate;

#endif
  }
  else seqfd=0L; // ALSA

//  DEBUGPRINTF("Opening devices : ");
  for (int i=0;i<n_total;i++)
  {
    device[i]->openDev(seqfd);
//    DEBUGPRINTF("%s ",device[i]->deviceName());
  }
//  DEBUGPRINTF("\n");
  for (int i=0;i<n_total;i++) if (!device[i]->ok()) _ok=0;
  if (_ok==0)
  {
    for (int i=0;i<n_total;i++) device[i]->closeDev();
//    DEBUGPRINTF("DeviceMan :: ERROR : Closing devices\n");
    return;
  }

//  DEBUGPRINTF("Devices opened\n");
}

void DeviceManager::closeDev(void)
{
  if (alsa)
  {
   if (device) 
     for (int i=0;i<n_total;i++) 
       if (device[i]) device[i]->closeDev();

   return;
  }

#ifdef HAVE_OSS_SUPPORT
  if (seqfd==-1) return;
  tmrStop(); 
  if (device)
    for (int i=0;i<n_total;i++)
       if (device[i]) device[i]->closeDev();
  /*
     DEBUGPRINTF("Closing devices : ");
     if (device!=NULL) for (int i=0;i<n_total;i++)
     {
       device[i]->initDev();
       DEBUGPRINTF("%s ",device[i]->deviceName());

  //	device[i]->closeDev();
  };
  DEBUGPRINTF("\n");
   */
  close(seqfd);
  seqfd=-1;
#endif
}

void DeviceManager::initDev(void)
{
  if (device!=0L)
  {
//    DEBUGPRINTF("Initializing devices :");
    for (int i=0;i<n_total;i++)
    {
      device[i]->initDev();
      DEBUGPRINTF("%s ",device[i]->deviceName());
    }
    DEBUGPRINTF("\n");
  }
}

void DeviceManager::noteOn         ( uchar chn, uchar note, uchar vel )
{
  MidiOut *midi=chntodev(chn);
  if (midi) midi->noteOn(chn,note,vel);
}
void DeviceManager::noteOff        ( uchar chn, uchar note, uchar vel )
{
  MidiOut *midi=chntodev(chn);
  if (midi) midi->noteOff(chn,note,vel);
}
void DeviceManager::keyPressure    ( uchar chn, uchar note, uchar vel )
{
  MidiOut *midi=chntodev(chn);
  if (midi) midi->keyPressure(chn,note,vel);
}
void DeviceManager::chnPatchChange ( uchar chn, uchar patch )
{
  MidiOut *midi=chntodev(chn);
  if (midi) midi->chnPatchChange(chn,patch);
}
void DeviceManager::chnPressure    ( uchar chn, uchar vel )
{
  MidiOut *midi=chntodev(chn);
  if (midi) midi->chnPressure(chn,vel);
}
void DeviceManager::chnPitchBender ( uchar chn, uchar lsb,  uchar msb )
{
  MidiOut *midi=chntodev(chn);
  if (midi) midi->chnPitchBender(chn,lsb,msb);
}
void DeviceManager::chnController  ( uchar chn, uchar ctl , uchar v )
{
  MidiOut *midi=chntodev(chn);
  if (midi) midi->chnController(chn,ctl,v);
}
void DeviceManager::sysEx          ( uchar *data,ulong size)
{
  for (int i=0;i<n_midi;i++)
    device[i]->sysex(data,size);
}

void DeviceManager::wait (double ticks)
{
#ifdef HAVE_ALSA_SUPPORT
  if (alsa) { ((AlsaOut *)device[default_dev])->wait(ticks); return; };
#endif

#ifdef HAVE_OSS_SUPPORT
  unsigned long int t=(unsigned long int)(ticks/convertrate);
  if (lastwaittime==t) return;
  lastwaittime=t;
  SEQ_WAIT_TIME(t);
  SEQ_DUMPBUF();     
#endif
}

//void DeviceManager::tmrSetTempo(int v)
void DeviceManager::tmrSetTempo(int v)
{
#ifdef HAVE_ALSA_SUPPORT
  if (alsa) { ((AlsaOut *)device[default_dev])->tmrSetTempo(v); return; }
#endif

#ifdef HAVE_OSS_SUPPORT
  SEQ_SET_TEMPO(v);
  SEQ_DUMPBUF();
#endif
}

void DeviceManager::tmrStart(long int
#ifdef HAVE_ALSA_SUPPORT
tpcn /*name the argument only if it is used*/
#endif
)
{
#ifdef HAVE_ALSA_SUPPORT
  if (alsa) { ((AlsaOut *)device[default_dev])->tmrStart(tpcn); return; }
#endif

#ifdef HAVE_OSS_SUPPORT
#ifdef CONTROLTIMER
  if (!timerstarted)
  {
      SEQ_START_TIMER();
      SEQ_DUMPBUF();
      timerstarted=1;
  }
  lastwaittime=0;
#else
  SEQ_START_TIMER();
  SEQ_DUMPBUF();
#endif          
#endif
}

void DeviceManager::tmrStop(void)
{
#ifdef HAVE_ALSA_SUPPORT
  if (alsa) { ((AlsaOut *)device[default_dev])->tmrStop(); return; }
#endif

#ifdef HAVE_OSS_SUPPORT
#ifdef CONTROLTIMER
  if (timerstarted)
  {
    SEQ_STOP_TIMER();
    SEQ_DUMPBUF();
    timerstarted=0;
  }
#else
  SEQ_STOP_TIMER();
  SEQ_DUMPBUF();
#endif          
#endif
}

void DeviceManager::tmrContinue(void)
{
#ifdef HAVE_ALSA_SUPPORT
  if (alsa) { ((AlsaOut *)device[default_dev])->tmrContinue(); return; }
#endif

#ifdef HAVE_OSS_SUPPORT
#ifdef CONTROLTIMER
  if (timerstarted)
  {
    SEQ_CONTINUE_TIMER();
    SEQ_DUMPBUF();
  }
#else
  SEQ_CONTINUE_TIMER();
  SEQ_DUMPBUF();
#endif        
#endif
}

void DeviceManager::sync(bool f)
{
#ifdef HAVE_ALSA_SUPPORT
  if (alsa) { ((AlsaOut *)device[default_dev])->sync(f); return ; };
#endif

#ifdef HAVE_OSS_SUPPORT
#ifdef DEVICEMANDEBUG
  printf("Sync %d\n",f);
#endif
  if (f)
  {
    seqbuf_clean();
    /* If you have any problem, try removing the next 2 lines,
       I though they would be useful here but the may have side effects */
    ioctl(seqfd,SNDCTL_SEQ_RESET);
    ioctl(seqfd,SNDCTL_SEQ_PANIC);
  }
  else
  {
    seqbuf_dump();
    ioctl(seqfd, SNDCTL_SEQ_SYNC);
  };
#endif
}

void DeviceManager::seqbuf_dump (void)
{
  if (!alsa)
  {
#ifdef HAVE_OSS_SUPPORT
    if (_seqbufptr)
    {
      int r=0;
      unsigned char *sb=_seqbuf;
      int w=_seqbufptr;
      r=write (seqfd, _seqbuf, _seqbufptr);
#ifdef DEVICEMANDEBUG
      printf("%d == %d\n",r,w);
      printf("%d\n",(errno==EAGAIN)? 1 : 0);
#endif
      while (((r == -1)&&(errno==EAGAIN))||(r != w))
      {
	if ((r==-1)&&(errno==EAGAIN))
	{
	  usleep(1);
	}
	else if ((r>0)&&(r!=w))
	{
	  w-=r;
	  sb+=r;
	}
	r=write (seqfd, sb, w);
#ifdef DEVICEMANDEBUG
	printf("%d == %d\n",r,w);
	printf("%d\n",(errno==EAGAIN)? 1 : 0);
#endif
      }
    }
    /*
     *   if (_seqbufptr)
     *       if (write (seqfd, _seqbuf, _seqbufptr) == -1)
     *       {
     *           printf("Error writing to /dev/sequencer in deviceManager::seqbuf_dump\n");
     *           perror ("write /dev/sequencer in seqbuf_dump\n");
     *           exit (-1);
     *       }
     */
    _seqbufptr = 0;
#endif
  }
}

void DeviceManager::seqbuf_clean(void)
{
#ifdef HAVE_ALSA_SUPPORT
  if (alsa)
    ((AlsaOut *)device[default_dev])->seqbuf_clean();
  else
#endif
#ifdef HAVE_OSS_SUPPORT
    _seqbufptr=0;
#endif
}


const char *DeviceManager::name(int i)
{
#ifdef HAVE_OSS_SUPPORT
  if (checkInit()<0) {_ok = 0; return NULL;}

  if (alsa)
  {
    if (i<n_midi) return device[i]->deviceName(); 
  }
  else
  {
    if (i<n_midi) return midiinfo[i].name;
    if (i<n_midi+n_synths) return synthinfo[i-n_midi].name;
  };
#endif
  return (char *)"";
}

const char *DeviceManager::type(int i)
{
#ifdef HAVE_OSS_SUPPORT
  if (checkInit()<0) {_ok = 0; return NULL;}

  if (alsa)
  {
    if (i<n_midi) return "ALSA device";
  }
  else
  {
    if (i<n_midi)
    {
      return "External Midi Port";
    }
    if (i<n_midi+n_synths)
    {
      switch (synthinfo[i-n_midi].synth_subtype)
      {
        case (FM_TYPE_ADLIB) : return "Adlib";break;
        case (FM_TYPE_OPL3) : return "FM";break;
        case (MIDI_TYPE_MPU401) : return "MPU 401";break;
        case (SAMPLE_TYPE_GUS) : return "GUS";break;
      }
    }
  }
#endif
  return "";
}

int DeviceManager::defaultDevice(void)
{
  return default_dev;
}

void DeviceManager::setDefaultDevice(int i)
{
  if (i>=n_total) return;
  default_dev=i;
  for (int i=0;i<16;i++) chn2dev[i]=default_dev;
}

const char *DeviceManager::midiMapFilename(void)
{
  if (device==0L) return "";
  if (default_dev>=n_total) return "";
  return (device[default_dev]!=NULL) ?
    device[default_dev]->midiMapFilename() : "";
}

void DeviceManager::setMidiMap(MidiMapper *map)
{
  if (map==NULL) return;
  mapper_tmp=map;
  if (default_dev>=n_total) {default_dev=0;return;};
  if ((device==0L)||(device[default_dev]==NULL))
    return;
  device[default_dev]->setMidiMapper(map);
}

int DeviceManager::setPatchesToUse(int *patchesused)
{
  if (checkInit()<0) return -1;
  if ((device==0L)||(device[default_dev]==NULL))
    return 0;

  if ((device[default_dev]->deviceType())==KMID_GUS)
  {
    GUSOut *gus=(GUSOut *)device[default_dev];
    gus->setPatchesToUse(patchesused);
  }
  return 0;
}

void DeviceManager::setVolumePercentage(int v)
{
  if (device!=0L)
  {
    for (int i=0;i<n_total;i++)
    {
      device[i]->setVolumePercentage(v);
    }
  }
}

void DeviceManager::setDeviceNumberForChannel(int chn, int dev)
{
  chn2dev[chn]=dev;
}

void DeviceManager::allNotesOff(void)
{
  for (int i=0;i<n_midi;i++)
    device[i]->allNotesOff();
}
