/**************************************************************************

    deviceman.cc  - The device manager, that hides the use of midiOut 
    Copyright (C) 1997,98  Antonio Larrosa Jimenez

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

    Send comments and bug fixes to antlarr@arrakis.es
    or to Antonio Larrosa, Rio Arnoya, 10 5B, 29006 Malaga, Spain

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
#include "midimapper.h"
#include "midispec.h"
#include "../version.h"

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
default_dev=def;
initialized=0;
ok=1;
device = NULL;
#ifdef HANDLETIMEINDEVICES
rate=100;
convertrate=10;
#endif
mapper_tmp = NULL;
seqfd=-1;
timerstarted=0;
for (int i=0;i<16;i++) chn2dev[i]=default_dev;
}

DeviceManager::~DeviceManager(void)
{
    closeDev();
    if (device!=NULL)
    {
        for (int i=0;i<n_midi;i++)
            delete device[i];
        delete device;
        device=NULL;
    }
}

int DeviceManager::OK(void)
{
    int r=ok;
    ok=1;
    return r;
}

int DeviceManager::checkInit(void)
{
    if (initialized==0) 
    {
        int r=initManager();
	if (default_dev>=n_total) default_dev=0;
        setMidiMap(mapper_tmp);
        DEBUGPRINTF("check : %d\n",r);
        return r;
    }
    return 0;
}

/*
 midiOut *DeviceManager::chntodev(int chn)
 {
 return device[chn2dev[chn]];
 };
 */

int DeviceManager::initManager(void)
{
    seqfd = open("/dev/sequencer", O_WRONLY | O_NONBLOCK, 0);
    if (seqfd==-1)
    {
        printf("ERROR: Couldn't open /dev/sequencer to get some information\n");
        ok=0;
        return -1;
    };
    n_synths=0;
    n_midi=0;
    ioctl(seqfd,SNDCTL_SEQ_NRSYNTHS,&n_synths);
    ioctl(seqfd,SNDCTL_SEQ_NRMIDIS,&n_midi);
    n_total=n_midi+n_synths;
    if (n_midi==0) 
    {
        printf("ERROR: There's no midi port\n");
/* This could be a problem if the user don't have a synth neither,
but not having any of both things is not a normal thing */
        //    ok=0;
        //    return 1;
    }
    
    device=new midiOut*[n_total];
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
        device[i]=new midiOut(i);
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
                device[i+n_midi]=new fmOut(i,synthinfo[i].nr_voices);
            else if ((synthinfo[i].synth_type==SYNTH_TYPE_SAMPLE)&&
                     (synthinfo[i].synth_subtype==SAMPLE_TYPE_GUS))
                device[i+n_midi]=new gusOut(i,synthinfo[i].nr_voices);
            else
                device[i+n_midi]=new synthOut(i);
        }
    }
    
    close(seqfd);
    
    //#ifdef AT_HOME
    //MidiMapper *map=new MidiMapper("/opt/kde/share/apps/kmid/maps/yamaha790.map");
    //device[0]->useMapper(map);
    //#endif
    
    initialized=1;
    
    return 0;
}

void DeviceManager::openDev(void)
{
    if (checkInit()<0) 
    {
        DEBUGPRINTF("DeviceManager::openDev : Not initialized\n");
        ok = 0;
        return;
    }
    ok=1;
    seqfd = open("/dev/sequencer", O_WRONLY | O_NONBLOCK, 0);
    if (seqfd==-1)
    {
        printf("Couldn't open\n");
        ok=0;
        return;
    }
    _seqbufptr = 0;
    ioctl(seqfd,SNDCTL_SEQ_RESET);
    //ioctl(seqfd,SNDCTL_SEQ_PANIC);
    
#ifndef HANDLETIMEINDEVICES
    rate=0;
    int r=ioctl(seqfd,SNDCTL_SEQ_CTRLRATE,&rate);
    if ((r==-1)||(rate<=0)) rate=HZ;
    
    convertrate=1000/rate;
#endif
    
    DEBUGPRINTF("Opening devices : ");
    for (int i=0;i<n_total;i++) 
    {
        device[i]->openDev(seqfd);
        DEBUGPRINTF("%s ",device[i]->devName());
    }
    DEBUGPRINTF("\n");
    for (int i=0;i<n_total;i++) if (!device[i]->OK()) ok=0;
    if (ok==0)
    {
        for (int i=0;i<n_total;i++) device[i]->closeDev();
        printf("DeviceMan :: ERROR : Closing devices\n");
        return;
    }
    
#ifdef DEVICEMANDEBUG
    printf("Devices opened\n");
    printf("rate: %d\n",rate);
#endif
}

void DeviceManager::closeDev(void)
{
    if (seqfd==-1) return;
#ifndef HANDLETIMEINDEVICES
    tmrStop();
#endif
/*
DEBUGPRINTF("Closing devices : ");
if (device!=NULL) for (int i=0;i<n_total;i++) 
    {
    device[i]->initDev();
    DEBUGPRINTF("%s ",device[i]->devName());
	
//	device[i]->closeDev();
    };
DEBUGPRINTF("\n");
*/
    close(seqfd);
    seqfd=-1;
}

