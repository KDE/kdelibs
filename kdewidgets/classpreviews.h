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
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

/* 

	This class is indented as a place to widgets previews for designer. Simply inherit
   your widget and modify its behaviour (i.e. enter example datas or do a reparent()).

   This will only affect the preview, designer/uic will still use the original widget you
   in the final source output!

 */

#ifndef __classimpls__
#define __classimpls__

#include <kdialog.h>
#include <kdialogbase.h>

class KDialogPreview : public KDialog
{
public:
	KDialogPreview(QWidget * parent = 0 , const char * name = 0/*, bool modal = FALSE, WFlags f = 0*/);
};

#endif
