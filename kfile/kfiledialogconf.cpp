/* This file is part of the KDE libraries
    Copyright (C) 1997, 1998 Richard Moore <rich@kde.org>
                  1998 Stephan Kulow <coolo@kde.org>
                  1998 Daniel Grana <grana@ie.iwi.unibe.ch>
    
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

#include <qlayout.h>
#include <qlined.h>
#include <qbttngrp.h>
#include <qgrpbox.h>
#include <qchkbox.h>
#include "kfiledialogconf.h"
#include <kapp.h>

enum Buttons { B_STATUSLINE=10, B_FILTER, B_SINGLECLICK, B_HIDDEN, 
	       B_LISTLABELS, B_SHORTVIEW, B_DETAILVIEW, B_MIX, B_KEEPDIR };


KFileDialogConfigureDlg::KFileDialogConfigureDlg(QWidget *parent, 
						 const char *name)
  : QTabDialog(parent, name, true)
{
  KFileDialogConfigure *kfdc= new KFileDialogConfigure(this);

  resize(kfdc->size());
  addTab(kfdc, klocale->translate("Look and Feel"));

  QLabel *label= new QLabel("KFileDialog\nBy\n"
			    "Richard Moore\nrich@kde.org", this);
  label->setAlignment(AlignCenter);
  label->adjustSize();
  addTab(label, klocale->translate("About"));

  setDefaultButton();
  setCancelButton();
  setApplyButton();
  setOKButton();
  connect( this, SIGNAL(applyButtonPressed()), kfdc, SLOT(saveConfiguration()) );
};

KFileDialogConfigure::KFileDialogConfigure(QWidget *parent, 
					   const char *name)
  : QWidget(parent, name)
{

  QHBoxLayout *horizontal = new QHBoxLayout( this, 10 );

  QVBoxLayout *vertical = new QVBoxLayout( );
  horizontal->addLayout( vertical, 10);
  
  QGroupBox *box1 = new QGroupBox( i18n("Width"), this ); 
  QHBoxLayout *hor1 = new QHBoxLayout( box1, 5 );
  myWidth = new QSlider( 0,
                         1280,
                         1,
                         300,
                         QSlider::Horizontal,
                         box1,
                         "???");
  myWidth->adjustSize();
  myWidth->setMaximumHeight( 25 );
  hor1->addWidget( myWidth, 1 );
  myWidthLabel = new QLabel(box1);
  myWidthLabel->setNum(1000);
  myWidthLabel->adjustSize();
  myWidthLabel->setFixedSize( myWidthLabel->sizeHint() );
  hor1->addSpacing( 0 );
  hor1->addWidget( myWidthLabel, 0 );
  box1->adjustSize();
  box1->setMinimumSize( 100, 50 );
  vertical->addWidget(box1, 5);
  
  QGroupBox *box2 = new QGroupBox( i18n("Height"), this ); 
  QHBoxLayout *hor2 = new QHBoxLayout( box2, 5 );
  myHeight = new QSlider( 0,
                          1280,
                          1,
                          300,
                          QSlider::Horizontal,
                          box2,
                          "HeightSlider");
  myHeight->adjustSize();
  myHeight->setMaximumHeight( 25 );
  hor2->addWidget( myHeight, 1 ),
  myHeightLabel = new QLabel(box2);
  myHeightLabel->setNum(1000);
  myHeightLabel->adjustSize();
  myHeightLabel->setFixedSize( myHeightLabel->sizeHint() );
  hor2->addSpacing( 5 );
  hor2->addWidget( myHeightLabel, 0 );
  box2->adjustSize();
  box2->setMinimumSize( 100, 50 );
  vertical->addWidget(box2, 5);

  QGroupBox *box3 = new QGroupBox( i18n("Panner"), this ); 
  QHBoxLayout *hor3 = new QHBoxLayout( box3, 5 );
  myPanner = new QSlider( 0,
                         100,
                         1,
                         40,
                         QSlider::Horizontal,
                         box3,
                         "PannerSlider");
  myPanner->setMaximumHeight( 24 );
  myPanner->adjustSize();
  hor3->addWidget( myPanner, 1 );
  myPannerLabel = new QLabel(box3);
  myPannerLabel->setNum(100);
  myPannerLabel->adjustSize();
  myPannerLabel->setFixedSize( myPannerLabel->sizeHint() );
  hor3->addSpacing( 0 );
  hor3->addWidget( myPannerLabel, 0 );
  box3->adjustSize();
  box3->setMinimumSize( 100, 50);
  vertical->addWidget(box3, 5);
  
  connect( myWidth, SIGNAL(valueChanged(int)), myWidthLabel, SLOT(setNum(int)) );
  connect( myHeight, SIGNAL(valueChanged(int)), myHeightLabel, SLOT(setNum(int)) );
  connect( myPanner, SIGNAL(valueChanged(int)), myPannerLabel, SLOT(setNum(int)) );
  //
  //
  QVBoxLayout *choices = new QVBoxLayout(5);
  horizontal->addLayout( choices, 2 );
  
  QButtonGroup *group= new QButtonGroup( i18n("View style"), this);
  group->setExclusive(true);
  QVBoxLayout *l1 = new QVBoxLayout(group, 5);  
  l1->addSpacing(10);
  myShortView= new QRadioButton( i18n("Show Short View"), group);
  group->insert( myShortView, B_SHORTVIEW );
  myShortView->adjustSize();
  myShortView->setMinimumSize( myShortView->sizeHint() );
  l1->addWidget( myShortView, 0 );
  l1->addSpacing(10);
  myDetailView= new QRadioButton( i18n("Show Detail View"), group);
  group->insert( myDetailView, B_DETAILVIEW );
  myDetailView->adjustSize();
  myDetailView->setMinimumSize( myDetailView->sizeHint() );
  l1->addWidget( myDetailView, 0 );
  choices->addWidget(group, 2);
  
  // Short View not working right now
  // myShortView->setEnabled(false);
 
  //
  //
  QButtonGroup *group2= new QButtonGroup(klocale->translate("Misc"), this);
  choices->addSpacing(15);
  choices->addWidget( group2, 5 );
  
  QVBoxLayout *l2 = new QVBoxLayout(group2, 5);  
  l2->addSpacing(10);
  myShowFilter= new QCheckBox(i18n("Show Filter"), group2);
  group2->insert( myShowFilter, B_FILTER );
  myShowFilter->adjustSize();
  myShowFilter->setMinimumSize( myShowFilter->sizeHint() );
  l2->addWidget( myShowFilter, 0 );
  l2->addSpacing(10);
  myShowListLabels= new QCheckBox(i18n("Show List Labels"), group2);
  group2->insert( myShowListLabels, B_LISTLABELS );
  myShowListLabels->adjustSize();
  myShowListLabels->setMinimumSize( myShowListLabels->sizeHint() );
  l2->addWidget( myShowListLabels, 0 );
  l2->addSpacing(10);
  myShowHidden= new QCheckBox( i18n("Show Hidden"), group2);
  group2->insert( myShowHidden, B_HIDDEN );
  myShowHidden->adjustSize();
  myShowHidden->setMinimumSize( myShowHidden->sizeHint() );
  l2->addWidget( myShowHidden, 0 );
  l2->addSpacing(10);
  myShowStatusLine= new QCheckBox( i18n("Show Status Line"), group2);
  group2->insert( myShowStatusLine, B_STATUSLINE );
  myShowStatusLine->adjustSize();
  myShowStatusLine->setMinimumSize( myShowStatusLine->sizeHint() );
  l2->addWidget( myShowStatusLine, 0 );
  l2->addSpacing(10);
  useSingleClick= new QCheckBox( i18n("Use single Click"), group2);
  group2->insert( useSingleClick, B_SINGLECLICK );
  useSingleClick->adjustSize();
  useSingleClick->setMinimumSize( useSingleClick->sizeHint() );
  l2->addWidget( useSingleClick, 0 );
  myMixDirsAndFiles = new QCheckBox( i18n("Mix dirs and files"), group2);
  group2->insert( myMixDirsAndFiles, B_MIX );
  myMixDirsAndFiles->adjustSize();
  myMixDirsAndFiles->setMinimumSize( myMixDirsAndFiles->sizeHint() );
  l2->addWidget( myMixDirsAndFiles, 0 );
  myKeepDirsFirst = new QCheckBox( i18n("Keep dirs first"), group2);
  group2->insert( myKeepDirsFirst, B_KEEPDIR );
  myKeepDirsFirst->adjustSize();
  myKeepDirsFirst->setMinimumSize( myKeepDirsFirst->sizeHint() );
  l2->addWidget( myKeepDirsFirst, 0 );

  group2->setMinimumSize( group2->childrenRect().size() );
  l2->activate();
  group->setMinimumSize( group->childrenRect().size() );
  l1->activate();
  horizontal->activate();
  this->adjustSize();

/*
  horizontal->addSpacing( 10 );
  horizontal->addWidget( group2, 5 );

   QBoxLayout *horizontalx= new QBoxLayout(QBoxLayout::LeftToRight);
   QLineEdit *findPathEdit;
   findPathEdit= new QLineEdit(this, "findpathedit");
   QLabel *tmpLabel;
   tmpLabel= new QLabel(findPathEdit, klocale->translate("&Find command:"), this);
   tmpLabel->adjustSize();
   tmpLabel->setAlignment(AlignCenter);
   tmpLabel->setMinimumSize(tmpLabel->size());
   tmpLabel->setMaximumHeight(tmpLabel->height());
   findPathEdit->setMinimumHeight(tmpLabel->size().height()+8);
   findPathEdit->setMaximumHeight(tmpLabel->size().height()+8);

   vertical->addLayout(horizontalx);
   horizontal->addWidget(tmpLabel, 1);
   horizontal->addWidget(findPathEdit, 10);
   vertical->activate();
*/

  // Now read the current settings
  KConfig *c;
  QString oldgroup;
  QString dirview;

  c= kapp->getConfig();
  oldgroup= c->group();
  c->setGroup("KFileDialog Settings");

  myWidth->setValue(c->readNumEntry("Width", 400));
  myHeight->setValue(c->readNumEntry("Height", 400));
  myPanner->setValue(c->readNumEntry("PannerPosition", 40));
  myPannerLabel->setNum(myPanner->value());

  QString currentViewStyle = c->readEntry("ViewStyle","DetailView");
  if ( currentViewStyle == "DetailView" )
    myDetailView->setChecked(TRUE);
  else
    myShortView->setChecked(TRUE);
  myShowHidden->setChecked(c->readNumEntry("ShowHidden", 0) != 0);
  myShowFilter->setChecked(c->readNumEntry("ShowFilter", 1) != 0);
  myShowListLabels->setChecked(c->readNumEntry("ShowListLabels", 1) != 0);
  useSingleClick->setChecked(c->readNumEntry("SingleClick", 1) != 0);
  myShowStatusLine->setChecked(c->readNumEntry("ShowStatusLine", 1) != 0);
  myMixDirsAndFiles->setChecked(c->readNumEntry("MixDirsAndFiles", 0));
  myKeepDirsFirst->setChecked(c->readNumEntry("KeepDirsFirst", 1));

  // Restore the old config group
  c->setGroup(oldgroup);

}