void DeviceManager::initDev(void)
{
    if (device!=NULL) 
    {
        DEBUGPRINTF("Initializing devices :");
        for (int i=0;i<n_total;i++) 
        {
            device[i]->initDev();
            DEBUGPRINTF("%s ",device[i]->devName());
        }
        DEBUGPRINTF("\n");
    }
}

void DeviceManager::noteOn         ( uchar chn, uchar note, uchar vel )
{
    midiOut *midi=chntodev(chn);
    midi->noteOn(chn,note,vel);
}
void DeviceManager::noteOff        ( uchar chn, uchar note, uchar vel )
{
    midiOut *midi=chntodev(chn);
    midi->noteOff(chn,note,vel);
}
void DeviceManager::keyPressure    ( uchar chn, uchar note, uchar vel )
{
    midiOut *midi=chntodev(chn);
    midi->keyPressure(chn,note,vel);
}
void DeviceManager::chnPatchChange ( uchar chn, uchar patch )
{
    midiOut *midi=chntodev(chn);
    midi->chnPatchChange(chn,patch);
}
void DeviceManager::chnPressure    ( uchar chn, uchar vel )
{
    midiOut *midi=chntodev(chn);
    midi->chnPressure(chn,vel);
}
void DeviceManager::chnPitchBender ( uchar chn, uchar lsb,  uchar msb )
{
    midiOut *midi=chntodev(chn);
    midi->chnPitchBender(chn,lsb,msb);
}
void DeviceManager::chnController  ( uchar chn, uchar ctl , uchar v )
{
    midiOut *midi=chntodev(chn);
    midi->chnController(chn,ctl,v);
}
void DeviceManager::sysex          ( uchar *data,ulong size)
{
    for (int i=0;i<n_midi;i++)
        device[i]->sysex(data,size);
}

void DeviceManager::wait (double ticks)
{
    unsigned long int t=(unsigned long int)(ticks/convertrate);
    if (lastwaittime==t) return;
    lastwaittime=t;
#ifdef HANDLETIMEINDEVICES
    device[default_dev]->wait(ticks);
#else
    //printf("%ld\n",t);
    SEQ_WAIT_TIME(t);
    SEQ_DUMPBUF();
#endif
}

void DeviceManager::tmrSetTempo(int v)
{
    //device[default_dev]->tmrSetTempo(v);
    SEQ_SET_TEMPO(v);
    SEQ_DUMPBUF();
}

void DeviceManager::tmrStart(void)
{
#ifdef HANDLETIMEINDEVICES
    device[default_dev]->tmrStart();
#else
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
#ifdef HANDLETIMEINDEVICES
    device[default_dev]->tmrStop();
#else
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
#ifdef HANDLETIMEINDEVICES
    device[default_dev]->tmrContinue();
#else
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

void DeviceManager::sync(int i)
{
#ifdef HANDLETIMEINDEVICES
    device[default_dev]->sync(i);
#else
#ifdef DEVICEMANDEBUG
    printf("Sync %d\n",i);
#endif
    if (i==1) 
    {    
        seqbuf_clean();
        /* If you have any problem, try removing the next 2 lines, 
         I though they would be useful here, but I don't know
         what they exactly do :-) */
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
}

void DeviceManager::seqbuf_clean(void)
{
    _seqbufptr=0;
}


char *DeviceManager::name(int i)
{
    if (checkInit()<0) {ok = 0; return NULL;}
    
    if (i<n_midi) return midiinfo[i].name; 
    if (i<n_midi+n_synths) return synthinfo[i-n_midi].name;
    return (char *)"";
}

const char *DeviceManager::type(int i)
{
    if (checkInit()<0) {ok = 0; return NULL;}
    
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
        return "";
    }
    return "";
}

int DeviceManager::getDefaultDevice(void)
{
    return default_dev;
}

void DeviceManager::setDefaultDevice(int i)
{
    if (i>=n_total) return; 
    default_dev=i;
    for (int i=0;i<16;i++) chn2dev[i]=default_dev;
}

char *DeviceManager::getMidiMapFilename(void)
{
    if (device==NULL) return (char *)"";
    return (device[default_dev]!=NULL) ? 
        device[default_dev]->getMidiMapFilename() : (char *)"";
}

void DeviceManager::setMidiMap(MidiMapper *map)
{
    if (map==NULL) return;
    mapper_tmp=map;
    if (default_dev>=n_total) {default_dev=0;return;};
    if ((device==NULL)||(device[default_dev]==NULL)) 
		return;
    device[default_dev]->useMapper(map);
}

int DeviceManager::setPatchesToUse(int *patchesused)
{
    if (checkInit()<0) return -1;
    
    if ((device[getDefaultDevice()]->devType())==KMID_GUS)
    {
        gusOut *gus=(gusOut *)device[getDefaultDevice()];
        gus->setPatchesToUse(patchesused);
    }
    return 0;
}

void DeviceManager::setVolumePercentage(int v)
{
    if (device!=NULL)
    {
        for (int i=0;i<n_total;i++)
        {
            device[i]->setVolumePercentage(v);
        }
    }
}
