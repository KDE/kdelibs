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
#include <stdlib.h>
#ifndef __FreeBSD__
// Note: getopt_long seems not to be defined on FreeBSD systems
#include <getopt.h>
#endif
#include <limits.h>
#include <g++/iostream.h>
#include "midispec.h"
#include "sndcard.h"
#include "songlist.h"
#include "../version.h"

/*
 These are the default values for some options, edit them if you want:
 */

#define DEFAULT_DEVICE 0
/*
 You can run 'consolekmid -l' to know the available devices
 */

#define DEFAULT_MAP "/opt/kde/share/apps/kmid/maps/YamahaPSS790.map"
/*
 #define DEFAULT_MAP NULL
 If you want to use a midi mapper, use something like this :
 #define DEFAULT_MAP "/opt/kde/share/apps/kmid/maps/YamahaPSS790.map"
 */

#define DEFAULT_VOLUME 100
/*
 This specifies the volume with which you will hear the song, 100 means that
 it will be played just as the song says, 50 plays music at half volume
 and 200 at double volume (but probably 120 will already be your synth
 maximum, so you will not hear a difference between 120 and 200, but that
 depends on the song)
 */

/*
 Don't edit beyond this line , unless you know what you are doing :-)
 */

PlayerController pctl;
int quiet=0;

void consoleOutput(void)
{
    if ((pctl.ev->command==MIDI_SYSTEM_PREFIX)&&((pctl.ev->command|pctl.ev->chn)==META_EVENT)&&(!quiet))
    {
        if ((pctl.ev->d1==5)||(pctl.ev->d1==1))
        {
            char *text=new char[pctl.ev->length+1];
            strncpy(text,(char *)pctl.ev->data,pctl.ev->length);
            text[pctl.ev->length]=0;
            char c=text[0];
            if ((c==0)||(c==10)||(c==13)||(c=='\\')||(c=='/')||(c=='@')) cout << endl;
            cout << text;
            cout.flush();
        };
        if (pctl.ev->d1==ME_SET_TEMPO)
        {
            printf("Tempo : %g\n",tempoToMetronomeTempo((pctl.ev->data[0]<<16)|(pctl.ev->data[1]<<8)|(pctl.ev->data[2])));
        };
    };
    
};

