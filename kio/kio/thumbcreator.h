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
/**
 * This is the baseclass for "thumbnail-plugins" in KDE. Using the class 
 * @ref KIO::PreviewJob allows you to generate small images (thumbnails)
 * for any kind of file, where a "ThumbCreator" is available. Have a look
 * at kdebase/kioslave/thumbnail/ for existing ThumbCreators.
 *
 * What you need to do to create and register a ThumbCreator:
 * @li Inherit from this class and reimplement the @ref create() method to
 *     generate a thumbnail for the given file-path.
 * @li Provide a factory method in your implementation file to instantiate
 * your plugin, e.g.:
 * <pre>
 * extern "C"
 * {
 *   ThumbCreator *new_creator()
 *   {
 *     return new YourThumbCreator();
 *   }
 * };
 * </pre>
 *
 * Compile your ThumbCreator as a module. The contents of Makefile.am
 * need to look like this:
 * <pre>
 * INCLUDES = $(all_includes)
 * LDFLAGS = $(all_libraries) $(KDE_RPATH)
 * kde_module_LTLIBRARIES = yourthumbcreator.la
 * yourthumbcreator_la_SOURCES = yourthumbcreator.cpp
 * yourthumbcreator_la_LIBADD = $(LIB_KIO)
 * yourthumbcreator_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
 * servicedir = $(kde_servicesdir)
 * service_DATA = yourthumbcreator.desktop
 * </pre>
 *
 * @li Create a file yourthumbcreator.desktop with the following contents:
 * <pre>
 * [Desktop Entry]
 * Encoding=UTF-8
 * Type=Service
 * Name=Name of the type of files your ThumbCreator supports
 * ServiceTypes=ThumbCreator
 * MimeTypes=application/x-somemimetype
 * CacheThumbnail=true
 * X-KDE-Library=yourthumbcreator
 * </pre>
 *
 * You can supply a comma-separated list of mimetypes to the MimeTypes entry,
 * naming all mimetypes your ThumbCreator supports. You can also use simple
 * wildcards, like 
 */
 //              text/* or image/*.
/*
 *
 * If your plugin is rather inexpensive (e.g. like the text preview ThumbCreator),
 * you can set CacheThumbnail=false to prevent your thumbnails from being cached
 * on disk.
 *
 * @short Baseclass for thumbnail-generating plugins.
 */
class ThumbCreator
{
public:
    enum Flags { None = 0, DrawFrame = 1, BlendIcon = 2 };
    virtual ~ThumbCreator() {}
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
