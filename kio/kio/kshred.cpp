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
#include <time.h>
#include <klocale.h>
#include <kdebug.h>
#include <stdlib.h>
#include <kapplication.h>

KShred::KShred(QString fileName)
{
  if (fileName.isEmpty())
  {
    kdError() << "KShred: missing file name in constructor" << endl;
    file = 0L;
  }
  else
  {
    file = new QFile();
    file->setName(fileName);
    if (!file->open(IO_ReadWrite))
    {
      kdError() << "KShred: cannot open file '" << fileName.local8Bit().data() << "' for writing\n" << endl;
      file = 0L;
      fileSize = 0;
    }
    else
      fileSize = file->size();

    totalBytes    = 0;
    bytesWritten  = 0;
    lastSignalled = 0;
    tbpc          = 0;
    fspc          = 0;
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
KShred::fillbyte(unsigned int byte)
{
  if (file == 0L)
    return false;
  unsigned char buff[4096];
  memset((void *) buff, byte, 4096);

  unsigned int n;
  for (unsigned int todo = fileSize; todo > 0; todo -= n)
  {
    n = (todo > 4096 ? 4096 : todo);
    if (!writeData(buff, n))
      return false;
  }
  if (!flush())
    return false;
  return file->at(0);
}


bool
KShred::fillpattern(unsigned char *data, unsigned int size)
{
  if (file == 0L)
    return false;

  unsigned int n;
  for (unsigned int todo = fileSize; todo > 0; todo -= n)
  {
    n = (todo > size ? size : todo);
    if (!writeData(data, n))
      return false;
  }
  if (!flush())
    return false;
  return file->at(0);
}


bool
KShred::fillrandom()
{
  if (file == 0L)
    return false;

  long int buff[4096 / sizeof(long int)];
  unsigned int n;

  for (unsigned int todo = fileSize; todo > 0; todo -= n)
  {
    n = (todo > 4096 ? 4096 : todo);
    // assumes that 4096 is a multipe of sizeof(long int)
    int limit = (n + sizeof(long int) - 1) / sizeof(long int);
    for (int i = 0; i < limit; i++)
      buff[i] = kapp->random();

    if (!writeData((unsigned char *) buff, n))
      return false;
  }
  if (!flush())
    return false;
  return file->at(0);
}


bool
KShred::shred(QString fileName)
{
  if (fileName.isEmpty())
    return false;

  KShred shredder(fileName);
  return shredder.shred();
}


bool
KShred::writeData(unsigned char *data, unsigned int size)
{
  unsigned int ret                  = 0;

  // write 'data' of size 'size' to the file
  while ((ret < size) && (file->putch((int) data[ret]) >= 0))
    ret++;

  if ((totalBytes > 0) && (ret > 0))
  {
    if (tbpc == 0)
    {
      tbpc = ((unsigned int) (totalBytes / 100)) == 0 ? 1 : totalBytes / 100;
      fspc = ((unsigned int) (fileSize / 100)) == 0 ? 1 : fileSize / 100;
    }
    bytesWritten += ret;
    unsigned int pc = (unsigned int) (bytesWritten / tbpc);
    if (pc > lastSignalled)
    {
      emit processedSize(fspc * pc);
      lastSignalled = pc;
    }
  }
  return ret == size;
}


bool
KShred::flush()
{
  if (file == 0L)
    return false;

  file->flush();
  return (fsync(file->handle()) == 0);
}


// shred the file, then close and remove it
//
// UPDATED: this function now uses 35 passes based on the the article
// Peter Gutmann, "Secure Deletion of Data from Magnetic and Solid-State
// Memory", first published in the Sixth USENIX Security Symposium
// Proceedings, San Jose, CA, July 22-25, 1996 (available online at
// http://rootprompt.org/article.php3?article=473)

bool
KShred::shred()
{
  unsigned char p[6][3] = {{'\222', '\111', '\044'}, {'\111', '\044', '\222'},
                           {'\044', '\222', '\111'}, {'\155', '\266', '\333'},
                           {'\266', '\333', '\155'}, {'\333', '\155', '\266'}};
  QString msg = i18n("Shredding:  pass %1 of 35");

  emit processedSize(0);

  // thirty-five times writing the entire file size
  totalBytes = fileSize * 35;
  int iteration = 1;

  for (int ctr = 0; ctr < 4; ctr++)
    if (!fillrandom())
      return false;
    else
    {
      emit infoMessage(msg.arg(iteration));
    }

  if (!fillbyte((unsigned int) 0x55))     // '0x55' is 01010101
    return false;
  emit infoMessage(msg.arg(iteration));

  if (!fillbyte((unsigned int) 0xAA))     // '0xAA' is 10101010
    return false;
  emit infoMessage(msg.arg(iteration));

  for (unsigned int ctr = 0; ctr < 3; ctr++)
    if (!fillpattern(p[ctr], 3))  // '0x92', '0x49', '0x24'
      return false;
    else
    {
      emit infoMessage(msg.arg(iteration));
    }

  for (unsigned int ctr = 0; ctr <= 255 ; ctr += 17)
    if (!fillbyte(ctr))    // sequence of '0x00', '0x11', ..., '0xFF'
      return false;
    else
    {
      emit infoMessage(msg.arg(iteration));
    }

  for (unsigned int ctr = 0; ctr < 6; ctr++)
    if (!fillpattern(p[ctr], 3))  // '0x92', '0x49', '0x24'
      return false;
    else
    {
      emit infoMessage(msg.arg(iteration));
    }

  for (int ctr = 0; ctr < 4; ctr++)
    if (!fillrandom())
      return false;
    else
    {
      emit infoMessage(msg.arg(iteration));
    }

  if (!file->remove())
    return false;
  file = 0L;
  emit processedSize(fileSize);
  return true;
}

#include "kshred.moc"

