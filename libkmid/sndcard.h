#ifndef _SNDCARD_H
#define _SNDCARD_H

#ifndef __FreeBSD__
#include <sys/soundcard.h>
#else
#include <machine/soundcard.h>
#endif

#endif
