/*  vi: ts=8 sts=4 sw=4
 *
 *  This file is part of the KDE libraries
 *  Copyright (C) 1997 Christoph Neerfeld <chris@kde.org>
 *            (C) 2000 Kurt Granroth <granroth@kde.org>
 *            (C) 2000 Geert Jansen <jansen@kde.org>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.	If not, write to
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 *
 */

#ifndef __KIconLoaderDialog_h__
#define __KIconLoaderDialog_h__

#include <qstring.h>
#include <qstringlist.h>
#include <qpushbutton.h>

#include "kdialogbase.h"
#include "kiconview.h"

class QComboBox;
class QTimer;
class QKeyEvent;
class QRadioButton;
class KProgress;
class KIconLoader;

/**
 * Canvas for the iconloader dialog.
 */
class KIconLoaderCanvas: public KIconView
{
    Q_OBJECT;

public:
    KIconLoaderCanvas(QWidget *parent=0L, const char *name=0L);
    virtual ~KIconLoaderCanvas();

    void loadFiles(QStringList files);
    QString getCurrent();

protected:
    virtual void keyPressEvent(QKeyEvent *e);

signals:
    void nameChanged(QString);
    void startLoading(int);
    void progress(int);
    void finished();

private slots:
    void slotLoadFiles();
    void slotCurrentChanged(QIconViewItem *item);

private:
    QStringList mFiles;
    QTimer *mpTimer;
    KIconLoader *mpLoader;

    class KIconLoaderCanvasPrivate;
    KIconLoaderCanvasPrivate *d;
};


/**
 * Dialog for interactive selection of icons.
 *
 * KIconLoaderDialog is a derived class from @ref KDialogBase.
 * It provides one function selectIcon() which displays a dialog.
 * This dialog lets you select the icons within the IconPath by image.
 */
class KIconLoaderDialog: public KDialogBase
{
    Q_OBJECT;

public:
    /**
     * Construct the iconloader dialog. This uses the global iconloader to
     * query the available system icons.
     */
    KIconLoaderDialog(QWidget *parent=0L, const char *name=0L);

    /**
     * Alternate constructor to use a different iconloader.
     */
    KIconLoaderDialog(KIconLoader *loader, QWidget *parent=0, 
	    const char *name=0);

    ~KIconLoaderDialog();

    /**
     * Pops up the dialog an lets the user select an icon.
     * @param group The icon group this icon is intended for. This makes
     * sure that the icons are shown with the same size and effects that they 
     * will have when used outside this dialog.
     * @param context The initial icon context. Initially, the icons having
     * the desired context are visible, but the user can change that.
     * @return The name of the icon. The name is suitable for use with 
     * @ref #KIconLoader, i.e. a relative name for installed icons and an
     * absolute path for user supplied ones.
     */
    QString selectIcon(int group=0, int context=0);

private slots:
    void slotButtonClicked(int);
    void slotContext(int);
    void slotStartLoading(int);
    void slotProgress(int);
    void slotFinished();

private:
    void init();
    void showIcons();

    int mGroup, mContext, mType;
    QStringList mFileList;
    QComboBox *mpCombo;
    QPushButton *mpBrowseBut;
    QRadioButton *mpRb1, *mpRb2;
    KProgress *mpProgress;
    KIconLoader *mpLoader;
    KIconLoaderCanvas *mpCanvas;

    class KIconLoaderDialogPrivate;
    KIconLoaderDialogPrivate *d;
};

/**
 * A pushbutton for choosing an icon. Pressing on the button will open a
 * dialog for the user to select an icon. The icon will be displayed on 
 * the button.
 */
class KIconLoaderButton: public QPushButton
{
    Q_OBJECT;

public:
    /** Creates a new KIconLoaderButton. */
    KIconLoaderButton(QWidget *parent=0L, const char *name=0L);

    /** Alternate constructor for use with a different iconloader. */
    KIconLoaderButton(KIconLoader *loader, QWidget *parent, 
	    const char *name=0L);

    ~KIconLoaderButton();

    /** Set the icon group and context. */
    void setIconType(int group, int context);

    /** Set the button's initial icon. */
    void setIcon(QString icon);

    /** Returns the selected icon name. */
    const QString icon() { return mIcon; }

signals:
    void iconChanged(QString icon);

private slots:
    void slotChangeIcon();

private:
    int mGroup, mContext;
    QString mIcon;
    KIconLoaderDialog *mpDialog;
    KIconLoader *mpLoader;;

    class KIconLoaderButtonPrivate;
    KIconLoaderButtonPrivate *d;
};

#endif // __KIconLoaderDialog_h__
