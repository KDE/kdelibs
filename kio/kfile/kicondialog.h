/* vi: ts=8 sts=4 sw=4
 *
 * This file is part of the KDE project, module kfile.
 * Copyright (C) 2000 Geert Jansen <jansen@kde.org>
 *           (C) 2000 Kurt Granroth <granroth@kde.org>
 *           (C) 1997 Christoph Neerfeld <chris@kde.org>
 *           (C) 2002 Carsten Pfeiffer <pfeiffer@kde.org>
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
class QProgressBar;
class KIconLoader;

/**
 * Icon canvas for KIconDialog.
 */
class KIO_EXPORT KIconCanvas: public KIconView
{
    Q_OBJECT

public:
    KIconCanvas(QWidget *parent=0L, const char *name=0L);
    ~KIconCanvas();

    /**
     * Load icons into the canvas.
     */
    void loadFiles(const QStringList& files);

    /**
     * Returns the current icon.
     */
    QString getCurrent() const;

public slots:
    void stopLoading();

signals:
    /**
     * Emitted when the current icon has changed.
     */
    void nameChanged(const QString&);

    void startLoading(int);
    void progress(int);
    void finished();

private slots:
    void slotLoadFiles();
    void slotCurrentChanged(Q3IconViewItem *item);

private:
    QStringList mFiles;
    QTimer *mpTimer;
    KIconLoader *mpLoader;

protected:
    virtual void virtual_hook( int id, void* data );

private:
    class KIconCanvasPrivate;
    KIconCanvasPrivate* const d;
};


/**
 * Dialog for interactive selection of icons. Use the function
 * getIcon() let the user select an icon.
 *
 * @short An icon selection dialog.
 */
class KIO_EXPORT KIconDialog: public KDialogBase
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
     * only icons of the specified group's size in getIcon() are shown.
     * When false, icons not available at the desired group's size will
     * also be selectable.
     */
    void setStrictIconSize(bool b);
    /**
     * Returns true if a strict icon size policy is set.
     */
    bool strictIconSize() const;
    /**
     * sets a custom icon directory
     * @since 3.1
     */
    void setCustomLocation( const QString& location );

    /**
     * Sets the size of the icons to be shown / selected.
     * @see KIcon::StdSizes
     * @see iconSize
     */
    void setIconSize(int size);

    /**
     * Returns the iconsize set via setIconSize() or 0, if the default
     * iconsize will be used.
     */
    int iconSize() const;

#ifndef KDE_NO_COMPAT
    /**
     * @deprecated in KDE 3.0, use the static method getIcon instead.
     */
    QString selectIcon(KIcon::Group group=KIcon::Desktop, KIcon::Context
	    context=KIcon::Application, bool user=false);
#endif

    /**
     * Allows you to set the same parameters as in the class method
     * getIcon().
     */
    void setup( KIcon::Group group,
                KIcon::Context context = KIcon::Application,
                bool strictIconSize = false, int iconSize = 0,
                bool user = false );

    /**
     * Allows you to set the same parameters as in the class method
     * getIcon(), as well as two additional parameters to lock
     * the choice between system and user dirs and to lock the custom user
     * dir itself.
     *
     * @since 3.3
     */

    void setup( KIcon::Group group, KIcon::Context context,
                bool strictIconSize, int iconSize, bool user, bool lockUser,
                bool lockCustomDir );

    /**
     * exec()utes this modal dialog and returns the name of the selected icon,
     * or QString() if the dialog was aborted.
     * @returns the name of the icon, suitable for loading with KIconLoader.
     * @see getIcon
     */
    QString openDialog();

    /**
     * show()es this dialog and emits a newIcon(const QString&) signal when
     * successful. QString() will be emitted if the dialog was aborted.
     */
    void showDialog();

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
     * @param iconSize the size of the icons -- the default of the icongroup
     *        if set to 0
     * @param user Begin with the "user icons" instead of "system icons".
     * @param parent The parent widget of the dialog.
     * @param caption The caption to use for the dialog.
     * @return The name of the icon, suitable for loading with KIconLoader.
     * @version New in 3.0
     */
    static QString getIcon(KIcon::Group group=KIcon::Desktop,
                           KIcon::Context context=KIcon::Application,
                           bool strictIconSize=false, int iconSize = 0,
                           bool user=false, QWidget *parent=0,
                           const QString &caption=QString());

signals:
    void newIconName(const QString&);

protected slots:
    void slotOk();

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

    int mGroupOrSize;
    KIcon::Context mContext;
    int mType;

    QStringList mFileList;
    QComboBox *mpCombo;
    QPushButton *mpBrowseBut;
    QRadioButton *mpRb1, *mpRb2;
    QProgressBar *mpProgress;
    KIconLoader *mpLoader;
    KIconCanvas *mpCanvas;

protected:
    virtual void virtual_hook( int id, void* data );
private:
    class KIconDialogPrivate;
    KIconDialogPrivate* const d;
};


/**
 * A pushbutton for choosing an icon. Pressing on the button will open a
 * KIconDialog for the user to select an icon. The current icon will be
 * displayed on the button.
 *
 * @see KIconDialog
 * @short A push button that allows selection of an icon.
 */
class KIO_EXPORT KIconButton: public QPushButton
{
    Q_OBJECT
    Q_PROPERTY( QString icon READ icon WRITE setIcon RESET resetIcon )
    Q_PROPERTY( int iconSize READ iconSize WRITE setIconSize)
    Q_PROPERTY( bool strictIconSize READ strictIconSize WRITE setStrictIconSize )

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
    void setIconType(KIcon::Group group, KIcon::Context context, bool user=false);

    /**
     * Sets the button's initial icon.
     */
    void setIcon(const QString& icon);

    /**
     * @since 4.0
     */
    void setIcon(const QIcon& icon)
    {QPushButton::setIcon(icon);}

    /**
     * Resets the icon (reverts to an empty button).
     */
    void resetIcon();

    /**
     * Returns the name of the selected icon.
     */
    QString icon() const { return mIcon; }

    /**
     * Sets the size of the icon to be shown / selected.
     * @see KIcon::StdSizes
     * @see iconSize
     */
    void setIconSize( int size );

    /**
     * Returns the iconsize set via setIconSize() or 0, if the default
     * iconsize will be used.
     */
    int iconSize() const;

signals:
    /**
     * Emitted when the icon has changed.
     */
    void iconChanged(const QString &icon);

private slots:
    void slotChangeIcon();
    void newIconName(const QString& name);

private:
    void init( KIconLoader *loader );

    bool mbUser;
    KIcon::Group mGroup;
    KIcon::Context mContext;

    QString mIcon;
    KIconDialog *mpDialog;
    KIconLoader *mpLoader;
    class KIconButtonPrivate;
    KIconButtonPrivate* const d;
};


#endif // __KIconDialog_h__
