/* 
 *
 * This file is part of the KDE project.
 * Copyright (C) 2001 Martin R. Jones <mjones@kde.org>
 * 
 * You can Freely distribute this program under the GNU General Public
 * License. See the file "COPYING" for the exact licensing terms.
 */

#ifndef KIMAGEFILEPREVIEW_H
#define KIMAGEFILEPREVIEW_H

#include <qimage.h>

#include <kurl.h>
#include <kpreviewwidgetbase.h>

class KFileDialog;
class QCheckBox;
class QPushButton;
class QLabel;
class QTimer;

/**
 * Image preview widget for the file dialog.
 */
class KImageFilePreview : public KPreviewWidgetBase 
{
	Q_OBJECT

	public:
		KImageFilePreview(KFileDialog *parent);

		virtual QSize sizeHint() const;

	public slots:
		void showPreview(const KURL &url);

	protected slots:
		void showImage();
		void updatePreview();
		void toggleAuto(const bool);

	protected:
		virtual void resizeEvent(QResizeEvent *e);

	private:
		bool autoMode;
		KURL currentURL;
		QImage currentImage;
		QTimer *timer;
		QLabel *imageLabel;
		QLabel *infoLabel;
		QCheckBox *autoPreview;
		QPushButton *previewButton;
};

#endif // KIMAGEFILEPREVIEW_H
