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

#include <kcombobox.h>
#include <kfilemetainfo.h>
#include <kglobal.h>
#include <klocale.h>
#include <kdebug.h>
#include <klineedit.h>
#include <kstringvalidator.h>

#include <qspinbox.h>
#include <qvalidator.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qcheckbox.h>
#include <qfileinfo.h>
#include <qdatetime.h>
#include <qdatetimeedit.h>

#include <iostream.h>

#ifdef Bool
#undef Bool
#endif

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

class MetaPropsItem
{
public:
    MetaPropsItem(QWidget* w, KFileMetaInfoItem& i, const QString& v)
    {
        widget = w;
        info   = i;
        vclass = v;
    }

    QWidget*            widget;
    KFileMetaInfoItem   info;
    QString             vclass;
};

class KFileMetaPropsPlugin::KFileMetaPropsPluginPrivate
{
public:
    KFileMetaPropsPluginPrivate()  {}
    ~KFileMetaPropsPluginPrivate() {}

    QFrame*                   m_frame;
    QGridLayout*              m_framelayout;
    QFrame*                   m_topframe;
    MetaPropsScrollView*      m_view;
    KFileMetaInfo             m_info;
    QPtrList<MetaPropsItem>   m_items;
//    QPushButton*              m_add;
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
    QStringList l;
    QFileInfo file_info(properties->item()->url().path());

    if ( !d->m_info.isValid() || (l = d->m_info.preferredKeys()) .isEmpty() )
        return;

    // let the dialog create the page frame
    d->m_topframe = properties->dialog()->addPage(i18n("&Meta Info"));
    d->m_topframe->setFrameStyle(QFrame::NoFrame);
    QVBoxLayout* tmp = new QVBoxLayout(d->m_topframe);
  
    // create a scroll view in the page
    d->m_view = new MetaPropsScrollView(d->m_topframe);

    tmp->addWidget(d->m_view);
  
    d->m_frame = d->m_view->frame();
  
    QGridLayout *toplayout = new QGridLayout(d->m_frame);
    toplayout->setSpacing(KDialog::spacingHint());
  
    QStringList::Iterator it = l.begin();
    int count = 0;
    for (; it!=l.end(); ++it)
    {
        KFileMetaInfoItem item = d->m_info.item(*it);
        if ( !item.isValid() ) continue;

        QWidget* w = 0L;
        QString valClass;
        bool editable = file_info.isWritable() && item.isEditable();
        
        if (!editable)
        {
            w = new QLabel(item.string(), d->m_frame);
        }
        else
        {
            switch (item.value().type())
            {
                // if you change something here, you need to also change also
                // slotAdd() and applyChanges()        
                case QVariant::Bool : w = makeBoolWidget  (item, d->m_frame); break;
                case QVariant::Int  : w = makeIntWidget   (item, d->m_frame, valClass); break;
                case QVariant::DateTime  : w = makeDateTimeWidget (item, d->m_frame, valClass); break;
                default             : w = makeStringWidget(item, d->m_frame, valClass); break;
            }
        }
      
        if (w)
        {
            // save a mapping between editable info objects and their widgets
            if (editable)
                 d->m_items.append(new MetaPropsItem(w, item, valClass));
          
            toplayout->addWidget( new QLabel(w, item.translatedKey() + ":",
                                             d->m_frame), count, 0);
            // start and end column for the value widget
            
            QHBoxLayout* sublayout = new QHBoxLayout;
            toplayout->addLayout(sublayout, count, 1);
            
            if (editable && (item.prefix() != QString::null))
            {
                QLabel* label = new QLabel( item.prefix(), d->m_frame );
                label->setAlignment( Qt::AlignRight | Qt::AlignVCenter );
                sublayout->addWidget( label );
                label->setSizePolicy(QSizePolicy(QSizePolicy::Expanding,
                                                 QSizePolicy::Fixed));
                
                if (w->isA("QLabel"))
                      w->setSizePolicy(QSizePolicy(QSizePolicy::Fixed,
                                                   QSizePolicy::Fixed));
                
            }
            else
                if (w->isA("QLabel"))
                {
                      w->setSizePolicy(QSizePolicy(QSizePolicy::Expanding,
                                                   QSizePolicy::Fixed));
                      ((QLabel*)w)->setAlignment( Qt::AlignRight | Qt::AlignVCenter );
                }

            sublayout->addWidget( w );

            if (editable && (item.suffix() != QString::null))
            {
                QLabel* label = new QLabel( item.suffix(), d->m_frame );
                label->setAlignment( Qt::AlignLeft | Qt::AlignVCenter);
                sublayout->addWidget( label );
                label->setSizePolicy(QSizePolicy(QSizePolicy::Fixed,
                                                 QSizePolicy::Fixed));
            }

            count++;

        }
    }
  
