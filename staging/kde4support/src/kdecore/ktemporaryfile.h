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

#include <kde4support_export.h>
#include "kglobal.h"

#include <QtCore/QTemporaryFile>

class KTemporaryFilePrivate;

/**
 * \class KTemporaryFile ktemporaryfile.h <KTemporaryFile>
 *
 * @deprecated use QTemporaryFile
 *
 * By default the filename will start with your application's name,
 * followed by six random characters. You can call QTemporaryFile::setFileTemplate()
 * to change that.
 *
 * Porting to QTemporaryFile is simple: in apps, you can probably juse use the default constructor.
 *
 * In parts and plugins, you were probably passing a component data to KTemporaryFile, so instead use:
 * QTemporaryFile(QDir::tempPath() + QLatin1Char('/') + componentData.name() + QLatin1String("XXXXXX"))
 *
 * For setPrefix, change the QDir::tempPath() from the above line.
 * For setSuffix, append it after the XXXXXX.
 *
 * In the simplest case where the application was only calling setSuffix(".txt"), this becomes
 * QTemporaryFile(QDir::tempPath() + QLatin1String("/myapp_XXXXXX.txt"))
 *
 * @see QTemporaryFile
 */
class KDE4SUPPORT_DEPRECATED_EXPORT KTemporaryFile : public QTemporaryFile
{
public:
    /**
     * Construct a new KTemporaryFile. The file will be stored in the temporary
     * directory configured in KDE. The default prefix is the value of the
     * default KDE temporary directory, plus your application's instance name.
     * The default suffix is ".tmp".
     *
     * \param componentData The KComponentData to use for the name of the file and
     * to look up the directory.
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
