#include <dcopc.h>
#include <stdio.h>

int main(int argc, char **argv)
{
 const char *name;
  
 name = dcop_register("test", 0);

 printf("dcop_register returns \"%s\"\n", name);
}