    // the add key (disabled until fully implemented)
/*    d->m_add = new QPushButton(i18n("&Add"), d->m_topframe);
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

QWidget* KFileMetaPropsPlugin::makeBoolWidget(const KFileMetaInfoItem& item,
                                              QWidget* parent)
{
  QCheckBox* c = new QCheckBox(parent);
  c->setChecked(item.value().toBool());
  connect(c, SIGNAL(toggled(bool)), this, SIGNAL(changed()));
  return c;
}

QWidget* KFileMetaPropsPlugin::makeIntWidget(const KFileMetaInfoItem& item, 
                                             QWidget* parent, QString& valClass)
{
  QSpinBox* sb = new QSpinBox(parent);
  sb->setValue(item.value().toInt());

  kdDebug(250) << "creating validator for " << item.key().local8Bit() << endl;
  QValidator* val = d->m_info.createValidator(item.key(), 0, 0);

  Q_ASSERT(val);
  
  if (val)
  {
    valClass = val->className();
    if (val->isA("QIntValidator"))
    {
      QIntValidator* validator = static_cast<QIntValidator*>(val);
      sb->setMinValue(validator->bottom());
      sb->setMaxValue(validator->top());
    }

    sb->insertChild(val);
    sb->setValidator(val);

  }
  
  connect(sb, SIGNAL(valueChanged(int)), this, SIGNAL(changed()));
  return sb;
}            

QWidget* KFileMetaPropsPlugin::makeDateTimeWidget(const KFileMetaInfoItem& item, 
                                             QWidget* parent, QString& /*valClass*/)
{
  return new QDateTimeEdit(item.value().toDateTime(), parent);
}

QWidget* KFileMetaPropsPlugin::makeStringWidget(const KFileMetaInfoItem& item, 
                                              QWidget* parent, QString& valClass)
{
  QValidator* validator = d->m_info.createValidator(item.key(), 0, 0);
  valClass = validator->className();
  
  if (validator->isA("KStringListValidator"))
  {
    KComboBox* b = new KComboBox( false, parent );

    // only debugging output
#ifndef NDEBUG
    QStringList l = dynamic_cast<KStringListValidator*>(validator)->stringList();
    QStringList::Iterator it = l.begin();
    for (; it!=l.end(); ++it) kdDebug(250) << (*it).local8Bit() << endl;
#endif
        
    b->insertStringList(static_cast<KStringListValidator*>(validator)->stringList());
    b->setCurrentText( item.value().toString() );
    b->setValidator( validator );
    b->insertChild( validator );
    connect(b, SIGNAL(activated(int)), this, SIGNAL(changed()));
    return b;
  }
  else
  {  
    KLineEdit* e = new KLineEdit( item.value().toString(), parent);
    e->setValidator( validator );
    e->insertChild( validator );
    connect(e, SIGNAL(textChanged(const QString&)), this, SIGNAL(changed()));
    return e;
  }
}        

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
  
  QPtrListIterator<MetaPropsItem> it( d->m_items );
  MetaPropsItem* item;
  for (; (item = it.current()); ++it)
  {
    // we depend on having the correct widget type here
    kdDebug(250) << "wanna add " << item->info.key().latin1() << endl;
    kdDebug(250) << "validator class is " << item->vclass.latin1() << endl;

    if (item->widget->inherits("QSpinBox")) {
        QSpinBox* w = static_cast<QSpinBox*>(item->widget);
        item->info.setValue(QVariant(w->value()));
    }
    else if (item->widget->inherits("QCheckBox")) {
        QCheckBox* w = static_cast<QCheckBox*>(item->widget);
        item->info.setValue(QVariant(w->isChecked()));
    }
    else if (item->widget->inherits("QComboBox")) {
          QComboBox* w = static_cast<QComboBox*>(item->widget);
          item->info.setValue(QVariant(w->currentText()));
    }
    else if (item->widget->inherits("QLineEdit")) {
          QLineEdit* w = static_cast<QLineEdit*>(item->widget);
          item->info.setValue(QVariant(w->text()));
    }
    else if (item->widget->inherits("QDateTimeEdit")) {
          QDateTimeEdit* w = static_cast<QDateTimeEdit*>(item->widget);
          item->info.setValue(QVariant(w->dateTime()));
    }
    else {
        kdDebug(250) << "unrecognized widget type. i'm a monster." << endl;
    }
    
  }
  d->m_info.applyChanges();
}

#include "kmetaprops.moc"
