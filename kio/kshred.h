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
#include <qstring.h>
#include <qfile.h>
#include <qobject.h>

/**
 * Erase a file in a way that makes recovery impossible.
 * For this, KShred write several times over the
 * existing file, using different patterns, before deleting it.
 * @author Andreas F. Pour <bugs@mieterra.com>
 * Integrated into KDE by David Faure <faure@kde.org>
 */
class KShred : public QObject {

  Q_OBJECT

    public:

        KShred(QString fileName);

        ~KShred();

        bool fill1s();
        bool fill0s();
        bool fillbyte(uint byte);
        bool fillrandom();
        bool fillpattern(char *pattern, uint size);
        // Does all of the above
        bool shred();

        /**
         * The easiest way to shred a file
         * No need to create an instance of the class
         */
        static bool shred(QString fileName);

    signals:
        /**
         * Some progress info
         */
        void processedSize( unsigned long bytes );

    private:
        bool writeData(char *data, uint size);
        QFile *file;
};

#endif
