#include <stdio.h>
#include <kapplication.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>

void showMem()
{
   char buf[257];

   printf("Reported Memory Usage Of This Process:\n");

   FILE *fs = fopen("/proc/self/status", "r");
   bool done = false;
   while (!done)
   {
       fgets(buf, 256, fs);
       buf[256] = 0;
       if ((strncmp(buf, "VmLib", 5)==0) ||
           (strncmp(buf, "VmData", 6)==0) ||
           (strncmp(buf, "VmSize", 6)==0) ||
           (strncmp(buf, "VmExe", 5)==0) ||
           (strncmp(buf, "VmRSS", 5)==0) ||
           (strncmp(buf, "VmLck", 5)==0))
          printf("%s", buf);
       done = (strncmp(buf, "VmLib", 5) == 0);
   }
   fclose(fs);
}

long showTotalMem()
{
   long realMem = 0; 
   char buf[257];

   FILE *fs = fopen("/proc/meminfo", "r");
   bool done = false;
   while (!done)
   {
       fgets(buf, 256, fs);
       buf[256] = 0;
       if (strlen(buf)==0) done = true;
       if (strncmp(buf, "Mem:", 4)==0)
       {
          long total = 0;
          long used = 0;
          long free = 0;
          long shared = 0;
          long buffers = 0;
          long cached = 0;
          sscanf(buf, "Mem: %ld %ld %ld %ld %ld %ld", 
             &total, &used, &free, &shared, &buffers, &cached);
          realMem = used-buffers-cached;
          printf("Actual Total Memory Usage: %0.1fKb\n", realMem/1024.0);
          done = true;
       }
   }
   fclose(fs);
   return realMem;
}

long memSize()
{
   long realMem = 0; 
   char buf[257];

   FILE *fs = fopen("/proc/meminfo", "r");
   bool done = false;
   while (!done)
   {
       fgets(buf, 256, fs);
       buf[256] = 0;
       if (strlen(buf)==0) done = true;
       if (strncmp(buf, "Mem:", 4)==0)
       {
          long total = 0;
          long used = 0;
          long free = 0;
          long shared = 0;
          long buffers = 0;
          long cached = 0;
          sscanf(buf, "Mem: %ld %ld %ld %ld %ld %ld", 
             &total, &used, &free, &shared, &buffers, &cached);
          realMem = total;
          done = true;
       }
   }
   fclose(fs);
   return realMem;
}

void *mmapFile(const char *file)
{
   int fd = open(file, O_RDONLY);
   if (fd == 0)
   {
      printf("open: %s\n", strerror(errno));
      exit(-1);
   }   

   struct stat stat_s;
   int result = fstat(fd, &stat_s);
   if (result)
   {
      printf("stat: %s\n", strerror(errno));
      exit(-1);
   }   
     
   void *ptr = mmap(0, stat_s.st_size, PROT_READ, MAP_SHARED, fd, 0);
   if (ptr == 0)
   {
      printf("mmap: %s\n", strerror(errno));
      exit(-1);
   }   
   return ptr;
}

int main(int argc, char *argv[]) 
{
  if ((argc==2) && (strcmp(argv[1], "all")==0))
  {
     long mem_size = memSize();
     mem_size += mem_size / 16;
     printf("Malloc... %0.1fMb\n", mem_size/(1024.0*1024.0));
     char *mem = (char *) malloc(mem_size);
     for(long i=0; i < mem_size; i+=1024)
     {
        mem[i] = 99;
     }
     printf("Done!\n");
     exit(1);
  }

  if ((argc==3) && (strcmp(argv[1], "calc")==0))
  {
     char buf[257];
     int total = 0;
     long val;
     while(true)
     {
        buf[0] = 0;
        fgets(buf, sizeof(buf), stdin);
        if (!strlen(buf))
        {
           printf("%s total = %0.1fKb (%d bytes)\n", argv[2], total/1024.0, total);
           exit(1);
        }
        sscanf(buf, "%lx", &val);
        total += val;
// printf("Val = %ld\n", val);
     }
     exit(1);
  }
  

  if ((argc>=2) && (strcmp(argv[1], "launch")==0))
  {
     showMem();

     char buf[200];
  
     if (argc >=3)
        snprintf(buf, 200, "%s &", argv[2]);
     else
        snprintf(buf, 200, "%s &", argv[0]);

     printf("Waiting for memory usage to settle down....\n");
     long prev = showTotalMem();
     long diff = 0;
     do {
       sleep(15);
       long next = showTotalMem();
       if (next > prev)
          diff = next - prev;
       else
          diff = prev-next;
       prev = next;
     }
     while (diff > 2*1024);

     for(int i=0; i < 5; i++)
     {
        printf("Launching #%d\n", i);
        system(buf);
        sleep(2);
     }

     sleep(10);
     printf("Waiting for memory usage to settle down....\n");
     prev = showTotalMem();
     diff = 0;
     do {
       sleep(15);
       long next = showTotalMem();
       if (next > prev)
          diff = next - prev;
       else
          diff = prev-next;
       prev = next;
     }
     while (diff > 2*1024);
     long fiveMem =  prev;

     for(int i=5; i < 15; i++)
     {
        printf("Launching #%d\n", i);
        system(buf);
        sleep(2);
     }

     sleep(10);
     printf("Waiting for memory usage to settle down....\n");
     prev = showTotalMem();
     diff = 0;
     do {
       sleep(15);
       long next = showTotalMem();
       if (next > prev)
          diff = next - prev;
       else
          diff = prev-next;
       prev = next;
     }
     while (diff > 2*1024);
     long fifteenMem =  prev;
     showMem();

     printf("Actual memory usage of 1 instance = %0.1f Kb\n", 
           (fifteenMem - fiveMem) /10240.0);
  }
//  showMem("second");

    KApplication app(argc,argv,"kurltest");

//  showMem("After KApplication constructor");

//  malloc(10*1024);

//  showMem("After 10K malloc");

  printf("Sleeping...\n");
  sleep(8000);
}
