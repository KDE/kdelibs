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

#ifndef kshred_h
#define kshred_h


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <qstring.h>
#include <qfile.h>
#include <qobject.h>

#include <kio/global.h>

/**
 * Erase a file in a way that makes recovery impossible -- well, no guarentee
 * of that, but at least as difficult as reasonably possible.
 * For this, KShred write several times over the
 * existing file, using different patterns, before deleting it.
 * @author Andreas F. Pour <bugs@mieterra.com>
 * @author David Faure <faure@kde.org> (integration into KDE and progress signal)
 */
class KShred : public QObject {

  Q_OBJECT

    public:

	/**
	 * Initialize the class using the name of the file to 'shred'.
	 * @param fileName fully qualified name of the file to shred.
	 */
        KShred(QString fileName);

	/*
	 * Destructor for the class.
	 */
        ~KShred();

	/**
	 * Writes all 1's over the entire file and flushes the file buffers.
	 * @return true on success, false on error (invalid filename or write error)
	 */

        bool fill1s();
	/**
	 * Writes all 0's over the entire file and flushes the file buffers.
	 * @return true on success, false on error (invalid filename or write error)
	 */
        bool fill0s();

	/**
	 * Writes the specified byte over the entire file and flushes the file buffers.
	 * @param byte the value to write over every byte of the file
	 * @return true on success, false on error (invalid filename or write error)
	 */
        bool fillbyte(unsigned int byte);

	/**
	 * Writes random bites over the entire file and flushes the file buffers.
	 * @return true on success, false on error (invalid filename or write error)
	 */
        bool fillrandom();

	/**
	 * Writes the specified byte array over the entire file and flushes the file buffers.
	 * @param pattern the value to write over the entire file
	 * @param size the length of the 'pattern' byte array
	 * @return true on success, false on error (invalid filename or write error)
	 */
        bool fillpattern(unsigned char *pattern, unsigned int size);

	/**
	 * Shreds a file by writing a series of values over it (uses @ref
	 * #fill0s, then @ref #fill1s, then @ref #fillrandom, then
         * @ref #fillbyte with 0101..., then @ref #fillbyte with 1010....
	 * @return true on success, false on error (invalid filename or write error)
         */
        bool shred();

        /**
         * The simplest method to shred a file.
         * No need to create an instance of the class.
	 * @param fileName fully qualified name of the file to shred.
         */
        static bool shred(QString fileName);

    signals:
        /**
         * Shows progress of the shredding.
	 * @param bytes the number of bytes written to the file
         */
        void processedSize(KIO::filesize_t bytes);

        /**
         * Shows a message in the progress dialog
	 * @param the message to display
         */
        void infoMessage(const QString &);

    private:
	/**
	 * @internal write the data to the file
	 */
        bool writeData(unsigned char *data, unsigned int size);

	/**
	 * @internal flush the data to the file
	 */
        bool flush();

	/**
	 * @internal structure for the file information
	 */
        QFile *file;

	/**
	 * @internal for the size of the file
	 */
        KIO::filesize_t fileSize;

	/**
	 * @internal for keeping track of progress
	 */
        unsigned int totalBytes;
        unsigned int bytesWritten;
        unsigned int lastSignalled;
        unsigned int tbpc;
        unsigned int fspc;
    private:
	class KShredPrivate* d;
};

#endif
