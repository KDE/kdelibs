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

#include "khtmlsavedpage.h"

SavedPage::SavedPage()
{
    isFrame = false;
    isFrameSet = false;
    xOffset = yOffset = 0;
    frameLayout = 0;
    frames = 0;
    forms = 0;
}

SavedPage::~SavedPage()
{
    if( frameLayout ) delete frameLayout;
    if( frames ) delete frames;
    if( forms ) delete forms;
}

