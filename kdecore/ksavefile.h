/* 
   This file is part of the KDE libraries
   Copyright (c) 1999 Waldo Bastian <bastian@kde.org>
   
   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.
   
   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.
   
   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#ifndef _KSAVEFILE_H_
#define _KSAVEFILE_H_

#include <qstring.h>
#include <stdio.h>
#include <errno.h>
#include <ktempfile.h>

class KSaveFilePrivate;

/**
 * The KSaveFile class has been made to write out changes to an existing
 * file atomically.
 * This means that EITHER:
 * a)
 *   All changes have been written successfully to the file.
 *
 * b)
 *   Some error occured, no changes have been written whatsoever and the
 *   old file is still in place.
 */
class KSaveFile
{
public:
   /**
    * Creates a new KSaveFile with the given file name.
    * @param filename the path of the file
    * @param mode the mode of the file (see chmod(1))
    */
   KSaveFile(const QString &filename, int mode = 0666 );

   /**
    * The destructor closes the file.
    **/
   ~KSaveFile();

   /**
    * Returns the status of the file based on errno. (see errno.h)
    * 0 means OK.
    *
    * You should check the status after object creation to check
    * whether a file could be created in the first place.
    *
    * You may check the status after closing the file to verify that
    * the file has indeed been written correctly.
    * @return the errno status, 0 means ok
    **/
   int status() const
   	{ return mTempFile.status(); }

   /**
    * The name of the file as passed to the constructor.
    * @return The name of the file, or QString::null if opening the
    *         file has failed
    **/
   QString name() const;

   /**
    * An integer file descriptor open for writing to the file.
    * @return The file descriptor, or a negative number if opening
    *         the temporary file failed
    **/
   int handle()	const
   	{ return mTempFile.handle(); }

   /**
    * A FILE* stream open for writing to the file.
    * @return FILE* stream open for writing to the file, or 0
    *         if opening the temporary file failed
    **/
   FILE *fstream()
   	{ return mTempFile.fstream(); }

   /**
    * A QFile* open for writing to the file.
    * @return A QFile open for writing to the file, or 0 if 
    *         opening the temporary file failed.
    **/
   QFile *file()
   	{ return mTempFile.file(); }

   /**
    * A QTextStream* open for writing to the file.
    * @return A QTextStream that is open for writing to the file, or 0
    *         if opening the temporary file failed
    **/
   QTextStream *textStream()
   	{ return mTempFile.textStream(); }

   /**
    * A QDataStream* open for writing to the file.
    * @return A QDataStream that is open for writing to the file, or 0 
    *         if opening the file failed
    **/
   QDataStream *dataStream()
   	{ return mTempFile.dataStream(); }

   /**
    * Aborts the write operation and removes any intermediate files
    * This implies a close.
    **/
   void abort();

   /**
    * Closes the file and makes the changes definitive.
    * Returns 'true' is successful, or 'false' if an error has occured.
    * See status() for details about errors.
    * @return true if successful, or false if an error has occured.
    **/
   bool close();
private:
   QString mFileName;
   KTempFile mTempFile;

   KSaveFilePrivate *d;
};

#endif
