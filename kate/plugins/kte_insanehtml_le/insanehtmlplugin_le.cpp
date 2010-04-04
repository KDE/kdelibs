/* 
Copyright (C) 2010 Joseph Wenninger <jowenn@kde.org>

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:

1. Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "insanehtmlplugin_le.h"
#include "insanehtmlplugin_le.moc"
#include <kgenericfactory.h>
#include <kaction.h>
#include <kactioncollection.h>

K_PLUGIN_FACTORY_DECLARATION(InsaneHTMLPluginLEFactory)
K_PLUGIN_FACTORY_DEFINITION(InsaneHTMLPluginLEFactory,
        registerPlugin<InsaneHTMLPluginLE>();
        )
K_EXPORT_PLUGIN(InsaneHTMLPluginLEFactory("ktexteditor_insanehtml_le", "ktexteditor_plugins"))

InsaneHTMLPluginLE::InsaneHTMLPluginLE(QObject *parent, const QList<QVariant> data):
  KTextEditor::Plugin(parent) {
    Q_UNUSED(data);

}


void InsaneHTMLPluginLE::addView (KTextEditor::View *view) {
  m_map.insert(view,new InsaneHTMLPluginLEView(this,view));
}

void InsaneHTMLPluginLE::removeView (KTextEditor::View *view) {
  delete m_map.take(view);
}


InsaneHTMLPluginLEView::InsaneHTMLPluginLEView(QObject* parent,KTextEditor::View* view):
  QObject(parent),KXMLGUIClient(),m_view(view) {
    
    setComponentData(InsaneHTMLPluginLEFactory::componentData());

    KAction *a=actionCollection()->addAction( "tools_insanehtml_le", this,SLOT(complete()) );
    a->setText(i18n("Insane HTML (LE) Expansion"));
    a->setShortcut( Qt::CTRL + Qt::Key_Period );
             
    setXMLFile( "insanehtml_le_ui.rc" );
    
    m_view->insertChildClient(this);
}

InsaneHTMLPluginLEView::~InsaneHTMLPluginLEView() {
  m_view->removeChildClient(this);
}