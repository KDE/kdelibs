/* vi: set ts=2 sw=2 tw=78:

 Extracts the dcopclient form a DCOPRef

 Copyright 2001 Waldo Bastian <bastian@kde.org>

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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
   const char *dcopref;
   char *delim;
   if ((argc != 2) || (argv[1][0] == '-'))
   {
      fprintf(stderr, "Usage: dcopclient <dcopref>\n");
      return 1;
   }
   dcopref = argv[1];
   if (strncmp(dcopref, "DCOPRef(", 8) != 0)
     goto bad_ref;

   if (dcopref[strlen(dcopref)-1] != ')')
     goto bad_ref;

   delim = strchr(dcopref, ',');
   if (!delim)
     goto bad_ref;

   dcopref += 8;
   *delim = 0;
   puts(dcopref);
   return 0;

bad_ref:
   fprintf(stderr, "Error: '%s' is not a valid DCOP reference.\n", dcopref);
   return 1;
}
