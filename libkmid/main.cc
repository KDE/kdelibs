/**************************************************************************

    main.cc  - The main function for ConsoleKMid 
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
#include "midfile.h"
#include "midiout.h"
#include "track.h"
#include "player.h"
#include <string.h>
#include <unistd.h>
#include "midispec.h"
#include "sndcard.h"
#include "../version.h"

// If you want to use a midi map you have to change the next definition :
#define MAP_PATH NULL
// For example to something like :
//#define MAP_PATH "/home/Antonio/prg/kmid1/kmid/maps/yamaha790.map"
//#define MAP_PATH "/opt/kde/share/apps/kmid/maps/YamahaPSS790_2.map"


PlayerController pctl;


void consoleOutput(void)
{
if ((pctl.ev->command==MIDI_SYSTEM_PREFIX)&&((pctl.ev->command|pctl.ev->chn)==META_EVENT))
    {
    if ((pctl.ev->d1==5)||(pctl.ev->d1==1))
	{
	char *text=new char[pctl.ev->length+1];
	strncpy(text,(char *)pctl.ev->data,pctl.ev->length);
	text[pctl.ev->length]=0;
	printf("%s\n",text);
	};
    if (pctl.ev->d1==ME_SET_TEMPO)
	{
	printf("Tempo : %g\n",tempoToMetronomeTempo((pctl.ev->data[0]<<16)|(pctl.ev->data[1]<<8)|(pctl.ev->data[2])));
	};
    };

};

int main(int argc, char **argv)
{
printf("ConsoleKMid version %s, Copyright (C) 1997,98 Antonio Larrosa Jimenez\n",VERSION_SHORTTXT);
printf("ConsoleKMid comes with ABSOLUTELY NO WARRANTY; for details view file COPYING\n");
printf("This is free software, and you are welcome to redistribute it\n");
printf("under certain conditions\n");
if (argc<2)
    {
    printf("Usage:  %s [nameofmidifile]\n",argv[0]);
    exit(0);
    };

DeviceManager * devman=new DeviceManager(0);
//midiOut *midi=new midiOut();
MidiMapper *map=new MidiMapper(MAP_PATH);
devman->initManager();
player *Player=new player(devman,&pctl);
devman->setMidiMap(map);
pctl.message=0;
pctl.gm=1;
pctl.error=0;
int nmid=1;
while ((nmid<argc)&&(pctl.error!=1))
    {
    Player->loadSong(argv[nmid]);
//    Player->play(1,consoleOutput);
    sleep(20);
    nmid++;
    };
//midi->useMapper(NULL);
//delete midi;
delete devman;
printf("Bye...\n");
//delete map;

return 0;
};
