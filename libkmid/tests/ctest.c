#include <libkmid/libkmid.h>
#include <unistd.h>
#include <stdio.h>

int main (int argc, char **argv)
{
  int i;
  char c;

  printf("Test1 %s. Using libkmid from a simple C application\n",kMidVersion());
  printf("%s\n",kMidCopyright());

  if (kMidInit()!=0) 
  {
    printf("Error initializing libkmid\n");
    return 0;
  }

  if (kMidDevices()!=0)
  {
  printf("Available devices :\n");
  for (i=0;i<kMidDevices();i++)
  {
    printf(" %d) %s - %s\n",i,kMidName(i),kMidType(i));
  }
  printf("\nSelect one:");
  c=getc(stdin);
  }
  else
  {
  printf("There's no available devices. Let's pretend we didn't noticed it\n");
  printf("and start playing.\n");
  c='0';
  }

  kMidSetDevice((int)(c-'0'));

  kMidLoad("Kathzy.mid");
  kMidPlay();

  for (i=0;i<45;i++)
  {
    printf("%d/45 seconds\n",i+1);
    sleep(1);
  }

  kMidStop();
  kMidDestruct();
 
  
  return 0;

};

