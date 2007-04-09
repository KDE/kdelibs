/*
 *
 * This file is part of the KDE project.
 * Copyright (C) 2001 Martin R. Jones <mjones@kde.org>
 *               2001 Carsten Pfeiffer <pfeiffer@kde.org>
 *
 * You can Freely distribute this program under the GNU Library General Public
 * License. See the file "COPYING" for the exact licensing terms.
 */

#ifndef KIMAGEFILEPREVIEW_H
#define KIMAGEFILEPREVIEW_H

#include <QtGui/QPixmap>

#include <kurl.h>
#include <kpreviewwidgetbase.h>

class KFileItem;
class KJob;
namespace KIO { class PreviewJob; }

/**
 * Image preview widget for the file dialog.
 */
class KIO_EXPORT KImageFilePreview : public KPreviewWidgetBase
{
	Q_OBJECT

	public:
        explicit KImageFilePreview(QWidget *parent);
		~KImageFilePreview();

		virtual QSize sizeHint() const;

	public Q_SLOTS:
		virtual void showPreview(const KUrl &url);
		virtual void clearPreview();

	protected Q_SLOTS:
		void showPreview();
		void showPreview( const KUrl& url, bool force );

		void toggleAuto(bool);
		virtual void gotPreview( const KFileItem*, const QPixmap& );

	protected:
		virtual void resizeEvent(QResizeEvent *e);
		virtual KIO::PreviewJob * createJob( const KUrl& url,
                                                     int w, int h );

	private Q_SLOTS:
		void slotResult( KJob * );
		virtual void slotFailed( const KFileItem* );

        private:
                class KImageFilePreviewPrivate;
        KImageFilePreviewPrivate *const d;

        Q_DISABLE_COPY(KImageFilePreview)
};

#endif // KIMAGEFILEPREVIEW_H
