/****************************************************************************
** $Id$
**
** Definition of QPolygonScanner class
**
** Created : 000120
**
** Copyright (C) 1999-2000 Troll Tech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Troll Tech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees holding valid Qt Professional Edition licenses may use this
** file in accordance with the Qt Professional Edition License Agreement
** provided with the Qt Professional Edition.
**
** See http://www.troll.no/pricing.html or email sales@troll.no for
** information about the Professional Edition licensing, or see
** http://www.troll.no/qpl/ for QPL licensing information.
**
*****************************************************************************/

#ifndef QPOLYGONSCANNER_H
#define QPOLYGONSCANNER_H

#ifndef QT_H
#include <qglobal.h>
#endif // QT_H

class QPointArray;
class QPoint;

class Q_EXPORT QPolygonScanner {
public:
    void scan( const QPointArray& pa, bool winding, int index=0, int npoints=-1 );
    virtual void processSpans( int n, QPoint* point, int* width )=0;
};

#endif // QPOLYGONSCANNER_H
