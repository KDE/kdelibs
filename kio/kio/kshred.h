/*--------------------------------------------------------------------------*
 | KShred.h  Copyright (c) 2000 MieTerra LLC. All rights reserved.          |
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
