/* This file is part of the KDE project
 *
 * Copyright (C) 2002 David Faure <david@mandrakesoft.com>
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License version 2, as published by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#ifndef kparts_browserrun_h
#define kparts_browserrun_h

#include <krun.h>
#include <kservice.h>
#include <kparts/browserextension.h>

namespace KParts {

    /**
     * This class extends KRun to provide additional functionality for browsers:
     * - "save or open" dialog boxes
     * - "save" functionality
     * - support for HTTP POST (including saving the result to a temp file if
     *   opening a separate application)
     * - warning before launching executables off the web
     * - custom error handling (i.e. treating errors as HTML pages)
     * - generation of SSL metadata depending on the previous URL shown by the part
     * @author David Faure <david@mandrakesoft.com>
     */
    class BrowserRun : public KRun
    {
        Q_OBJECT
    public:
        /**
         * @param url the URL we're probing
         * @param args URL args - includes data for a HTTP POST, etc.
         * @param part the part going to open this URL - can be 0L if not created yet
         * @param window the mainwindow - passed to KIO::Job::setWindow()
         * @param removeReferrer if true, the "referrer" metadata from @p args isn't passed on
         * @param trustedSource if false, a warning will be shown before launching an executable
	 * Always pass false for @p trustedSource, except for local directory views.
         */
        BrowserRun( const KURL& url, const KParts::URLArgs& args,
                    KParts::ReadOnlyPart *part, QWidget *window,
                    bool removeReferrer, bool trustedSource );
        virtual ~BrowserRun() {}

        //KParts::URLArgs urlArgs() const { return m_args; }
        //KParts::ReadOnlyPart* part() const { return m_part; }
        KURL url() const { return m_strURL; }

        enum AskSaveResult { Save, Open, Cancel };
        static AskSaveResult askSave( const KURL & url, KService::Ptr offer, const QString& mimeType, const QString & suggestedFilename = QString::null );

        // virtual so that KHTML can implement differently (HTML cache)
        virtual void save( const KURL & url, const QString & suggestedFilename );
        // static so that it can be called from other classes
        static void simpleSave( const KURL & url, const QString & suggestedFilename );

        static bool allowExecution( const QString &serviceType, const KURL &url );

        static bool isExecutable( const QString &serviceType );
        static bool isTextExecutable( const QString &serviceType );

    protected:
        virtual void scanFile();
        virtual void handleError( KIO::Job * job );

        // NotHandled means that foundMimeType should call KRun::foundMimeType,
        // i.e. launch an external app.
        enum NonEmbeddableResult { Handled, NotHandled, Delayed };
        /**
	 * Helper for foundMimeType: call this if the mimetype couldn't be embedded
	 */
        NonEmbeddableResult handleNonEmbeddable( const QString& mimeType );

    protected slots:
        void slotBrowserScanFinished(KIO::Job *job);
        void slotBrowserMimetype(KIO::Job *job, const QString &type);
        void slotCopyToTempFileResult(KIO::Job *job);
        virtual void slotStatResult( KIO::Job *job );

    protected:
        KParts::URLArgs m_args;
        KParts::ReadOnlyPart *m_part; // QGuardedPtr?
        QWidget *m_window;            // QGuardedPtr?
        QString m_suggestedFilename;
        QString m_sMimeType;
        bool m_bRemoveReferrer;
        bool m_bTrustedSource;

    };
}
#endif
