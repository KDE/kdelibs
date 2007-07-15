/*
   Copyright (c) 2000 Bernd Johannes Wuebben <wuebben@math.cornell.edu>
   Copyright (c) 2000 Stephan Kulow <coolo@kde.org>

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef KSENDBUGMAIL_MAIN_H
#define KSENDBUGMAIL_MAIN_H

#include <QtCore/QObject>

class SMTP;

class BugMailer : public QObject {
    Q_OBJECT
public:
    BugMailer(SMTP* s) : QObject(0), sm(s) { setObjectName( "mailer" ); }

public Q_SLOTS:
    void slotError(int);
    void slotSend();
private:
    SMTP *sm;
};

#endif
