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

#ifndef KICONDIALOG_H
#define KICONDIALOG_H

#include <kio/kio_export.h>

#include <QtCore/QStringList>
#include <QtGui/QPushButton>

#include <kdialog.h>
#include <klistwidget.h>

class KIconLoader;

/**
 * Icon canvas for KIconDialog.
 */
class KIO_EXPORT KIconCanvas: public KListWidget
{
    Q_OBJECT

public:
    /**
     * Creates a new icon canvas.
     *
     * @param parent The parent widget.
     */
    explicit KIconCanvas(QWidget *parent=0L);

    /**
     * Destroys the icon canvas.
     */
    ~KIconCanvas();

    /**
     * Load icons into the canvas.
     */
    void loadFiles(const QStringList& files);

    /**
     * Returns the current icon.
     */
    QString getCurrent() const;

public Q_SLOTS:
    /**
     * Call this slot to stop the loading of the icons.
     */
    void stopLoading();

Q_SIGNALS:
    /**
     * Emitted when the current icon has changed.
     */
    void nameChanged(const QString&);

    /**
     * This signal is emitted when the loading of the icons
     * has started.
     *
     * @param count The number of icons to be loaded.
     */
    void startLoading(int count);

    /**
     * This signal is emitted whenever an icon has been loaded.
     *
     * @param number The number of the currently loaded item.
     */
    void progress(int number);

    /**
     * This signal is emitted when the loading of the icons
     * has been finished.
     */
    void finished();

private:
    class KIconCanvasPrivate;
    KIconCanvasPrivate* const d;

    Q_DISABLE_COPY(KIconCanvas)

    Q_PRIVATE_SLOT(d, void _k_slotLoadFiles())
    Q_PRIVATE_SLOT(d, void _k_slotCurrentChanged(QListWidgetItem *item))
};


/**
 * Dialog for interactive selection of icons. Use the function
 * getIcon() let the user select an icon.
 *
 * @short An icon selection dialog.
 */
class KIO_EXPORT KIconDialog: public KDialog
{
    Q_OBJECT

public:
    /**
     * Constructs an icon selection dialog using the global iconloader.
     *
     * @param parent The parent widget.
     */
    explicit KIconDialog(QWidget *parent=0L);

    /**
     * Constructs an icon selection dialog using a specific iconloader.
     *
     * @param loader The icon loader to use.
     * @param parent The parent widget.
     */
    explicit KIconDialog(KIconLoader *loader, QWidget *parent=0);

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
     */
    void setCustomLocation( const QString& location );

    /**
     * Sets the size of the icons to be shown / selected.
     * @see KIconLoader::StdSizes
     * @see iconSize
     */
    void setIconSize(int size);

    /**
     * Returns the iconsize set via setIconSize() or 0, if the default
     * iconsize will be used.
     */
    int iconSize() const;

    /**
     * Allows you to set the same parameters as in the class method
     * getIcon(), as well as two additional parameters to lock
     * the choice between system and user dirs and to lock the custom user
     * dir itself.
     */

    void setup( KIconLoader::Group group,
                KIconLoader::Context context = KIconLoader::Application,
                bool strictIconSize = false, int iconSize = 0,
                bool user = false, bool lockUser = false,
                bool lockCustomDir = false );

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
     */
    static QString getIcon(KIconLoader::Group group=KIconLoader::Desktop,
                           KIconLoader::Context context=KIconLoader::Application,
                           bool strictIconSize=false, int iconSize = 0,
                           bool user=false, QWidget *parent=0,
                           const QString &caption=QString());

Q_SIGNALS:
    void newIconName(const QString&);

protected Q_SLOTS:
    void slotOk();

private:
    class KIconDialogPrivate;
    KIconDialogPrivate* const d;

    Q_DISABLE_COPY(KIconDialog)

    Q_PRIVATE_SLOT(d, void _k_slotContext(int))
    Q_PRIVATE_SLOT(d, void _k_slotStartLoading(int))
    Q_PRIVATE_SLOT(d, void _k_slotProgress(int))
    Q_PRIVATE_SLOT(d, void _k_slotFinished())
    Q_PRIVATE_SLOT(d, void _k_slotAcceptIcons())
    Q_PRIVATE_SLOT(d, void _k_slotBrowse())
    Q_PRIVATE_SLOT(d, void _k_slotOtherIconClicked())
    Q_PRIVATE_SLOT(d, void _k_slotSystemIconClicked())
};

#endif // KICONDIALOG_H