int main(int argc, char **argv)
{
    int option_index=0;
    int help=0,error=0,list_dev=0;
    int vol_percentage=DEFAULT_VOLUME;
    int device=DEFAULT_DEVICE;
    int gm=1;
    int interactive=0; 
    char *map_path=NULL;
    char *inputfile=NULL;
    FILE *inputfh=NULL;
    if (DEFAULT_MAP!=NULL)
    {
        map_path=new char[strlen(DEFAULT_MAP)+1];
        strcpy(map_path,DEFAULT_MAP);
    };
    int c;
#ifndef __FreeBSD__
    struct option long_options[]=
    {
        {"device",1,NULL,'d'},
        {"list",0,NULL,'l'},
        {"map",1,NULL,'m'},
        {"map",1,NULL,'m'},
        {"volume",1,NULL,'v'},
        {"file",1,NULL,'f'},
        {"mt32",0,NULL,'3'},
        {"interactive",0,NULL,'i'},
        {"quiet",0,NULL,'q'},
        {"help",0,NULL,'h'},
        { 0, 0, 0, 0}
    };
    c=getopt_long( argc, argv, "d:lm:v:f:3iqh" , long_options, &option_index);
#else
    c=getopt( argc, argv, "d:lm:v:f:3iqh");
#endif
    
    long l;
    while (c!=-1)
    {
        switch (c)
        {
        case 'd' :
            l=strtol(optarg,NULL,10);
            if ((l==LONG_MAX)||(l==LONG_MIN)||(l<0))
            {
                printf("Error: Invalid argument, must be a number between 0 and 200\n");
            } else device=l;
            break;
        case 'm' :
            if (map_path!=NULL) delete map_path;
            map_path=new char[strlen(optarg)+1];
            strcpy(map_path,optarg);
            break;
        case 'v' :
            l=strtol(optarg,NULL,10);
            if ((l==LONG_MAX)||(l==LONG_MIN)||(l<0)||(l>200))
            {
                printf("Error: Invalid argument, must be a number between 0 and 200\n");
            } else vol_percentage=l;
            break;
        case 'f' :
            if (inputfile!=NULL) delete inputfile;
            inputfile=new char[strlen(optarg)+1];
            strcpy(inputfile,optarg);
            break;
        case 'h' :  help=1;break;
        case 'l' :  list_dev=1;break;
        case '3' :  gm=0;break;
        case 'i' :  interactive=1;break;
        case 'q' :  quiet=1;break;
        case 0 :
        case ':' :
        case '?' :
        default :
            printf("Try '%s --help' for more information\n",argv[0]);
            error=1;
            break;
        }
#ifndef __FreeBSD__
        c=getopt_long( argc, argv, "d:lm:v:f:3iqh" , long_options, &option_index);
#else
        c=getopt( argc, argv, "d:lm:v:f:3iqh");
#endif
    };
    
    if (inputfile!=NULL)
    {
        inputfh=fopen(inputfile,"rt");
        if (inputfh==NULL)
        {
            printf("Error: Cannot open input file %s\n",inputfile);
            error=1;
        };
        delete inputfile;
    };
    
    if (error) exit(0);
    printf("ConsoleKMid version %s, Copyright (C) 1997,98 Antonio Larrosa Jimenez\n",VERSION_SHORTTXT);
    if (!quiet)
    {
        printf("ConsoleKMid comes with ABSOLUTELY NO WARRANTY; for details view file COPYING\n");
        printf("This is free software, and you are welcome to redistribute it\n");
        printf("under certain conditions\n");
    };
    
    if ((argc<2)||(help))
    {
        printf("\nUsage:  %s [Options] MidiFiles \n",argv[0]);
        printf("Options :\n"
               "  -d, --device=DEVICE\tOutput device to use\n"
               "  -l, --list \t\tList midi devices\n"
               "  -m, --map=MAPFILE\tMidi map to use\n"
               "  -v, --volume=VOLUME\tVolume percentage (n is between 0 and 200 %%)\n"
               "  -f, --file=INPUTFILE\tTakes the midi files to play from a text file\n"
               "  -3, --mt32 \t\tEmulate a mt32 synth\n"
               "  -i, --interactive\t\tAsk for each file if it must be played\n"
               "  -h, --help \t\tDisplay this help and exit\n");
        printf("\nPlease report bugs to Antonio Larrosa (antlarr@arrakis.es)\n");
        exit(0);
    };
    
    
    DeviceManager * devman=new DeviceManager(device);
    devman->initManager();
    if (!devman->OK()) exit(0);
    
    if (list_dev)
    {
        printf("Available devices :\n");
        for (int i=0;i<devman->numberOfMidiPorts()+devman->numberOfSynthDevices();i++)
        {
            if (strcmp(devman->type(i),"")!=0)
                printf("%d) %s - %s\n",i,devman->name(i),devman->type(i));
            else
                printf("%d) %s\n",i,devman->name(i));
        };
        delete devman;
        exit(0);
    };
    MidiMapper *map=new MidiMapper(map_path);
    if (!map->OK()) exit(0);
    devman->setMidiMap(map);
    
    SongList *songlist=new SongList();
    int nmid=optind;
    while (nmid<argc)
    {
        songlist->AddSong(argv[nmid]);
        nmid++;
    };
    if (inputfh!=NULL)
    {
        char s[400];
        while (!feof(inputfh))
        {
            if (fscanf(inputfh,"%s",s)!=EOF) songlist->AddSong(s);
        };
        fclose(inputfh);
    };
    
    player *Player=new player(devman,&pctl);
    Player->setParseSong(false);
    pctl.message=0;
    pctl.gm=gm;
    pctl.error=0;
    pctl.volumepercentage=vol_percentage;
    for (int i=0;i<16;i++)
    {
        pctl.forcepgm[i]=0;
        pctl.pgm[i]=0;
    };
    
    songlist->setActiveSong(1);
    int ok=1;
    char *name;
    int playfile=1;
    while (ok)
    {
        name=songlist->getActiveSongName();
        if (interactive==1)
        {
            cout << "Do you want to hear " << name << " ? ";
            char c[20];
            cin >> c;
            if ((c[0]=='N')||(c[0]=='n')) playfile=0;
            else 
            {
                if (c[0]=='q') 
                {
                    playfile=0;
                    break;
                }
                else
                {	
                    playfile=1;
                    if ((c[0]=='A')||(c[0]=='a')||(c[0]=='*')) interactive=0;
                };
            };
        };
        if (playfile)
        {
            cout << "Loading song : " << name << endl;
            if (strncmp(name,"file:",5)==0) name+=5;
            if (Player->loadSong(name)==0)
                Player->play(1,consoleOutput);
            if (!quiet) cout << endl;
        };
        ok=songlist->next();
    };
    delete songlist;
    delete Player;
    delete devman;
    exit(0);
};
