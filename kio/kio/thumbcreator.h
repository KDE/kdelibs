/*  This file is part of the KDE libraries
    Copyright (C) 2000 Malte Starostik <malte@kde.org>

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

#ifndef _THUMBCREATOR_H_
#define _THUMBCREATOR_H_ "$Id$"

class QString;
class QImage;

class ThumbCreator
{
public:
    enum Flags { None = 0, DrawFrame = 1, BlendIcon = 2 };
    /**
     * Creates a thumbnail
     * Note that the width and height parameters should not be used
     * for scaling. Only plugins that create an image "from scratch",
     * like the TextCreator should directly use the specified size.
     * If the resulting preview is larger than width x height, it will be
     * scaled down.
     *
     * @param path the (always local) file path to create a preview for
     * @param width maximum width for the preview
     * @param height maximum height for the preview
     * @param img this image will contain the preview on success
     * 
     * @return true if preview generation succeeded
     */
    virtual bool create(const QString &path, int width, int height, QImage &img) = 0;
    /**
     * The flags of this plugin:
     * @li None nothing special
     * @li DrawFrame a frame should be painted around the preview
     * @li BlendIcon the mimetype icon should be blended over the preview
     *
     * @return flags for this plugin
     */
    virtual Flags flags() const { return None; }
};

typedef ThumbCreator *(*newCreator)();

#endif
