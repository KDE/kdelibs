/* $Id$

 * $Log$
 * Revision 1.2  1998/09/21 18:34:47  bieker
 * Use paths.h if we can.
 *
 * Revision 1.1  1997/10/08 19:28:33  kalle
 * Test program for KSimpleConfig
 *
 */

//
// configtest.cpp: libKDEcore example
//
// demonstrates use of KSimpleConfig class

#include <ksimpleconfig.h>
#include <kapp.h>
#include <unistd.h>

#ifdef HAVE_PATHS_H
#include <paths.h>
#endif

#ifndef _PATH_TMP
#define _PATH_TMP "/tmp/"
#endif

int main( int argc, char **argv )
{
  KApplication  a( argc, argv );

  KSimpleConfig aConfig( _PATH_TMP"/simpleconfig.cfg" );

  aConfig.setGroup( "Test group" );
  aConfig.writeEntry( "key1", "value1" );
  aConfig.writeEntry( "key2", "value2" );

  aConfig.setGroup( "Another Test group" );
  aConfig.writeEntry( "akey1", "avalue1" );
  aConfig.writeEntry( "akey2", "avalue2" );

  fprintf( stderr, "entries written, sleeping for 10 seconds\n" );
  aConfig.sync();
  sleep( 10 );

  aConfig.setGroup( "Test group" );
  aConfig.deleteEntry( "key2", false );
  fprintf( stderr, "Deleted on entry from Test group, sleeping\n" );
  aConfig.sync();
  sleep( 10 );

  aConfig.deleteGroup( "Another Test group", true );
  fprintf( stderr, "Deleted Another Test group\n" );
}

