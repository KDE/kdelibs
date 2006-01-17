/*
 * This file is part of the KDE project.
 * Copyright (C) 2003 Carsten Pfeiffer <pfeiffer@kde.org>
 *
 * You can Freely distribute this program under the GNU Library General Public
 * License. See the file "COPYING" for the exact licensing terms.
 */

#ifndef KFILEMETAPREVIEW_H
#define KFILEMETAPREVIEW_H

#include <QHash>
#include <QStackedWidget>
#include <kpreviewwidgetbase.h>
#include <kurl.h>

class KIO_EXPORT KFileMetaPreview : public KPreviewWidgetBase
{
    Q_OBJECT

public:
    KFileMetaPreview(QWidget *parent);
    ~KFileMetaPreview();

    virtual void addPreviewProvider( const QString& mimeType,
                                     KPreviewWidgetBase *provider );
    virtual void clearPreviewProviders();

public Q_SLOTS:
    virtual void showPreview(const KUrl &url);
    virtual void clearPreview();

protected:
    virtual KPreviewWidgetBase *previewProviderFor( const QString& mimeType );

protected:
    virtual void virtual_hook( int id, void* data );

private:
    void initPreviewProviders();

    QStackedWidget *m_stack;
    QHash<QString,KPreviewWidgetBase*> m_previewProviders;
    bool haveAudioPreview;

    // may return 0L
    static KPreviewWidgetBase * createAudioPreview( QWidget *parent );
    static bool s_tryAudioPreview;

private:
    class KFileMetaPreviewPrivate;
    KFileMetaPreviewPrivate *d;
};

#endif // KFILEMETAPREVIEW_H
