/* This file is part of the KDE libraries
    Copyright (C) 2000 Carsten Pfeiffer <pfeiffer@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/
#ifndef KFILE_H
#define KFILE_H

#include <QtCore/QDir>

#include <kio/kiowidgets_export.h>

/**
 * KFile is a class which provides a namespace for some enumerated
 * values associated with the kfile library.  You will never need to
 * construct a KFile object itself.
 */

class KIOWIDGETS_EXPORT KFile
{
    Q_GADGET
    Q_FLAGS(Modes)
public:
    /**
     * Modes of operation for the dialog.
     * @li @p File - Get a single file name from the user.
     * @li @p Directory - Get a directory name from the user.
     * @li @p Files - Get multiple file names from the user.
     * @li @p ExistingOnly - Never return a filename which does not exist yet
     * @li @p LocalOnly - Don't return remote filenames
     */
    enum Mode {
        File         = 1,
        Directory    = 2,
        Files        = 4,
        ExistingOnly = 8,
        LocalOnly    = 16,
        ModeMax      = 65536
    };
    Q_DECLARE_FLAGS(Modes, Mode)

    enum FileView {
        Default         = 0,
        Simple          = 1,
        Detail          = 2,
        SeparateDirs    = 4,
        PreviewContents = 8,
        PreviewInfo     = 16,
        Tree            = 32,
        DetailTree      = 64,
        FileViewMax     = 65536
    };

    enum SelectionMode {
        Single      = 1,
        Multi       = 2,
        Extended    = 4,
        NoSelection = 8
    };


    //
    // some bittests
    //

    // sorting specific

    static bool isSortByName( const QDir::SortFlags& sort );

    static bool isSortBySize( const QDir::SortFlags& sort );

    static bool isSortByDate( const QDir::SortFlags& sort ); 

    static bool isSortByType( const QDir::SortFlags& sort );

    static bool isSortDirsFirst( const QDir::SortFlags& sort );

    static bool isSortCaseInsensitive( const QDir::SortFlags& sort );


    // view specific
    static bool isDefaultView( const FileView& view );

    static bool isSimpleView( const FileView& view );

    static bool isDetailView( const FileView& view ); 

    static bool isSeparateDirs( const FileView& view );

    static bool isPreviewContents( const FileView& view ); 

    static bool isPreviewInfo( const FileView& view );
    
    static bool isTreeView( const FileView& view );
    
    static bool isDetailTreeView( const FileView& view );

private:
    KFile(); // forbidden
};

Q_DECLARE_OPERATORS_FOR_FLAGS(KFile::Modes)

#endif // KFILE_H
