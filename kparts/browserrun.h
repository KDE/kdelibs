/* This file is part of the KDE project
 *
 * Copyright (C) 2002 David Faure <faure@kde.org>
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
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
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
     * @author David Faure <faure@kde.org>
     */
    class KPARTS_EXPORT BrowserRun : public KRun
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

        // BIC: merge with above constructor
        /**
         * @param url the URL we're probing
         * @param args URL args - includes data for a HTTP POST, etc.
         * @param part the part going to open this URL - can be 0L if not created yet
         * @param window the mainwindow - passed to KIO::Job::setWindow()
         * @param removeReferrer if true, the "referrer" metadata from @p args isn't passed on
         * @param trustedSource if false, a warning will be shown before launching an executable
         * @param hideErrorDialog if true, no dialog will be shown in case of errors.
         * Always pass false for @p trustedSource, except for local directory views.
         */
        BrowserRun( const KURL& url, const KParts::URLArgs& args,
                    KParts::ReadOnlyPart *part, QWidget *window,
                    bool removeReferrer, bool trustedSource, bool hideErrorDialog );

        virtual ~BrowserRun();

        //KParts::URLArgs urlArgs() const { return m_args; }
        //KParts::ReadOnlyPart* part() const { return m_part; }
        KURL url() const { return m_strURL; }

        bool hideErrorDialog() const;
        
        // Suggested filename given by the server (e.g. HTTP content-disposition)
        QString suggestedFilename() const { return m_suggestedFilename; }
        
        enum AskSaveResult { Save, Open, Cancel };
        /**
         * Ask the user whether to save or open a url in another application.
         * @param url the URL in question
         * @param offer the application that will be used to open the URL
         * @param mimeType the mimetype of the URL
         * @param suggestedFilename optional filename suggested by the server
         * @return Save, Open or Cancel.
         */
        static AskSaveResult askSave( const KURL & url, KService::Ptr offer, const QString& mimeType, const QString & suggestedFilename = QString() );
        /**
         * Similar to askSave but for the case where the current application is
         * able to embed the url itself (instead of passing it to another app).
         * @param url the URL in question
         * @param mimeType the mimetype of the URL
         * @param suggestedFilename optional filename suggested by the server
         * @param flags reserved for later use
         * @return Save, Open or Cancel.
         */
        static AskSaveResult askEmbedOrSave( const KURL & url, const QString& mimeType, const QString & suggestedFilename = QString(), int flags = 0 );

        // virtual so that KHTML can implement differently (HTML cache)
        virtual void save( const KURL & url, const QString & suggestedFilename );

        // static so that it can be called from other classes
        static void simpleSave( const KURL & url, const QString & suggestedFilename,
                                QWidget* window =0 );


        static bool allowExecution( const QString &serviceType, const KURL &url );

        /** BIC: Obsoleted by KRun::isExecutable( const QString &serviceType ); */
        static bool isExecutable( const QString &serviceType );
        static bool isTextExecutable( const QString &serviceType );

    protected:
        /**
         * Reimplemented from KRun
         */
        virtual void scanFile();
        /**
         * Reimplemented from KRun
         */
        virtual void init();
        /**
         * Called when an error happens.
         * NOTE: @p job could be 0L, if you passed hideErrorDialog=true.
         * The default implementation shows a message box, but only when job != 0 ....
         * It is strongly recommended to reimplement this method if
         * you passed hideErrorDialog=true.
         */
        virtual void handleError( KIO::Job * job );

        /**
         * NotHandled means that foundMimeType should call KRun::foundMimeType,
         * i.e. launch an external app.
         */
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
        QPointer<QWidget> m_window;
        // Suggested filename given by the server (e.g. HTTP content-disposition)
        // When set, we should really be saving instead of embedding
        QString m_suggestedFilename;
        QString m_sMimeType;
        bool m_bRemoveReferrer;
        bool m_bTrustedSource;
    private:
        void redirectToError( int error, const QString& errorText );
        class BrowserRunPrivate;
        BrowserRunPrivate* const d;

    };
}
#endif
