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

// #include <kchoicevalidator.h>
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

#include <iostream.h>

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
    QPushButton*              m_add;
};

KFileMetaPropsPlugin::KFileMetaPropsPlugin(KPropertiesDialog* props)
  : KPropsDlgPlugin(props)
{
    d = new KFileMetaPropsPluginPrivate;
  
    KFileItem * fileitem = properties->item();
    cout << "KFileMetaPropsPlugin constructor" << endl;

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
  
    QGridLayout *toplayout = new QGridLayout(d->m_frame, KDialog::spacingHint());
  
    QStringList::Iterator it = l.begin();
    int count = 0;
    for (; it!=l.end(); ++it)
    {
        KFileMetaInfoItem item = d->m_info.item(*it);
        if ( !item.isValid() ) continue;

        QWidget* w = 0L;
        QString valClass;
        
        switch (item.value().type())
        {
            // if you change something here, you need to also change also
            // slotAdd() and applyChanges()        
            case QVariant::Bool : w = makeBoolWidget  (item, d->m_frame); break;
            case QVariant::Int  : w = makeIntWidget   (item, d->m_frame, valClass); break;
            default             : w = makeStringWidget(item, d->m_frame, valClass); break;
        }
      
        if (w)
        {
            // save a mapping between editable info objects and their widgets
            if (item.isEditable())
            d->m_items.append(new MetaPropsItem(w, item, valClass));
          
            toplayout->addWidget( new QLabel(w, item.translatedKey() + ":",
                                             d->m_frame), count, 0);
            // start and end column for the value widget
            int left=1,right=3;

            if (item.prefix()!=QString::null)
            {
                left = 2;
                QLabel* label = new QLabel( item.prefix(), d->m_frame );
                label->setAlignment( Qt::AlignRight );
                toplayout->addWidget( label, count, 1 );
            }

            if (item.suffix()!=QString::null)
            {
                right = 2;
                QLabel* label = new QLabel( item.suffix(), d->m_frame );
                label->setAlignment( Qt::AlignLeft );
                toplayout->addWidget( label, count, 3 );
            }
        
            toplayout->addMultiCellWidget( w, count, count, left, right );
            count++;
        }
    }
  
    // the add key
    d->m_add = new QPushButton(i18n("&Add"), d->m_topframe);
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
                    cout << "**first addable key is " << (*it).latin1() << "**" <<endl;
                    break;
                }
                cout << "**already existing key is " << (*it).latin1() << "**" <<endl;
        }
    }
}

void KFileMetaPropsPlugin::slotAdd()
{
    // add a lineedit for the name
  
  
  
    // insert the item in the list
  
}

QWidget* KFileMetaPropsPlugin::makeBoolWidget(const KFileMetaInfoItem& item,
                                              QWidget* parent)
{
  if (!item.isEditable())
    return new QLabel(item.value().toBool() ? i18n("Yes") : i18n("No"), parent);

  QCheckBox* c = new QCheckBox(parent);
  c->setChecked(item.value().toBool());
  connect(c, SIGNAL(toggled()), this, SIGNAL(changed()));
  return c;
}

QWidget* KFileMetaPropsPlugin::makeIntWidget(const KFileMetaInfoItem& item, 
                                             QWidget* parent, QString& valClass)
{
  if (!item.isEditable()) return new QLabel(item.value().toString(), parent);
  
  QSpinBox* sb = new QSpinBox(parent);
  sb->setValue(item.value().toInt());

  cout << "creating validator for " << item.key().local8Bit() << endl;
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

QWidget* KFileMetaPropsPlugin::makeStringWidget(const KFileMetaInfoItem& item, 
                                              QWidget* parent, QString& valClass)
{
  if (!item.isEditable()) return new QLabel(item.value().toString(), parent);

  QValidator* validator = d->m_info.createValidator(item.key(), 0, 0);
  valClass = validator->className();
  
  if (validator->isA("KStringListValidator"))
  {
    KComboBox* b = new KComboBox( false, parent );

    // only debugging output
    QStringList l = dynamic_cast<KStringListValidator*>(validator)->stringList();
    QStringList::Iterator it = l.begin();
    for (; it!=l.end(); ++it) cout << (*it).local8Bit() << endl;
        
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
  cout << "applying changes" << endl;
  // insert the fields that changed into the info object
  
  QPtrListIterator<MetaPropsItem> it( d->m_items );
  MetaPropsItem* item;
  for (; (item = it.current()); ++it)
  {
    // we depend on having the correct widget type here
    cout << "wanna add " << item->info.key().latin1() << endl;
    cout << "validator class is " << item->vclass.latin1() << endl;
    switch (item->info.type())
    {
      case QVariant::Int:
      {
        QSpinBox* w = static_cast<QSpinBox*>(item->widget);
        item->info.setValue(QVariant(w->value()));
        break;
      }
      case QVariant::Bool:
      {
        QCheckBox* w = static_cast<QCheckBox*>(item->widget);
        item->info.setValue(QVariant(w->isChecked()));
        break;
      }
      case QVariant::CString:
      {
        if (item->vclass == "KChoiceValidator")
        {
          KComboBox* w = static_cast<KComboBox*>(item->widget);
          item->info.setValue(QVariant(w->currentText().local8Bit()));
        }
        else
        {
          QLineEdit* w = static_cast<QLineEdit*>(item->widget);
          item->info.setValue(QVariant(w->text().local8Bit()));
        }
        break;
      }
      default:
      {
        if (item->vclass == "KChoiceValidator")
        {
          KComboBox* w = static_cast<KComboBox*>(item->widget);
          item->info.setValue(QVariant(w->currentText()));
        }
        else
        {
          QLineEdit* w = static_cast<QLineEdit*>(item->widget);
          item->info.setValue(QVariant(w->text()));
        }
      }
    }
    
  }
  d->m_info.applyChanges();
}

#include "kmetaprops.moc"
