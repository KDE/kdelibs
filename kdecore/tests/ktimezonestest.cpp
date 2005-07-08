#include "ktimezones.h"
#include <kapplication.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) 
{
  KInstance instance("ktimezonestest");

  if ((argc==2) && (strcmp(argv[1], "local")==0))
  {
     KTimezones timezones;
     const KTimezone* timezone = timezones.local();
     printf( "Local timezone: %s\n", timezone->name().latin1() );
     return 0;
  }

  printf( "Usage: ktimezonestest local!\n" );
  return 1;
}
