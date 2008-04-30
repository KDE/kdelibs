/* kate: tab-indents off; replace-tabs on; tab-width 4; remove-trailing-space on; encoding utf-8;*/
/* This file is part of the KDE libraries
 *  Copyright 2006 Jaison Lee <lee.jaison@gmail.com>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 */

#ifndef ktemporaryfile_h
#define ktemporaryfile_h

#include <kdecore_export.h>
#include <kglobal.h>

#include <QtCore/QTemporaryFile>

class KTemporaryFilePrivate;

/**
 * \class KTemporaryFile ktemporaryfile.h <KTemporaryFile>
 *
 * @brief A QTemporaryFile that will save in the KDE temp directory.
 *
 * This class derives from QTemporaryFile and makes sure that your temporary
 * files go in the temporary directory defined by KDE. (This is retrieved by
 * using KStandardDirs to locate the "tmp" resource.) In general, whenever you
 * would use a QTemporaryFile() use a KTemporaryFile() instead.
 *
 * By default the filename will start with your application's instance name,
 * followed by six random characters and an extension of ".tmp". You can use
 * setPrefix() and setSuffix() to change the beginning and ending of the random
 * name, as well as change the directory if you wish (read the descriptions of
 * these functions for more information). For complex specifications, you may
 * be better off calling QTemporaryFile::setFileTemplate() directly.
 *
 * For example, let's make a new temporary file:
 *
 * @code
 * KTemporaryFile temp;
 * @endcode
 *
 * This temporary file will currently be stored in the default KDE temporary
 * directory and have an extension of ".tmp". Now, let's change the directory:
 *
 * @code
 * temp.setPrefix("/var/lib/foodata/");
 * @endcode
 *
 * Now the temporary file will be stored in "/var/lib/foodata" instead of the
 * default KDE temporary directory, with an extension of ".tmp". It's important
 * to remember the leading and trailing slashes to properly define the path!
 * Next, let's change the suffix to a particular extension:
 *
 * @code
 * temp.setSuffix(".pdf");
 * @endcode
 *
 * Now the temporary file will be stored in "/var/lib/foodata" and have an
 * extension of ".pdf" instead of ".tmp".
 *
 * Once you are done determining the name of the file, call open() to
 * create the file.
 *
 * @code
 * if ( !temp.open() ) {
 *     // handle error...
 * }
 * @endcode
 *
 * If open() is unable to create the file it will return false. If the call to
 * open() returns true you are ready to use your temporary file. If you don't
 * want the file removed automatically when the KTemporaryFile object is
 * destroyed, you need to call setAutoRemove(false), but make sure you have a
 * good reason for leaving your temp files around.
 *
 * @see QTemporaryFile
 *
 * @author Jaison Lee <lee.jaison@gmail.com>
 */
class KDECORE_EXPORT KTemporaryFile : public QTemporaryFile
{
public:
    /**
     * Construct a new KTemporaryFile. The file will be stored in the temporary
     * directory configured in KDE. The default prefix is the value of the
     * default KDE temporary directory, plus your application's instance name.
     * The default suffix is ".tmp".
     *
     * \param componentData The KComponentData to use for the name of the file and to look up the
     * directory.
     */
    explicit KTemporaryFile(const KComponentData &componentData = KGlobal::mainComponent());

    /**
     * Destructor.
     */
    virtual ~KTemporaryFile();

    /**
     * @brief Sets a prefix to use when creating the file.
     *
     * This function sets a prefix to use when creating the file. The random
     * part of the filename will come after this prefix. The prefix can also
     * change or modify the target directory. If @p prefix is an absolute
     * path it will override the default temporary directory. If @p prefix is
     * a relative directory it will be relative to the default temporary
     * location. To set a relative directory for the current working directory
     * you should use QTemporaryFile::setFileTemplate() directly.
     * @param prefix The prefix to use when creating the file. Remember to
     *  end the prefix with a '/' if you are designating a directory.
     */
    void setPrefix(const QString &prefix);

    /**
     * @brief Sets a suffix to use when creating the file.
     *
     * Sets a suffix to use when creating the file. The random part of the
     * filename will come before this suffix.
     * @param suffix The suffix to use when creating the file.
     */
    void setSuffix(const QString &suffix);

private:
    KTemporaryFilePrivate *const d;
};

#endif
