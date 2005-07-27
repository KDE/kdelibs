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
#ifndef SCALED_IMAGE_PLANE_H
#define SCALED_IMAGE_PLANE_H

#include "array2d.h"
#include "imageplane.h"
#include "rawimageplane.h"
#include "imagetile.h"

namespace khtmlImLoad {

/**
 A scaled image plane pulls data from a RawImagePlane and resizes it
*/
class ScaledImagePlane: public ImagePlane
{
private:
    RawImagePlane*     parent;
    Array2D<ImageTile> tiles;

    
    unsigned int* calcScaleTable(unsigned int orig, unsigned int scaled)
    {
        //### I bet this has all sorts of imprecision problems w/high ratios
        unsigned int* origin = new unsigned int[scaled];
        
        //### FIXME: replace with something that clamps on right edge later?
        double ratio    = double(orig)/double(scaled);
        int    intRatio = int(ratio*65536.0 + 1);
        int    pos      = 0;
        
        for (unsigned int pix = 0; pix < scaled; pix++)
        {
            origin[pix]  =  pos >> 16;
            pos          += intRatio;
        }
        
        return origin;
    }
    
    unsigned int* xScaleTable;
    unsigned int* yScaleTable;
public:
    virtual ~ScaledImagePlane();
    

    ScaledImagePlane(unsigned int _width, unsigned int _height, RawImagePlane* _parent):
            ImagePlane(_width, _height), parent(_parent), tiles(tilesWidth, tilesHeight)
    {
        //Create the mapping tables
        yScaleTable = calcScaleTable(parent->height, height);
        xScaleTable = calcScaleTable(parent->width , width );
    }


    virtual bool isUpToDate(unsigned int tileX, unsigned int tileY,
                            PixmapTile* tile);

    virtual void ensureUpToDate(unsigned int tileX, unsigned int tileY,
                            PixmapTile* tile);
};

}

#endif
// kate: indent-width 4; replace-tabs on; tab-width 4; space-indent on;
