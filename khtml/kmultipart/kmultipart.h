/* This file is part of the KDE project
   Copyright (C) 2002 David Faure <david@mandrakesoft.com>

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

#ifndef __kmultipart_h__
#define __kmultipart_h__

#include <kparts/part.h>
#include <kparts/factory.h>
#include <kparts/browserextension.h>

class KHTMLPart;
class KInstance;
class KTempFile;
class KLineParser;

/**
 * http://www.netscape.com/assist/net_sites/pushpull.html
 */
class KMultiPart : public KParts::ReadOnlyPart
{
    Q_OBJECT
public:
    KMultiPart( QWidget *parentWidget, const char *widgetName,
                QObject *parent, const char *name, const QStringList& );
    virtual ~KMultiPart();

    virtual bool openFile() { return false; }
    virtual bool openURL( const KURL &url );

    virtual bool closeURL();

    static KAboutData* createAboutData();

protected:
    virtual void guiActivateEvent( KParts::GUIActivateEvent *e );
    void setPart( const QString& mimeType );

    void startOfData();
    void sendData( const QCString& line );
    void endOfData();

private slots:
    //void slotPopupMenu( KXMLGUIClient *cl, const QPoint &pos, const KURL &u, const QString &mime, mode_t mode );
    void slotJobFinished( KIO::Job *job );
    void slotData( KIO::Job *, const QByteArray & );
    //void updateWindowCaption();

    void slotPartCompleted();

private:
    KParts::BrowserExtension* m_extension;
    QGuardedPtr<KParts::ReadOnlyPart> m_part;
    bool m_isHTMLPart;
    KIO::Job* m_job;
    QCString m_boundary;
    int m_boundaryLength;
    QString m_mimeType; // the one handled by m_part - store the kservice instead?
    QString m_nextMimeType; // while parsing headers
    KTempFile* m_tempFile;
    KLineParser* m_lineParser;
    bool m_bParsingHeader;
    bool m_bGotAnyHeader;
};

#if 0
class KMultiPartBrowserExtension : public KParts::BrowserExtension
{
    //Q_OBJECT
public:
    KMultiPartBrowserExtension( KMultiPart *parent, const char *name = 0 );

    virtual int xOffset();
    virtual int yOffset();

//protected slots:
    void print();
    void reparseConfiguration();

private:
    KMultiPart *m_imgPart;
};
#endif

#endif
