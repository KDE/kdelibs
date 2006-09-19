/****************************************************************************

 Copyright (C) 2005 Lubos Lunak        <l.lunak@kde.org>

Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files (the "Software"),
to deal in the Software without restriction, including without limitation
the rights to use, copy, modify, merge, publish, distribute, sublicense,
and/or sell copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.

****************************************************************************/

/*

This utility helps to have some configuration options available in startkde
without the need to launch anything linked to KDE libraries (which may need
some time to load).

The configuration options are written to $KDEHOME/share/config/startupconfigkeys,
one option per line, as <file> <group> <key> <default>. It is possible to
use ' for quoting multiword entries. Values of these options will be written
to $KDEHOME/share/config/startupconfig as a shell script that will set
the values to shell variables, named <file>_<group>_<key> (all spaces replaced
by underscores, everything lowercase). So e.g. line
"ksplashrc KSplash Theme Default" may result in "ksplashrc_ksplash_theme=Default".

In order to real a whole group it is possible to use <file> <[group]>, e.g.
"ksplashrc [KSplash]", which will set shell variables for all keys in the group.
It is not possible to specify default values, but since the configuration options
are processed in the order they are specified this can be solved by first
specifying a group and then all the entries that need default values.

When a kconf_update script is used to update such option, kstartupconfig is run
before kconf_update and therefore cannot see the change in time. To avoid this
problem, together with the kconf_update script also the matching global config
file should be updated (any change, kstartupconfig will see the timestamp change).

Note that the kdeglobals config file is not used as a depedendency for other config
files.

Since the checking is timestamp-based, config files that are frequently updated
should not be used.

Kstartupconfig works by storing every line from startupconfigkeys in file startupconfigfiles
followed by paths of all files that are relevant to the option. Non-existent files
have '!' prepended (for the case they'll be later created), the list of files is
terminated by line containing '*'. If the timestamps of all relevant files are older
than the timestamp of the startupconfigfile file, there's no need to update anything.
Otherwise kdostartupconfig is launched to create or update all the necessary files
(which already requires loading KDE libraries, but this case should be rare).

*/

#include <config.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main()
    {
    char kdehome[ 1024 ];
    if( getenv( "KDEHOME" ))
        strlcpy( kdehome, getenv( "KDEHOME" ), 1024 );
    else if( getenv( "HOME" ))
        {
        strlcpy( kdehome, getenv( "HOME" ), 1024 );
        strlcat( kdehome, "/.kde", 1024  );
        }
    else
        return 1;
    char filename[ 1024 ];
    strlcpy( filename, kdehome, 1024 );
    strlcat( filename, "/share/config/startupconfig", 1024 );
    if( access( filename, R_OK ) != 0 )
        {
        int ret = system( "kdostartupconfig" );
        return WEXITSTATUS( ret );
        }
    strlcpy( filename, kdehome, 1024 );
    strlcat( filename, "/share/config/startupconfigfiles", 1024 );
    struct stat st;
    if( stat( filename, &st ) != 0 )
        {
        int ret = system( "kdostartupconfig" );
        return WEXITSTATUS( ret );
        }
    time_t config_time = st.st_mtime;
    FILE* config = fopen( filename, "r" );
    if( config == NULL )
        {
        int ret = system( "kdostartupconfig" );
        return WEXITSTATUS( ret );
        }
    strlcpy( filename, kdehome, 1024 );
    strlcat( filename, "/share/config/startupconfigkeys", 1024 );
    FILE* keys = fopen( filename, "r" );
    if( keys == NULL )
        {
        fclose( config );
        return 2;
        }
    bool need_update = true;
    for(;;)
        {
        char keyline[ 1024 ];
        if( fgets( keyline, 1023, keys ) == NULL )
            {
            need_update = false;
            break;
            }
        if( char* nl = strchr( keyline, '\n' ))
            *nl = '\0';
        char line[ 1024 ];
        if( fgets( line, 1023, config ) == NULL )
            break;
        if( char* nl = strchr( line, '\n' ))
            *nl = '\0';
        if( strcmp( keyline, line ) != 0 )
            break;
        bool ok = false;
        for(;;)
            {
            if( fgets( line, 1023, config ) == NULL )
                break;
            if( char* nl = strchr( line, '\n' ))
                *nl = '\0';
            if( *line == '\0' )
                break;
            if( *line == '*' )
                {
                ok = true;
                break;
                }
            if( *line == '!' )
                {
                if( access( line + 1, R_OK ) == 0 )
                    break; // file now exists -> update
                }
            else
                {
                struct stat st;
                if( stat( line, &st ) != 0 )
                    break;
                if( st.st_mtime > config_time )
                    break;
                }
            }
        if( !ok )
            break;
        }
    fclose( keys );
    fclose( config );
    if( need_update )
        {
        int ret = system( "kdostartupconfig" );
        return WEXITSTATUS( ret );
        }
    return 0;
    }
