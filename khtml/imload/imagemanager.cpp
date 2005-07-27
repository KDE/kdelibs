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


#include "imagemanager.h"
#include "decoders/jpegloader.h"
#include "decoders/pngloader.h"
#include "decoders/gifloader.h"

namespace khtmlImLoad {

TileCache*      ImageManager::imgCache   = 0;
TileCache*      ImageManager::pixCache   = 0;
Updater*        ImageManager::theUpdater = 0;
LoaderDatabase* ImageManager::loaderDB   = 0;
QPixmap*        ImageManager::emptyPix   = 0;
AnimTimer*      ImageManager::anmTimer   = 0;

//Each tile is 64x64 pixels, so normally
//64x64x4 = 16K, so 1 megabyte = 64 tiles, roughly.
//I'll probably switch to more precize accounting eventually

//### any way of detecting memory size?

unsigned int ImageManager::imageCacheSize()
{
    return 64*32;
}

unsigned int ImageManager::pixmapCacheSize()
{
    return 64*64;
}

void ImageManager::initLoaders()
{
    loaderDB->registerLoaderProvider(new JPEGLoaderProvider);
    loaderDB->registerLoaderProvider(new PNGLoaderProvider);
    loaderDB->registerLoaderProvider(new GIFLoaderProvider);
}

}

// kate: indent-width 4; replace-tabs on; tab-width 4; space-indent on;
