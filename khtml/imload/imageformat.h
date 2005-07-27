/*
    Large image displaying library.

    Copyright (C) 2004,2005 Maks Orlovich (maksim@kde.org)

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in
    all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
    AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
    AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
    CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

*/

#ifndef IMAGE_FORMAT_H
#define IMAGE_FORMAT_H

#include <QColor>
#include <QVector>
#include <QImage>

namespace khtmlImLoad {

struct ImageFormat
{
    enum Type
    {
        Image_RGB_32,     //32-bit RGB + padding
        Image_RGBA_32,    //32-bit RGB + alpha
                          //note that this is interpreted as normal by
                          //the loader interface, as premultiplied
                          //by the drawing code
        Image_Palette_8   //8-bit paletted image        
    } type;
    
    int depth() const
    {
        switch (type)
        {
        case Image_RGB_32:
        case Image_RGBA_32:
            return 4;
        default:
            return 1;    
        }
    }

    QImage makeImage(int width, int height) const
    {
        QImage toRet;
        switch (type)
        {
        case Image_RGB_32:
            toRet = QImage(width, height, QImage::Format_RGB32);
            break;
        case Image_RGBA_32:
            toRet = QImage(width, height, QImage::Format_ARGB32_Premultiplied);
            break;
        case Image_Palette_8:
            toRet = QImage(width, height, QImage::Format_Indexed8);
            toRet.setColorTable(palette);
        }

        return toRet;
    }
    
    bool hasAlpha() const
    {
        return  (type == Image_RGBA_32);
    }
    
    QVector<QRgb> palette;

    //A helper for setting up a format descriptor for 8-bit grayscale
    void greyscaleSetup()
    {
        palette.clear();
        for (int i=0; i<256; i++)
            palette.append(qRgb(i,i,i));
        type = ImageFormat::Image_Palette_8;    
    }
};

}

#endif
// kate: indent-width 4; replace-tabs on; tab-width 4; space-indent on;
