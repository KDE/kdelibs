/* This file is part of the KDE libraries
 *  Copyright (C) 2005 Joseph Wenninger <jowenn@kde.org>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 */
#include <editorchooser.h>
#include "moc_editorchooser.cpp"

#include <QComboBox>
#include <QtCore/QStringList>
#include <QLabel>
#include <QLayout>

#include <kmimetypetrader.h>
#include <kconfig.h>
#include <klocale.h>
#include <kglobal.h>

#include "ui_editorchooser_ui.h"
#include <kconfiggroup.h>

using namespace KTextEditor;

namespace KTextEditor
{
  class PrivateEditorChooser
  {
  public:
    PrivateEditorChooser()
    {
    }
    ~PrivateEditorChooser(){}
    // Data Members
    Ui::EditorChooser *chooser;
    QStringList ElementNames;
    QStringList elements;
  };
}

EditorChooser::EditorChooser(QWidget *parent)
  : QWidget(parent)
{
  d = new PrivateEditorChooser ();

  d->chooser = new Ui::EditorChooser();
  d->chooser->setupUi(this);

  KService::List offers = KMimeTypeTrader::self()->query("text/plain", "KTextEditor/Document");
  KConfigGroup config = KSharedConfig::openConfig("default_components")->group("KTextEditor");
  QString editor = config.readPathEntry("embeddedEditor", QString());

  if (editor.isEmpty()) editor = "katepart";

  // search default component
  for (KService::List::Iterator it = offers.begin(); it != offers.end(); ++it)
  {
    if ((*it)->desktopEntryName().contains(editor))
    {
      d->chooser->editorCombo->addItem(i18n("System Default (currently: %1)", (*it)->name()));
      break;
    }
  }

  // add list of all available components
  for (KService::List::Iterator it = offers.begin(); it != offers.end(); ++it)
  {
    d->chooser->editorCombo->addItem((*it)->name());
    d->elements.append((*it)->desktopEntryName());
  }
  d->chooser->editorCombo->setCurrentIndex(0);

  connect(d->chooser->editorCombo,SIGNAL(activated(int)),this,SIGNAL(changed()));

  setMinimumSize(sizeHint());
}

EditorChooser:: ~EditorChooser()
{
  delete d->chooser;
  delete d;
}

void EditorChooser::readAppSetting(const QString& postfix)
{
  KConfigGroup cg(KGlobal::config(), "KTEXTEDITOR:" + postfix);
  QString editor = cg.readPathEntry("editor", QString());
  if (editor.isEmpty())
    d->chooser->editorCombo->setCurrentIndex(0);
  else
  {
    // + 1, because item 0 is the default one.
    int idx = d->elements.indexOf(editor) + 1;
    d->chooser->editorCombo->setCurrentIndex(idx);
  }
}

void EditorChooser::writeAppSetting(const QString& postfix)
{
  KConfigGroup cg(KGlobal::config(), "KTEXTEDITOR:" + postfix);
  cg.writeEntry("DEVELOPER_INFO","NEVER TRY TO USE VALUES FROM THAT GROUP, THEY ARE SUBJECT TO CHANGES");
  cg.writePathEntry("editor", (d->chooser->editorCombo->currentIndex()<=0) ? //< for broken installations, where editor list is empty
  QString() : QString(d->elements.at(d->chooser->editorCombo->currentIndex()-1)));
}

KTextEditor::Editor *EditorChooser::editor(const QString& postfix,
                                           bool fallBackToKatePart)
{
  // try to read the used library from the application's config
  KConfigGroup cg(KGlobal::config(), "KTEXTEDITOR:" + postfix);
  QString editor = cg.readPathEntry("editor", QString());
  if (editor.isEmpty())
  {
    // there is no library set in the application's config,
    // fall back to KDE's system default
    KConfig config("default_components");
    editor = config.group("KTextEditor").readPathEntry("embeddedEditor", "katepart");
  }

  KService::Ptr serv = KService::serviceByDesktopName(editor);
  if (serv)
  {
    KTextEditor::Editor *tmpEd = KTextEditor::editor(serv->library().toLatin1());
    if (tmpEd) return tmpEd;
  }
  if (fallBackToKatePart)
    return KTextEditor::editor("katepart");

  return 0;
}

// kate: space-indent on; indent-width 2; replace-tabs on;