void 
KFileDialogConfigure::saveConfiguration()
{
  // Now read the current settings
  KConfig *c;
  QString oldgroup;
  QString dirview;

  c= kapp->getConfig();
  oldgroup= c->group();
  c->setGroup("KFileDialog Settings");

  c->writeEntry("Width", myWidth->value());
  c->writeEntry("Height", myHeight->value());
  c->writeEntry("PannerPosition", myPanner->value());

  if ( myDetailView->isChecked() )
    c->writeEntry("ViewStyle","DetailView");
  else
    c->writeEntry("ViewStyle","ShortView");

  if (myShowHidden->isChecked() )
    c->writeEntry("ShowHidden", 1);
  else
    c->writeEntry("ShowHidden", 0);

  if (myShowFilter->isChecked() )
    c->writeEntry("ShowFilter", 1);
  else
    c->writeEntry("ShowFilter", 0);

  if (myShowListLabels->isChecked() )
    c->writeEntry("ShowListLabels", 1);
  else
    c->writeEntry("ShowListLabels", 0);

  if (useSingleClick->isChecked() )
    c->writeEntry("SingleClick", 1);
  else
    c->writeEntry("SingleClick", 0);
  
  if (myShowStatusLine->isChecked() )
    c->writeEntry("ShowStatusLine", 1);
  else
    c->writeEntry("ShowStatusLine", 0);

  c->writeEntry("MixDirsAndFiles", 
		myMixDirsAndFiles->isChecked());

  c->writeEntry("KeepDirsFirst", 
		myKeepDirsFirst->isChecked());

  // Restore the old config group
  c->setGroup(oldgroup);
}

#include "kfiledialogconf.moc"
