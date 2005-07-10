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

     // Find the local timezone.
     const KTimezone* timezone = timezones.local();
     printf( "Local timezone: %s\n", timezone->name().latin1() );

     // Find the current offset of the UTC timezone.
     timezone = timezones.zone("UTC");
     printf( "UTC timezone offset should be 0: %d\n", timezone->offset(QDateTime::currentDateTime()) );

     // Find some offsets for Europe/London.
     char *london = "Europe/London";
     timezone = timezones.zone(london);
     QDateTime winter(QDateTime::fromString("2005-01-01T00:00:00", Qt::ISODate));
     QDateTime summer(QDateTime::fromString("2005-06-01T00:00:00", Qt::ISODate));
     printf( "%s winter timezone offset should be 0: %d\n", london, timezone->offset(winter) );
     printf( "%s summer timezone offset should be 3600: %d\n", london, timezone->offset(summer) );
     return 0;
  }

  printf( "Usage: ktimezonestest local!\n" );
  return 1;
}
