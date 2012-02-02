/* This file is part of the KDE libraries
 * Copyright (C) 2001 Frerich Raabe <raabe@kde.org>
 *               2003 Carsten Pfeiffer <pfeiffer@kde.org>
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

#ifndef __KPREVIEWWIDGETBASE_H__
#define __KPREVIEWWIDGETBASE_H__

#include <QWidget>

#include <kio/kio_export.h>

class KUrl;

/**
 * Abstract baseclass for all preview widgets which shall be used via
 * KFileDialog::setPreviewWidget(const KPreviewWidgetBase *).
 * Ownership will be transferred to KFileDialog, so you have to create
 * the preview with "new" and let KFileDialog delete it.
 *
 * Just derive your custom preview widget from KPreviewWidgetBase and implement
 * all the pure virtual methods. The slot showPreview(const KUrl &) is called
 * every time the file selection changes.
 *
 * @short Abstract baseclass for all preview widgets.
 * @author Frerich Raabe <raabe@kde.org>
 */
class KIO_EXPORT KPreviewWidgetBase : public QWidget
{
    Q_OBJECT

public:
    /**
     * Constructor. Construct the user interface of your preview widget here
     * and pass the KFileDialog this preview widget is going to be used in as
     * the parent.
     *
     * @param parent The KFileDialog this preview widget is going to be used in
     */
    KPreviewWidgetBase(QWidget *parent);
    ~KPreviewWidgetBase();

public Q_SLOTS:
    /**
     * This slot is called every time the user selects another file in the
     * file dialog. Implement the stuff necessary to reflect the change here.
     *
     * @param url The URL of the currently selected file.
     */
    virtual void showPreview(const KUrl &url) = 0;

    /**
     * Reimplement this to clear the preview. This is called when e.g. the
     * selection is cleared or when multiple selections exist, or the directory
     * is changed.
     */
    virtual void clearPreview() = 0;

    QStringList supportedMimeTypes() const;

protected:
    void setSupportedMimeTypes( const QStringList& mimeTypes );

private:
    class KPreviewWidgetBasePrivate;
    KPreviewWidgetBasePrivate *const d;

    Q_DISABLE_COPY(KPreviewWidgetBase)
};

#endif
