#ifndef _AWE_SUP_H
#define _AWE_SUP_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef HAVE_AWE_VOICE_H
#include <awe_voice.h>
#define HAVE_AWE32
#elif defined(HAVE_LINUX_AWE_VOICE_H)
#include <linux/awe_voice.h>
#define HAVE_AWE32
#elif defined(HAVE__USR_SRC_SYS_I386_ISA_SOUND_AWE_VOICE_H)
#include "/usr/src/sys/i386/isa/sound/awe_voice.h"
#define HAVE_AWE32
#elif defined(HAVE__USR_SRC_SYS_GNU_I386_ISA_SOUND_AWE_VOICE_H)
#include "/usr/src/sys/gnu/i386/isa/sound/awe_voice.h"
#define HAVE_AWE32
#endif


#endif
