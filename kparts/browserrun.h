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
         * @param args URL args - includes reload, metaData, etc.
         * @param browserArgs browser-related args - includes data for a HTTP POST, etc.
         * @param part the part going to open this URL - can be 0 if not created yet
         * @param window the mainwindow - passed to KIO::Job::setWindow()
         * @param removeReferrer if true, the "referrer" metadata from @p args isn't passed on
         * @param trustedSource if false, a warning will be shown before launching an executable.
          Always pass false for @p trustedSource, except for local directory views.
         * @param hideErrorDialog if true, no dialog will be shown in case of errors.
         */
        BrowserRun( const QUrl & url,
                    const KParts::OpenUrlArguments& args,
                    const KParts::BrowserArguments& browserArgs,
                    KParts::ReadOnlyPart *part,
                    QWidget *window,
                    bool removeReferrer, bool trustedSource, bool hideErrorDialog = false );

        virtual ~BrowserRun();

        KParts::OpenUrlArguments& arguments();
        KParts::BrowserArguments& browserArguments();
        KParts::ReadOnlyPart* part() const;
        KUrl url() const;

        bool hideErrorDialog() const;

        /**
         * @return Suggested disposition by the server (e.g. HTTP content-disposition)
         */
        QString contentDisposition() const;

        /**
         * @return Wheter the returned disposition suggests saving or opening inline
         */
        bool serverSuggestsSave() const;

        enum AskSaveResult { Save, Open, Cancel };
        /**
         * Ask the user whether to save or open a url in another application.
         * @param url the URL in question
         * @param offer the application that will be used to open the URL
         * @param mimeType the mimetype of the URL
         * @param suggestedFileName optional file name suggested by the server
         * @return Save, Open or Cancel.
         * @deprecated use BrowserOpenOrSaveQuestion
         * @code
         *  BrowserOpenOrSaveQuestion dlg(parent, url, mimeType, suggestedFileName);
         *  const BrowserOpenOrSaveQuestion::Result res = dlg.askOpenOrSave();
         * @endcode
         */
#ifndef KDE_NO_DEPRECATED
        static KPARTS_DEPRECATED AskSaveResult askSave( const QUrl & url, KService::Ptr offer, const QString& mimeType, const QString & suggestedFileName = QString() );
#endif

        enum AskEmbedOrSaveFlags { InlineDisposition = 0, AttachmentDisposition = 1 };
        /**
         * Similar to askSave but for the case where the current application is
         * able to embed the url itself (instead of passing it to another app).
         * @param url the URL in question
         * @param mimeType the mimetype of the URL
         * @param suggestedFileName optional filename suggested by the server
         * @param flags set to AttachmentDisposition if suggested by the server
         * @return Save, Open or Cancel.
         * @deprecated use BrowserOpenOrSaveQuestion
         * @code
         *  BrowserOpenOrSaveQuestion dlg(parent, url, mimeType, suggestedFileName);
         *  const BrowserOpenOrSaveQuestion::Result res = dlg.askEmbedOrSave(flags);
         *  // Important: returns Embed now, not Open!
         * @endcode
         */
#ifndef KDE_NO_DEPRECATED
        static KPARTS_DEPRECATED AskSaveResult askEmbedOrSave( const QUrl & url, const QString& mimeType, const QString & suggestedFileName = QString(), int flags = 0 );
#endif

        // virtual so that KHTML can implement differently (HTML cache)
        virtual void save( const QUrl & url, const QString & suggestedFileName );

        // static so that it can be called from other classes
        static void simpleSave( const QUrl & url, const QString & suggestedFileName,
                                QWidget* window =0 ); // KDE5: remove
        /**
         * If kget integration is enabled, passes the url to kget.
         * Otherwise, asks the user for a destination url, and calls saveUrlUsingKIO.
         * @since 4.4
         */
        static void saveUrl(const KUrl & url, const QString & suggestedFileName,
                            QWidget* window, const KParts::OpenUrlArguments& args);

        /**
         * Starts the KIO file copy job to download @p srcUrl into @p destUrl.
         * @since 4.4
         */
        static void saveUrlUsingKIO(const QUrl & srcUrl, const QUrl & destUrl,
                                    QWidget* window, const QMap<QString, QString> &metaData);

        static bool allowExecution( const QString &mimeType, const KUrl &url );

        static bool isTextExecutable( const QString &mimeType );

        /**
         * KDE webbrowsing kparts support error urls to display errors in-line in the browser component.
         * This helper method creates the error URL from its parameters.
         * @param error the KIO error code (or KIO::ERR_SLAVE_DEFINED if not from KIO)
         * @param errorText the text of the error message
         * @param initialUrl the URL that we were trying to open (as a string, so that this can
         *                   support invalid URLs as well)
         * @since 4.6
         */
        static KUrl makeErrorUrl(int error, const QString& errorText, const QString& initialUrl);

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
        virtual void handleError( KJob * job );

        /**
         * NotHandled means that foundMimeType should call KRun::foundMimeType,
         * i.e. launch an external app.
         */
        enum NonEmbeddableResult { Handled, NotHandled, Delayed };

        /**
         * Helper for foundMimeType: call this if the mimetype couldn't be embedded
         */
        NonEmbeddableResult handleNonEmbeddable( const QString& mimeType ); // TODO KDE5: remove, and add =0 to the other overload

        /**
         * Helper for foundMimeType: call this if the mimetype couldn't be embedded
         * @param mimeType the mimetype found for the URL
         * @param pSelectedService Output variable: pointer to a KService::Ptr, which will be set
         *        to the service selected in the BrowserOpenOrSaveQuestion dialog, if any.
         *
         * How to handle this properly: if pSelectedService is non-zero, then the dialog will show
         * additional "open with" buttons. In your code, you should write:
         * @code
            if (selectedService) {
                KRun::setPreferredService(selectedService->desktopEntryName());
                // and let this code path fall back to KRun::foundMimeType(mimeType);
            } else {
                KRun::displayOpenWithDialog(url(), m_window, false, suggestedFileName());
                setFinished(true);
            }
         * @endcode
         *
         * @since 4.5
         */
        NonEmbeddableResult handleNonEmbeddable(const QString& mimeType, KService::Ptr* pSelectedService);

    protected Q_SLOTS:
        void slotBrowserScanFinished(KJob *job);
        void slotBrowserMimetype(KIO::Job *job, const QString &type);
        void slotCopyToTempFileResult(KJob *job);
        virtual void slotStatResult( KJob *job );

    private:
        void redirectToError( int error, const QString& errorText );
        class BrowserRunPrivate;
        BrowserRunPrivate* const d;

    };
}
#endif
