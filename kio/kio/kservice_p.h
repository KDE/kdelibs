/* This file is part of the KDE project
   Copyright (C) 2003 Waldo Bastian <bastian@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef __kservices_p_h__
#define __kservices_p_h__

#include <qtimer.h>

#include <kprogress.h>

class KServiceProgressDialog : public KProgressDialog
{
   Q_OBJECT
public:
   KServiceProgressDialog(QWidget *parent, const char *name, 
                          const QString &caption, const QString &text);
public slots:   
   void slotProgress();
   void slotFinished();

private:
   QTimer m_timer;
   int m_timeStep;
};

#endif
