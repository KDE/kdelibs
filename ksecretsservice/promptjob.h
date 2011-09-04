/* This file is part of the KDE project
 *
 * Copyright (C) 2011 Valentin Rusu <kde@rusu.info>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef PROMPTJOB_H
#define PROMPTJOB_H

#include <kjob.h>
#include <QDBusObjectPath>
#include <qwindowdefs.h>

class OrgFreedesktopSecretPromptInterface;

class PromptJob : public KJob {
    Q_OBJECT
    Q_DISABLE_COPY(PromptJob)
public:
    PromptJob( const QDBusObjectPath &path, const WId &parentWindowId, QObject *parent );
    
    virtual void start();

    bool isDismissed() const { return dismissed; }
    const QDBusVariant & result() const { return opResult; }
    
private Q_SLOTS:
    void promptCompleted(bool dismissed, const QDBusVariant &result);
    
private:
    QDBusObjectPath promptPath;
    WId             parentWindowId;
    OrgFreedesktopSecretPromptInterface *promptIf;
    bool            dismissed;
    QDBusVariant    opResult;
};


#endif // PROMPTJOB_H

