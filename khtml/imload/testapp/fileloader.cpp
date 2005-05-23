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

#include "fileloader.h"
#include "image.h"

#include <qstring.h>
#include <qtimer.h>


FileLoader::FileLoader(Image* _image, QString fileName):file(fileName)
{
    image = _image;
    
    file.open(IO_ReadOnly);
    
    QTimer* timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()),
            this,  SLOT  (feedData()));
    timer->start(100);            
}

const int BLOCK = 4096;

void FileLoader::feedData()
{
    char buf[BLOCK];
    
    int size = file.readBlock(buf, BLOCK);
    if (size > 0)
        image->processData((uchar*)buf, size);
    //### TODO:Error handle
}

#include "fileloader.moc"
// kate: indent-width 4; replace-tabs on; tab-width 4; space-indent on;
