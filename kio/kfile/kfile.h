/* This file is part of the KDE libraries
    Copyright (C) 2000 Carsten Pfeiffer <pfeiffer@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    version 2, License as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/
#ifndef KFILE_H
#define KFILE_H

#include <qdir.h>

/**
 * KFile is a class which provides a namespace for some enumerated
 * values associated with the kfile library.  You will never need to
 * construct a KFile object itself.
 */

class KFile
{
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
	LocalOnly    = 16
    };

    enum FileView {
	Default         = 0,
	Simple          = 1,
	Detail          = 2,
	SeparateDirs    = 4,
	PreviewContents = 8,
	PreviewInfo     = 16 // not implemented yet!
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

    // grr, who had the idea to set QDir::Name to 0x0?
    static bool isSortByName( const QDir::SortSpec& sort ) {
	return (sort & QDir::Time) != QDir::Time &&
	       (sort & QDir::Size) != QDir::Size;
    }

    static bool isSortBySize( const QDir::SortSpec& sort ) {
	return (sort & QDir::Size) == QDir::Size;
    }

    static bool isSortByDate( const QDir::SortSpec& sort ) {
	return (sort & QDir::Time) == QDir::Time;
    }

    static bool isSortDirsFirst( const QDir::SortSpec& sort ) {
	return (sort & QDir::DirsFirst) == QDir::DirsFirst;
    }

    static bool isSortCaseInsensitive( const QDir::SortSpec& sort ) {
	return (sort & QDir::IgnoreCase) == QDir::IgnoreCase;
    }


    // view specific
    static bool isDefaultView( const FileView& view ) {
	return (view & Default) == Default;
    }

    static bool isSimpleView( const FileView& view ) {
	return (view & Simple) == Simple;
    }

    static bool isDetailView( const FileView& view ) {
	return (view & Detail) == Detail;
    }

    static bool isSeparateDirs( const FileView& view ) {
	return (view & SeparateDirs) == SeparateDirs;
    }

    static bool isPreviewContents( const FileView& view ) {
	return (view & PreviewContents) == PreviewContents;
    }

    static bool isPreviewInfo( const FileView& view ) {
        return (view & PreviewInfo) == PreviewContents;
    }

};

#endif // KFILE_H
