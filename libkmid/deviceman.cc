#include "deviceman.h"
#include "midiout.h"
#include <stdio.h>
#include "sndcard.h"
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include "synthout.h"
#include "midimapper.h"

SEQ_DEFINEBUF (1024);
//#define AT_HOME


DeviceManager::DeviceManager(int def)
{
default_dev=def;
initialized=0;
ok=1;
device = NULL;
seqfd=-1;
for (int i=0;i<16;i++) chn2dev[i]=default_dev;
};

DeviceManager::~DeviceManager(void)
{
closeDev();
for (int i=0;i<n_midi;i++)
	delete device[i];
delete device;
device=NULL;
};

int DeviceManager::checkInit(void)
{
if (initialized==0) return initManager();
return 0;
};


midiOut *DeviceManager::chntodev(int chn)
{
return device[chn2dev[chn]];
};

int DeviceManager::initManager(void)
{
seqfd = open("/dev/sequencer", O_WRONLY, 0);
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
    printf("ERROR: There's no midi port");
//    ok=0;
//    return 1;
    };
device=new midiOut*[n_total];
midiinfo=new midi_info[n_midi];
synthinfo=new synth_info[n_synths];

int i;
for (i=0;i<n_midi;i++)
    {
    midiinfo[i].device=i;
    if (ioctl(seqfd,SNDCTL_MIDI_INFO,&midiinfo[i])!=-1)
        {
        printf("----");
        printf("Device : %d\n",i);
        printf("Name : %s\n",midiinfo[i].name);
        printf("Device type : %d\n",midiinfo[i].dev_type);
        };
    device[i]=new midiOut(i);
    };
 
for (i=0;i<n_synths;i++)
    {
    synthinfo[i].device=i;
    if (ioctl(seqfd,SNDCTL_SYNTH_INFO,&synthinfo[i])!=-1)
        {
        printf("----");
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
            };
        device[i+n_midi]=new synthOut(i);
        };
    };

close(seqfd);

#ifdef AT_HOME
MidiMapper *map=new MidiMapper("/opt/kde/share/apps/kmid/maps/yamaha790.map");
device[0]->useMapper(map);
#endif

initialized=1;

return 0;
};

void DeviceManager::openDev(void)
{
if (checkInit()<0) {ok = 0; return;};
ok=1;
seqfd = open("/dev/sequencer", O_WRONLY, 0);
if (seqfd==-1)
   {
   printf("Couldn't open\n");
   ok=0;
   return;
   };
for (int i=0;i<n_total;i++) device[i]->openDev(seqfd);
for (int i=0;i<n_total;i++) if (!device[i]->OK()) ok=0;
if (ok==0)
   {
   for (int i=0;i<n_total;i++) device[i]->closeDev();
   printf("DeviceMan :: ERROR : Closing devices\n");
   return;
   };
printf("Devices opened\n");
};

void DeviceManager::closeDev(void)
{
if (seqfd==-1) return;
for (int i=0;i<n_total;i++) device[i]->initDev();
close(seqfd);
seqfd=-1;
};

void DeviceManager::initDev(void)
{
for (int i=0;i<n_total;i++) device[i]->initDev();
};

void DeviceManager::noteOn         ( uchar chn, uchar note, uchar vel )
{
midiOut *midi=chntodev(chn);
midi->noteOn(chn,note,vel);
};

void DeviceManager::noteOff        ( uchar chn, uchar note, uchar vel )
{
midiOut *midi=chntodev(chn);
midi->noteOff(chn,note,vel);
};
void DeviceManager::keyPressure    ( uchar chn, uchar note, uchar vel )
{
midiOut *midi=chntodev(chn);
midi->keyPressure(chn,note,vel);
};
void DeviceManager::chnPatchChange ( uchar chn, uchar patch )
{
midiOut *midi=chntodev(chn);
midi->chnPatchChange(chn,patch);
};
void DeviceManager::chnPressure    ( uchar chn, uchar vel )
{
midiOut *midi=chntodev(chn);
midi->chnPressure(chn,vel);
};
void DeviceManager::chnPitchBender ( uchar chn, uchar lsb,  uchar msb )
{
midiOut *midi=chntodev(chn);
midi->chnPitchBender(chn,lsb,msb);
};
void DeviceManager::chnController  ( uchar chn, uchar ctl , uchar v )
{
midiOut *midi=chntodev(chn);
midi->chnController(chn,ctl,v);
};
void DeviceManager::sysex          ( uchar *data,ulong size)
{
for (int i=0;i<n_midi;i++)
    device[i]->sysex(data,size);
};

void DeviceManager::wait (double ticks)
{
device[default_dev]->wait(ticks);
};

void DeviceManager::tmrSetTempo(int v)
{
device[default_dev]->tmrSetTempo(v);
};

void DeviceManager::tmrStart(void)
{
device[default_dev]->tmrStart();
};

void DeviceManager::tmrStop(void)
{
device[default_dev]->tmrStop();
};

void DeviceManager::tmrContinue(void)
{
device[default_dev]->tmrContinue();
};

void DeviceManager::sync(int i)
{
device[default_dev]->sync(i);
};

char *DeviceManager::name(int i)
{
if (checkInit()<0) {ok = 0; return NULL;};

if (i<n_midi) return midiinfo[i].name; 
if (i<n_midi+n_synths) return synthinfo[i-n_midi].name;
return (char *)"";
};

char *DeviceManager::type(int i)
{
if (checkInit()<0) {ok = 0; return NULL;};

if (i<n_midi) 
    {
    return "External Midi Port"; 
    };
if (i<n_midi+n_synths) 
    {
    switch (synthinfo[i-n_midi].synth_subtype)
        {
        case (FM_TYPE_ADLIB) : return "Adlib";break;
        case (FM_TYPE_OPL3) : return "FM";break;
        case (MIDI_TYPE_MPU401) : return "MPU 401";break;
        case (SAMPLE_TYPE_GUS) : return "GUS";break;
        };
    return (char *)"";
    };
return (char *)"";
};

int DeviceManager::getDefaultDevice(void)
{
return default_dev;
};

void DeviceManager::setDefaultDevice(int i)
{
if (i>=n_total) return;
default_dev=i;
for (int i=0;i<16;i++) chn2dev[i]=default_dev; 
};

char *DeviceManager::getMidiMapFilename(void)
{
if (device==NULL) return (char *)"";
return (device[default_dev]!=NULL) ? 
	device[default_dev]->getMidiMapFilename() : (char *)"";
};

void DeviceManager::setMidiMap(MidiMapper *map)
{
if ((device==NULL)||(device[default_dev]==NULL)||(map==NULL)) return;
device[default_dev]->useMapper(map);
};
