/*
    Large image displaying library.

    Copyright (C) 2004 Maks Orlovich (maksim@kde.org)

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
#ifndef SCALED_PLANE_H
#define SCALED_PLANE_H

#include "plane.h"

namespace khtmlImLoad {

struct ScaledPlane: public Plane
{
    struct TileAndOffset
    {
        unsigned int tile;
        unsigned int offset; 
    };
    
    TileAndOffset* calcScaleTable(unsigned int orig, unsigned int scaled)
    {
        //### I bet this has all sorts of imprecision problems w/high ratios
        TileAndOffset* origin = new TileAndOffset[scaled];
        
        double ratio = double(orig)/double(scaled);
        
        /**
        When scaling, we consider each pixel to be at the midpoint of its range. That is, if we have width 5, 
        the pixels are 0.5, 1.5, 2.5, 3.5, and 4.5 
        
        We walk the scaled list, and calculate where the pixel must have originated
        */
        for (unsigned int pix = 0; pix < scaled; pix++)
        {
            double id = pix + 0.5;
            
            unsigned int orig = (unsigned int)(id * ratio);
            
            origin[pix].tile   = orig / Tile::TileSize;
            origin[pix].offset = orig % Tile::TileSize; 
        }
        
        return origin;
    }
    
    TileAndOffset* xScaleTable;
    TileAndOffset* yScaleTable;
    
    ScaledPlane(unsigned int width    , unsigned int height, 
                unsigned int origWidth, unsigned int origHeight): Plane(width, height)
    {
        //Create the mapping tables
        yScaleTable = calcScaleTable(origHeight, height);
        xScaleTable = calcScaleTable(origWidth , width );
    }
    
    ~ScaledPlane()
    {
        delete[] xScaleTable;
        delete[] yScaleTable;
    }   
};

}

#endif
