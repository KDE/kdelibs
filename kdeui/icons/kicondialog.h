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

#include <kdeui_export.h>

#include <QDialog>
#include <QtCore/QStringList>
#include <QPushButton>

#include <kiconloader.h>

/**
 * Dialog for interactive selection of icons. Use the function
 * getIcon() let the user select an icon.
 *
 * @short An icon selection dialog.
 */
class KDEUI_EXPORT KIconDialog: public QDialog
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
    Q_PRIVATE_SLOT(d, void _k_customFileSelected(const QString &path))
    Q_PRIVATE_SLOT(d, void _k_slotOtherIconClicked())
    Q_PRIVATE_SLOT(d, void _k_slotSystemIconClicked())
};

#endif // KICONDIALOG_H
