/* 
    This file is part of the KDE libraries

    Copyright (C) 1998 Lars Knoll <knoll@mpi-hd.mpg.de>

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/
//----------------------------------------------------------------------------
//
// SavedPage --- class for saving and restoring HTML pages
// $Id$

#ifndef KHTMLPAGE_H
#define KHTMLPAGE_H

#include <qstrlist.h>
#include <qstring.h>
#include <qlist.h>

struct FrameLayout
{
    QString rows;
    QString cols;
    int frameBorder;
    bool allowResize;
};

struct SavedPage
{
    SavedPage();
    ~SavedPage();
    QString frameName;
    bool isFrame;
    int scrolling;
    int frameborder;
    int marginwidth;
    int marginheight;
    bool allowresize;
    bool isFrameSet;
    QString url;
    QString title;
    int xOffset;
    int yOffset;
    QStrList *forms;
    FrameLayout *frameLayout;
    QList<SavedPage> *frames;
};

#endif

