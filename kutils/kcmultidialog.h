/*
   Copyright (c) 2000 Matthias Elter <elter@kde.org>
   Copyright (c) 2003 Daniel Molkentin <molkentin@kde.org>
   Copyright (c) 2003 Matthias Kretz <kretz@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.

*/

#ifndef KCMULTIDIALOG_H
#define KCMULTIDIALOG_H

#include <qptrlist.h>
#include <qptrdict.h>

#include <kdialogbase.h>
#include <kcmodule.h>
#include "kcmoduleinfo.h"

/**
 * A class that offers a KDialogBase containing arbitrary KControl Modules
 *
 * @short A method that offers a KDialogBase containing arbitrary
 *        KControl Modules.
 *
 * @author Matthias Elter <elter@kde.org>, Daniel Molkentin <molkentin@kde.org>
 * @since 3.2
 */
class KCMultiDialog : public KDialogBase
{
    Q_OBJECT

public:
    /**
     * Constructs a new KCMultiDialog
     *
     * @param parent The parent Widget
     * @param name The widget name
     * @param baseGroup The baseGroup, if you want to call a module out of
     *                  kcontrol, just keep "settings"
     * @param modal If you pass true here, the dialog will be modal
     **/
    KCMultiDialog(const QString& baseGroup = QString::fromLatin1("settings"),
                  QWidget *parent=0, const char *name=0,
                  bool modal=false);

    /**
     * Construct a personalized KCMultiDialog.
     *
     * @param dialogFace You can use TreeList, Tabbed, Plain, Swallow or
     *        IconList.
     * @param caption The dialog caption. Do not specify the application name
     *        here. The class will take care of that.
     * @param parent Parent of the dialog.
     * @param name Dialog name (for internal use only).
     * @param modal Controls dialog modality. If @p false, the rest of the
     *        program interface (example: other dialogs) is accessible while
     *        the dialog is open.
     */
    KCMultiDialog( int dialogFace, const QString & caption, QWidget * parent = 0, const char * name = 0, bool modal = false );

    /**
     * Destructor
     **/
   virtual ~KCMultiDialog();

    /**
     * Add a module.
     *
     * @param module Specify the name of the module that is to be added
     *               to the list of modules the dialog will show.
     *
     * @param withfallback Try harder to load the module. Might result
     *                     in the module appearing outside the dialog.
     **/
    void addModule(const QString& module, bool withfallback=true);

    /**
     * Add a module.
     *
     * @param moduleinfo Pass a @ref KCModuleInfo object which will be
     *                   used for creating the module. It will be added
     *                   to the list of modules the dialog will show.
     *
     * @param withfallback Try harder to load the module. Might result
     *                     in the module appearing outside the dialog.
     **/
    void addModule(const KCModuleInfo& moduleinfo, bool withfallback=false);

signals:
    void configCommitted( const QCString & instanceName );

protected slots:
    /**
     * This slot is called when the user presses the "Default" Button
     * You can reimplement it if needed.
     *
     * @note Make sure you call the original implementation!
     **/
    virtual void slotDefault();

    /**
     * This slot is called when the user presses the "Apply" Button
     * You can reimplement it if needed
     *
     * @note Make sure you call the original implementation!
     **/
    virtual void slotApply();

    /**
     * This slot is called when the user presses the "OK" Button
     * You can reimplement it if needed
     *
     * @note Make sure you call the original implementation!
     **/
    virtual void slotOk();

    /**
     * This slot is called when the user presses the "Help" Button
     * You can reimplement it if needed
     *
     * @note Make sure you call the original implementation!
     **/
    virtual void slotHelp();

private slots:

    void slotAboutToShow(QWidget *);

    void clientChanged(bool state);

private:
    void init();
    void apply();

    struct LoadInfo {
      LoadInfo(const KCModuleInfo &_info, bool _withfallback)
         : info(_info), withfallback(_withfallback)
         { }
      KCModuleInfo info;
      bool withfallback;
    };
    QPtrList<KCModule> modules;
    QPtrDict<LoadInfo> moduleDict;
    QPtrDict<QStringList>  modulePrefParent;
    QString _docPath;
    QString _baseGroup;
    bool createTreeList;

    // For future use
    class KCMultiDialogPrivate;
    KCMultiDialogPrivate *d;
};

#endif //KCMULTIDIALOG_H
