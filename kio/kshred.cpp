/*--------------------------------------------------------------------------*
 | KShred.cpp  Copyright (c) 2000 MieTerra LLC. All rights reserved.        |
 |                                                                          |
 |     The installation, use, copying, compilation, modification and        |
 | distribution of this work is subject to the 'Artistic License'.          |
 | You should have received a copy of that License Agreement along with     |
 | this file; if not, you can obtain a copy at                              |
 | http://www.mieterra.com/legal/MieTerra_Artistic.html.                    |
 | THIS PACKAGE IS PROVIDED TO YOU "AS IS", WITHOUT ANY WARRANTY            |
 | OR GUARANTEE OF ANY KIND. THE AUTHORS AND DISTRIBUTORS OF THE            |
 | PACKAGE OR ANY PART THEREOF SPECIFICALLY DISCLAIM ALL                    |
 | WARRANTIES OF ANY KIND, EITHER EXPRESSED OR IMPLIED (EITHER IN           |
 | FACT OR BY LAW), INCLUDING, BUT NOT LIMITED TO, THE IMPLIED              |
 | WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR               |
 | PURPOSE, ANY WARRANTY THAT THE PACKAGE OR ANY PART THEREOF               |
 | IS FREE OF DEFECTS AND THE WARRANTIES OF TITLE AND NONINFRINGEMENT       |
 | OF THIRD PARTY RIGHTS. THE ENTIRE RISK AS TO THE QUALITY AND             |
 | PERFORMANCE OF THE PACKAGE OR ANY PART THEREOF IS WITH THE LICENSEE.     |
 | SHOULD ANY PART OF THE PACKAGE PROVE DEFECTIVE, YOU ASSUME THE           |
 | COST OF ALL NECESSARY SERVICING, REPAIR OR CORRECTION. THIS              |
 | DISCLAIMER OF WARRANTY CONSTITUTES AN ESSENTIAL PART OF THIS             |
 | LICENSE.                                                                 |
 | Terms used but not defined in this paragraph have the meanings           |
 | assigned thereto in the License Agreement referred to above.             |
 |                                                                          |
 |  Credits:  Andreas F. Pour <bugs@mieterra.com>                           |
 *--------------------------------------------------------------------------*/

#include "kshred.h"
#include <time.h>
#include <kdebug.h>


KShred::KShred(QString fileName)
{
  if (fileName.isEmpty())
  {
    kDebugError( "KShred: missing file name in constructor" );
    file = 0L;
  }
  else
  {
    file = new QFile();
    file->setName(fileName);
    if (!file->open(IO_ReadWrite))
    {
      kDebugError( "KShred: cannot open file '%s' for writing\n", fileName.local8Bit().data() );
      file = 0L;
    }
  }
}


KShred::~KShred()
{
  if (file != 0L)
    delete file;
}


bool
KShred::fill1s()
{
  return fillbyte(0xFF);
}


bool
KShred::fill0s()
{
  return fillbyte(0x0);
}


bool
KShred::fillbyte(uint byte)
{
  if (file == 0L)
    return false;
  char buff[4096];
  memset(buff, byte, 4096);

  uint n;
  for (uint todo = file->size(); todo > 0; todo -= n)
  {
    n = (todo > 4096 ? 4096 : todo);
    if (!writeData(buff, n))
      return false;
  }
  file->flush();
  return true;
}


bool
KShred::fillpattern(char *data, uint size)
{
  if (file == 0L)
    return false;

  uint n;
  for (uint todo = file->size(); todo > 0; todo -= n)
  {
    n = (todo > size ? size : todo);
    if (!writeData(data, n))
      return false;
  }
  file->flush();
  return true;
}


bool
KShred::fillrandom()
{
  if (file == 0L)
    return false;

  srandom((unsigned int) time(0L));
  long int buff[4096 / sizeof(long int)];
  uint n;

  for (uint todo = file->size(); todo > 0; todo -= n)
  {
    n = (todo > 4096 ? 4096 : todo);
    // assumes that 4096 is a multipe of sizeof(long int)
    int limit = (n + sizeof(long int) - 1) / sizeof(long int);
    for (int i = 0; i < limit; i++)
      buff[i] = random();

    if (!writeData((char *) buff, n))
      return false;
  }
  file->flush();
  return true;
}


bool
KShred::shred(QString fileName)
{

  kDebugError( "KShred: shredding '%s'\n", fileName.local8Bit().data() );
  if (fileName.isEmpty())
    return false;

  KShred shredder(fileName);
  return shredder.shred();
}


bool
KShred::writeData(char *data, uint size)
{
  // write 'data' of size 'size' to the file
  return (file->writeBlock(data, size) >= 0);
}


// shred the file, then close and remove it
bool
KShred::shred()
{
  if (!fill0s())
    return false;
  if (!fill1s())
    return false;
  if (!fillrandom())
    return false;
  if (!fillbyte((uint) 0x55))     // '0x55' is 01010101
    return false;
  if (!fillbyte((uint) 0xAA))     // '0xAA' is 10101010
    return false;
  if (!file->remove())
    return false;
  file = 0L;
  return true;
}
