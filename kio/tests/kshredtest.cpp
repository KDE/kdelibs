/*--------------------------------------------------------------------------*
 KShred.h  Copyright (c) 2000 MieTerra LLC.
 Credits:  Andreas F. Pour <bugs@mieterra.com>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include "kshred.h"
#include <qstring.h>

int main(int argc, char **argv)

{
  if (argc < 3)
  {
    fprintf(stderr, "Error, usage is '%s FILE MODE'\n", argv[0]);
    exit(-1);
  }

  //if (!shred(argv[1]))
    //printf("ERROR shredding %s\n", argv[1]);
  //exit(1);

  KShred::shred(argv[1]);
  printf("Done\n");
  exit(1);

  KShred *shredder = new KShred(argv[1]);

  switch (argv[2][0])
  {
    case '0':
      if (!shredder->fill0s())
        fprintf(stderr, "ERROR in filling 0s\n");
      break;
    case '1':
      if (!shredder->fill1s())
        fprintf(stderr, "ERROR in filling 1s\n");
      break;
    case 'r':
      if (!shredder->fillrandom())
        fprintf(stderr, "ERROR in filling random\n");
      break;
    case 'b':
      if (!shredder->fillbyte((uint) argv[2][1]))
        fprintf(stderr, "ERROR in filling byte '%c'\n", argv[2][1]);
      break;
    case 's':
      if (!shredder->shred())
        fprintf(stderr, "ERROR in shredding\n");
      break;
    default:
      if (!shredder->fillpattern((unsigned char*)(argv[2]), strlen(argv[2])))
        fprintf(stderr, "ERROR in filling with pattern '%s'\n", argv[2]);
      break;
  }
  fprintf(stdout, "Done\n");
  return 0;
}
