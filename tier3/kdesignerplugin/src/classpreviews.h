/* This file is part of the KDE libraries
    Copyright (C) 2001 Daniel Molkentin <molkentin@kde.org> 

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

/*

   This class is intended as a place to widgets previews for designer. Simply inherit
   your widget and modify its behavior (i.e. enter example data or do a reparent()).

   This will only affect the preview, designer/uic will still use the original widget
   in the final source output!

 */

#ifndef CLASSPREVIEWS_H
#define CLASSPREVIEWS_H

#include <QDialog>

class KDialogPreview : public QDialog
{
public:
	KDialogPreview(QWidget * parent = 0);
};

#endif // CLASSPREVIEWS_H
