/* This file is part of the KDE libraries
 * Copyright (C) 2001, 2002 Rolf Magnus <ramagnus@kde.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License as published by the Free Software Foundation version 2.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 *  $Id$
 */

#include "kmetaprops.h"

#include <kfilemetainfo.h>
#include <kglobal.h>
#include <klocale.h>
#include <kdebug.h>
#include <kfilemetainfowidget.h>

#include <qvalidator.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qfileinfo.h>
#include <qdatetime.h>
#include <qstylesheet.h>
#include <qvgroupbox.h>

#include <iostream.h>

#undef Bool

class MetaPropsScrollView : public QScrollView
{
public:
    MetaPropsScrollView(QWidget* parent = 0, const char* name = 0)
        : QScrollView(parent, name)
    {
      setFrameStyle(QFrame::NoFrame);
      m_frame = new QFrame(viewport(), "MetaPropsScrollView::m_frame");
      m_frame->setFrameStyle(QFrame::NoFrame);
      addChild(m_frame, 0, 0);
    };

    QFrame* frame() {return m_frame;};

protected:
    virtual void viewportResizeEvent(QResizeEvent* ev)
    {
      QScrollView::viewportResizeEvent(ev);
      m_frame->resize( kMax(m_frame->sizeHint().width(), ev->size().width()),
                       kMax(m_frame->sizeHint().height(), ev->size().height()));
    };

private:
      QFrame* m_frame;
};

class KFileMetaPropsPlugin::KFileMetaPropsPluginPrivate
{
public:
    KFileMetaPropsPluginPrivate()  {}
    ~KFileMetaPropsPluginPrivate() {}

    QFrame*                       m_frame;
    QGridLayout*                  m_framelayout;
    KFileMetaInfo                 m_info;
//    QPushButton*                m_add;
    QPtrList<KFileMetaInfoWidget> m_editWidgets;
};

KFileMetaPropsPlugin::KFileMetaPropsPlugin(KPropertiesDialog* props)
  : KPropsDlgPlugin(props)
{
    d = new KFileMetaPropsPluginPrivate;

    KFileItem * fileitem = properties->item();
    kdDebug(250) << "KFileMetaPropsPlugin constructor" << endl;

    d->m_info  = fileitem->metaInfo();
    if (!d->m_info.isValid())
    {
        d->m_info = KFileMetaInfo(properties->kurl().path(-1));
        fileitem->setMetaInfo(d->m_info);
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
    QFileInfo file_info(properties->item()->url().path());

    kdDebug(250) << "KFileMetaPropsPlugin::createLayout" << endl;

    // is there any valid and non-empty info at all?
    if ( !d->m_info.isValid() || (d->m_info.preferredKeys()).isEmpty() )
        return;

    // let the dialog create the page frame
    QFrame* topframe = properties->dialog()->addPage(i18n("&Meta Info"));
    topframe->setFrameStyle(QFrame::NoFrame);
    QVBoxLayout* tmp = new QVBoxLayout(topframe);

    // create a scroll view in the page
    MetaPropsScrollView* view = new MetaPropsScrollView(topframe);

    tmp->addWidget(view);

    d->m_frame = view->frame();

    QVBoxLayout *toplayout = new QVBoxLayout(d->m_frame);
    toplayout->setSpacing(KDialog::spacingHint());

    // now get a list of groups
    KFileMetaInfoProvider* prov = KFileMetaInfoProvider::self();
    QStringList groupList = d->m_info.preferredGroups();

    const KFileMimeTypeInfo* mtinfo = prov->mimeTypeInfo(d->m_info.mimeType());
    if (!mtinfo) kdDebug(7034) << "no mimetype info there\n";

    for (QStringList::Iterator git=groupList.begin(); git!=groupList.end(); ++git)
    {
        kdDebug(7033) << *git << endl;

        QStringList itemList=d->m_info.group(*git).preferredKeys();
        if (itemList.isEmpty())
            continue;

	    QGroupBox *groupBox = new QGroupBox(2, Qt::Horizontal, "", d->m_frame);
    	toplayout->addWidget(groupBox);

        QValueList<KFileMetaInfoItem> readItems;
        QValueList<KFileMetaInfoItem> editItems;

        for (QStringList::Iterator iit = itemList.begin(); iit!=itemList.end(); ++iit)
        {
            KFileMetaInfoItem item = d->m_info[*git][*iit];
            if ( !item.isValid() ) continue;

            bool editable = file_info.isWritable() && item.isEditable();

            if (editable)
                editItems.append( item );
            else
                readItems.append( item );
        }

        KFileMetaInfoWidget* w = 0L;
        // then first add the editable items to the layout
        for (QValueList<KFileMetaInfoItem>::Iterator iit= editItems.begin(); iit!=editItems.end(); ++iit)
        {
            (new QLabel((*iit).translatedKey() + ":", groupBox));

            QValidator* val = mtinfo->createValidator(*git, (*iit).key());
            if (!val) kdDebug(7033) << "didn't get a validator for " << *git << "/" << (*iit).key() << endl;
            w = new KFileMetaInfoWidget(*iit, val, groupBox);
            d->m_editWidgets.append( w );
            connect(w, SIGNAL(valueChanged(const QVariant&)),
                    this, SIGNAL(changed()));
        }

        // and then the read only items
        for (QValueList<KFileMetaInfoItem>::Iterator iit= readItems.begin(); iit!=readItems.end(); ++iit)
        {
            (new QLabel((*iit).translatedKey()+ ":", groupBox));
            (new KFileMetaInfoWidget(*iit, 0L, groupBox));
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
        for (QStringList::Iterator it = sk.begin(); it!=sk.end(); ++it)
        {
                if ( l.find(*it)==l.end() )
                {
                    d->m_add->setEnabled(true);
                    kdDebug(250) << "**first addable key is " << (*it).latin1() << "**" <<endl;
                    break;
                }
                kdDebug(250) << "**already existing key is " << (*it).latin1() << "**" <<endl;
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

bool KFileMetaPropsPlugin::supports( KFileItemList _items )
{
#ifdef _GNUC
#warning TODO: Add support for more than one item
#endif
  if (_items.count()!=1) return false;
  return true;
}

void KFileMetaPropsPlugin::applyChanges()
{
  kdDebug(250) << "applying changes" << endl;
  // insert the fields that changed into the info object

  QPtrListIterator<KFileMetaInfoWidget> it( d->m_editWidgets );
  KFileMetaInfoWidget* w;
  for (; (w = it.current()); ++it) w->apply();
  d->m_info.applyChanges();
}

#include "kmetaprops.moc"
