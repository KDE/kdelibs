#ifndef _SNDCARD_H
#define _SNDCARD_H

#ifndef __FreeBSD__
#include <sys/soundcard.h>
#else
#include <machine/soundcard.h>
#endif


#ifndef HZ
#define HZ 100
#endif
#ifndef MIDI_TYPE_MPU401
#define MIDI_TYPE_MPU401 0x401
#endif


#endif
