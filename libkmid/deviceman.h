#ifndef DEVICEMAN_H
#define DEVICEMAN_H

#include "dattypes.h"

class midiOut;
class MidiMapper;
class DeviceManager
{
protected:

midiOut **device; 	// The midi devices objects
struct midi_info *midiinfo;   // Midi info
struct synth_info *synthinfo; // Synth info
int chn2dev[16];    // Stores the device thru which a channel will be sent
int n_synths;  // Number of synths devices
int n_midi;  // Number of midi ports
int n_total; // n_midi+n_synths

int initialized;
int seqfd; // The real file handler that is opened and closed.
int default_dev; // The device to which timer events will be sent

int ok;

public:

	DeviceManager(int def=0);
	~DeviceManager(void);

  int initManager(void); // Returns 0 if no error. -1 in case of it couldn't
			//  initialize
  int checkInit(void); // calls initManager if it hasn't been called and
			// does nothing in other case. Returns 0 if initialized
			// -1 if error 

  midiOut *chntodev(int chn);
  int OK(void) {int r=ok;ok=1;return r;};

// The following funtion are here to emulate a midi, so that the DeviceManager
// send the events to the appropiate devices.

  void openDev        (void);
  void closeDev       (void);
  void initDev        (void);

  void noteOn         ( uchar chn, uchar note, uchar vel );
  void noteOff        ( uchar chn, uchar note, uchar vel );
  void keyPressure    ( uchar chn, uchar note, uchar vel );
  void chnPatchChange ( uchar chn, uchar patch );
  void chnPressure    ( uchar chn, uchar vel );
  void chnPitchBender ( uchar chn, uchar lsb,  uchar msb );
  void chnController  ( uchar chn, uchar ctl , uchar v );
  void sysex          ( uchar *data,ulong size);

  void wait (double ticks);

  void tmrSetTempo(int v);
  void tmrStart(void);
  void tmrStop(void);
  void tmrContinue(void);

  void sync(int i=0);

  int getDefaultDevice(void);
  void setDefaultDevice(int i);

  char *getMidiMapFilename(void);
  void setMidiMap(MidiMapper *map);

  int numberOfMidiPorts(void) {return n_midi;};
  int numberOfSynthDevices(void) {return n_synths;};
  char *name(int i);
  char *type(int i);

};

#endif
