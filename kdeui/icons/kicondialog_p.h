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

#ifndef KICONDIALOG_P_H
#define KICONDIALOG_P_H

#include <QtCore/QStringList>
#include <QFileDialog>
#include <QPushButton>
#include <QPointer>
#include <QListWidget>

#include <kiconloader.h>

class QProgressBar;
class QRadioButton;

class KComboBox;
class KIconCanvasDelegate;
class KListWidgetSearchLine;

/**
 * Icon canvas for KIconDialog.
 */
class KIconCanvas: public QListWidget
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

private Q_SLOTS:
    void loadFiles();
    void currentListItemChanged(QListWidgetItem *item);

private:
    bool m_loading;
    QStringList m_files;
    QTimer *m_timer;
    KIconCanvasDelegate *m_delegate;
};

class KIconDialog::KIconDialogPrivate
{
  public:
    KIconDialogPrivate(KIconDialog *qq) {
        q = qq;
        m_bStrictIconSize = true;
	m_bLockUser = false;
	m_bLockCustomDir = false;
	searchLine = 0;
        mNumOfSteps = 1;
    }
    ~KIconDialogPrivate() {}

    void init();
    void showIcons();
    void setContext( KIconLoader::Context context );

    // slots
    void _k_slotContext(int);
    void _k_slotStartLoading(int);
    void _k_slotProgress(int);
    void _k_slotFinished();
    void _k_slotAcceptIcons();
    void _k_slotBrowse();
    void _k_customFileSelected(const QString &path);
    void _k_slotOtherIconClicked();
    void _k_slotSystemIconClicked();

    KIconDialog *q;

    int mGroupOrSize;
    KIconLoader::Context mContext;

    KComboBox *mpCombo;
    QPushButton *mpBrowseBut;
    QRadioButton *mpSystemIcons, *mpOtherIcons;
    QProgressBar *mpProgress;
    int mNumOfSteps;
    KIconLoader *mpLoader;
    KIconCanvas *mpCanvas;
    int mNumContext;
    KIconLoader::Context mContextMap[ 12 ]; // must match KDE::icon::Context size, code has assert

    bool m_bStrictIconSize, m_bLockUser, m_bLockCustomDir;
    QString custom;
    QString customLocation;
    KListWidgetSearchLine *searchLine;
    QPointer<QFileDialog> browseDialog;
};


#endif // KICONDIALOG_P_H
