/* This file is part of the KDE libraries
    Copyright (C) 2001,2002 Rolf Magnus <ramagnus@kde.org>

    library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
 */

#include "kmetaprops.h"
#include "kpropertiesdialog_p.h"

#include <kdebug.h>
#include <kfilemetainfowidget.h>
#include <kfilemetainfo.h>
#include <kglobal.h>
#include <kglobalsettings.h>
#include <klocale.h>
#include <kprotocolinfo.h>

#include <QtGui/QDoubleValidator>
#include <QtGui/QLayout>
#include <QtGui/QLabel>
#include <QtCore/QFileInfo>
#include <QtCore/QDate>
#include <QtGui/QGroupBox>
#include <QResizeEvent>
#include <QtCore/QLinkedList>
#include <QScrollArea>
#include <QTextDocument>

using namespace KDEPrivate;

class KFileMetaPropsPlugin::KFileMetaPropsPluginPrivate
{
public:
    KFileMetaPropsPluginPrivate()  {}
    ~KFileMetaPropsPluginPrivate() {}

    QWidget*                      m_frame;
    QGridLayout*                  m_framelayout;
    KFileMetaInfo                 m_info;
//    QPushButton*                m_add;
    QList<KFileMetaInfoWidget *> m_editWidgets;
};

KFileMetaPropsPlugin::KFileMetaPropsPlugin(KPropertiesDialog* props)
  : KPropertiesDialogPlugin(props),d(new KFileMetaPropsPluginPrivate)
{

    KFileItem &fileitem = properties->item();
    kDebug(250) << "KFileMetaPropsPlugin constructor";

    d->m_info  = fileitem.metaInfo();
    if (!d->m_info.isValid())
    {
        d->m_info = KFileMetaInfo(properties->kurl().path(KUrl::RemoveTrailingSlash));
        fileitem.setMetaInfo(d->m_info);
    }

    if ( properties->items().count() > 1 )
    {
        // not yet supported
        // we should allow setting values for a list of files. Itt makes sense
        // in some cases, like the album of a list of mp3s
        return;
    }

    createLayout();

    setDirty(true);
}

void KFileMetaPropsPlugin::createLayout()
{
    QFileInfo file_info(properties->item().url().toLocalFile());

    kDebug(250) << "KFileMetaPropsPlugin::createLayout";

    // is there any valid and non-empty info at all?
    if ( !d->m_info.isValid() )
        return;

    // now get a list of groups
    //KFileMetaInfoProvider* prov = KFileMetaInfoProvider::self();
    KFileMetaInfoGroupList groupList = d->m_info.preferredGroups();
    if (groupList.isEmpty())
        return;

    QScrollArea* scrollArea = new QScrollArea;
    scrollArea->setFrameStyle(QFrame::NoFrame);
    properties->addPage(scrollArea, i18n("&Meta Info"));

    d->m_frame = new QWidget(scrollArea);
    scrollArea->setWidget(d->m_frame);
    scrollArea->setWidgetResizable(true);

    QVBoxLayout *toplayout = new QVBoxLayout(d->m_frame);

    foreach (const KFileMetaInfoGroup& group, groupList) {
        //kDebug(7033) << *git;

        KFileMetaInfoItemList itemList = group.items();
        if (itemList.isEmpty())
            continue;

        QGroupBox *groupBox = new QGroupBox( Qt::escape(group.name()),
            d->m_frame);
        QGridLayout *grouplayout = new QGridLayout(groupBox);
        grouplayout->activate();

        toplayout->addWidget(groupBox);

        KFileMetaInfoItemList readItems;
        KFileMetaInfoItemList editItems;

        foreach (const KFileMetaInfoItem& item, itemList) {
            if ( !item.isValid() ) continue;

            bool editable = file_info.isWritable() && item.isEditable();

            if (editable)
                editItems.append( item );
            else
                readItems.append( item );
        }

        KFileMetaInfoWidget* w = 0L;
        int row = 0;
        // then first add the editable items to the layout
        foreach (const KFileMetaInfoItem& item, editItems) {
            QLabel* l = new QLabel(item.name() + ':', groupBox);
            grouplayout->addWidget(l, row, 0);
            l->setAlignment( Qt::AlignLeft | Qt::AlignTop );
            QValidator* val = item.properties().createValidator();
            if (!val) kDebug(7033) << "didn't get a validator for "
                << item.name() << endl;
            w = new KFileMetaInfoWidget(item, val, groupBox);
            grouplayout->addWidget(w, row, 1);
            d->m_editWidgets.append( w );
            connect(w, SIGNAL(valueChanged(const QVariant&)), this, SIGNAL(changed()));
            ++row;
        }

        // and then the read only items
        foreach (const KFileMetaInfoItem& item, readItems) {
            QLabel* l = new QLabel(item.name() + ':', groupBox);
            grouplayout->addWidget(l, row, 0);
            l->setAlignment( Qt::AlignLeft | Qt::AlignTop );
            w = new KFileMetaInfoWidget(item, KFileMetaInfoWidget::ReadOnly, 0L, groupBox);
            grouplayout->addWidget(w, row, 1);
            ++row;
        }
    }

    toplayout->addStretch(1);

    // the add key (disabled until fully implemented)
/*    d->m_add = new QPushButton(i18n("&Add"), topframe);
    d->m_add->setSizePolicy(QSizePolicy(QSizePolicy::Fixed,
                                        QSizePolicy::Fixed));
    connect(d->m_add, SIGNAL(clicked()), this, SLOT(slotAdd()));
    tmp->addWidget(d->m_add);

    // if nothing can be added, deactivate it
    if ( !d->m_info.supportsVariableKeys() )
    {
        // if supportedKeys() does contain anything not in preferredKeys,
        // we have something addable

        QStringList sk = d->m_info.supportedKeys();
        d->m_add->setEnabled(false);
        for (QStringList::const_iterator it = sk.begin(); it!=sk.end(); ++it)
        {
                if ( l.find(*it)==l.end() )
                {
                    d->m_add->setEnabled(true);
                    kDebug(250) << "**first addable key is " << (*it).toLatin1().constData() << "**";
                    break;
                }
                kDebug(250) << "**already existing key is " << (*it).toLatin1().constData() << "**";
        }
    } */
}

/*void KFileMetaPropsPlugin::slotAdd()
{
    // add a lineedit for the name



    // insert the item in the list

}*/

KFileMetaPropsPlugin::~KFileMetaPropsPlugin()
{
    delete d;
}

bool KFileMetaPropsPlugin::supports( const KFileItemList& _items )
{
    kDebug() ;

    // TODO: Add support for more than one item

    // TODO check that KDesktopPropsPlugin is correct, i.e. that we never want metainfo for
    // a .desktop file? Used to be that only Application desktop files were filtered out
    if (KDesktopPropsPlugin::supports(_items) || KUrlPropsPlugin::supports(_items))
        return false; // Having both is redundant.
    if (_items.count() != 1)
        return false;

    bool metaDataEnabled = KGlobalSettings::showFilePreview(_items.first().url());
    kDebug() << "metaDataEnabled=" << metaDataEnabled;
    return metaDataEnabled;
}

void KFileMetaPropsPlugin::applyChanges()
{
    kDebug(250) << "applying changes";
    // insert the fields that changed into the info object

    foreach(KFileMetaInfoWidget* w, d->m_editWidgets)
        w->apply();
    d->m_info.applyChanges();
}

#include "kmetaprops.moc"
