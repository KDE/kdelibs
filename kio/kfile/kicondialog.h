/* vi: ts=8 sts=4 sw=4
 *
 * $Id$
 *
 * This file is part of the KDE project, module kfile.
 * Copyright (C) 2000 Geert Jansen <jansen@kde.org>
 *           (C) 2000 Kurt Granroth <granroth@kde.org>
 *           (C) 1997 Christoph Neerfeld <chris@kde.org>
 *
 * This is free software; it comes under the GNU Library General
 * Public License, version 2. See the file "COPYING.LIB" for the
 * exact licensing terms.
 */

#ifndef __KIconDialog_h__
#define __KIconDialog_h__

#include <qstring.h>
#include <qstringlist.h>
#include <qpushbutton.h>

#include <kicontheme.h>
#include <kdialogbase.h>
#include <kiconview.h>

class QComboBox;
class QTimer;
class QKeyEvent;
class QRadioButton;
class KProgress;
class KIconLoader;



/**
 * Icon canvas for KIconDialog.
 */
class KIconCanvas: public KIconView
{
    Q_OBJECT

public:
    KIconCanvas(QWidget *parent=0L, const char *name=0L);
    ~KIconCanvas();

    /** Load icons into the canvas. */
    void loadFiles(QStringList files);

    /** Returns the current icon. */
    QString getCurrent() const;

public slots:
    void stopLoading();

signals:
    /** Emitted when the current icon has changed. */
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
    class KIconCanvasPrivate;
    KIconCanvasPrivate *d;
};


/**
 * Dialog for interactive selection of icons. Use the function
 * getIcon() let the user select an icon.
 *
 * @short An icon selection dialog.
 */
class KIconDialog: public KDialogBase
{
    Q_OBJECT

public:
    /**
     * Constructs an icon selection dialog using the global iconloader.
     */
    KIconDialog(QWidget *parent=0L, const char *name=0L);
    /**
     * Constructs an icon selection dialog using a specific iconloader.
     */
    KIconDialog(KIconLoader *loader, QWidget *parent=0,
	    const char *name=0);
    /**
     * Destructs the dialog.
     */
    ~KIconDialog();

    /**
     * Sets a strict icon size policy for allowed icons. When true,
     * only icons of the specified group's size in selectIcon are shown.
     * When false, icons not available at the desired group's size will
     * also be selectable.
     */
    void setStrictIconSize(bool b);
    /**
     * Returns true if a strict icon size policy is set.
     */
    bool strictIconSize() const;

    /**
     * @deprecated in KDE 3.0, use the static method getIcon instead.
     */
    QString selectIcon(int group=KIcon::Desktop, int
	    context=KIcon::Application, bool user=false);

    /**
     * Pops up the dialog an lets the user select an icon.
     *
     * @param group The icon group this icon is intended for. Providing the
     * group shows the icons in the dialog with the same appearance as when
     * used outside the dialog.
     * @param context The initial icon context. Initially, the icons having
     * this context are shown in the dialog. The user can change this.
     * @param strictIconSize When true, only icons of the specified group's size
     * are shown, otherwise icon not available in the desired group's size
     * will also be selectable.
     * @param user Begin with the "user icons" instead of "system icons".
     * @return The name of the icon, suitable for loading with KIconLoader.
     * @version New in 3.0
     */
    static QString getIcon(int group=KIcon::Desktop, int context=KIcon::Application,
                           bool strictIconSize=false, bool user=false,
                           QWidget *parent=0, const QString &caption=QString::null);

private slots:
    void slotButtonClicked(int);
    void slotContext(int);
    void slotStartLoading(int);
    void slotProgress(int);
    void slotFinished();
    void slotAcceptIcons();
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
    KIconCanvas *mpCanvas;
    class KIconDialogPrivate;
    KIconDialogPrivate *d;
};


/**
 * A pushbutton for choosing an icon. Pressing on the button will open a
 * KIconDialog for the user to select an icon. The current icon will be
 * displayed on the button.
 *
 * @see KIconDialog
 * @short A push button that allows selection of an icon.
 */
class KIconButton: public QPushButton
{
    Q_OBJECT

public:
    /**
     * Constructs a KIconButton using the global iconloader.
     */
    KIconButton(QWidget *parent=0L, const char *name=0L);

    /**
     * Constructs a KIconButton using a specific KIconLoader.
     */
    KIconButton(KIconLoader *loader, QWidget *parent, const char *name=0L);
    /**
     * Destructs the button.
     */
    ~KIconButton();

    /**
     * Sets a strict icon size policy for allowed icons. When true,
     * only icons of the specified group's size in setIconType are allowed,
     * and only icons of that size will be shown in the icon dialog.
     */
    void setStrictIconSize(bool b);
    /**
     * Returns true if a strict icon size policy is set.
     */
    bool strictIconSize() const;

    /**
     * Sets the icon group and context. Use KIcon::NoGroup if you want to
     * allow icons for any group in the given context.
     */
    void setIconType(int group, int context, bool user=false);

    /**
     * Sets the button's initial icon.
     */
    void setIcon(const QString& icon);

    /**
     * Resets the icon (reverts to an empty button).
     */
    void resetIcon();

    /**
     * Returns the name of the selected icon.
     */
    QString icon() const { return mIcon; }

signals:
    /**
     * Emitted when the icon has changed.
     */
    void iconChanged(QString icon);

private slots:
    void slotChangeIcon();

private:
    bool mbUser;
    int mGroup, mContext;

    QString mIcon;
    KIconDialog *mpDialog;
    KIconLoader *mpLoader;
    class KIconButtonPrivate;
    KIconButtonPrivate *d;
};


#endif // __KIconDialog_h__
