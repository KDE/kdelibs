#include "kxmlguifilemerger_p.h"
#include <QFile>
#include <QDomDocument>
#include "kxmlguifactory.h"
#include <kglobal.h>
#include <kstandarddirs.h>

struct DocStruct
{
    QString file;
    QString data;
};

typedef QMap<QString, QMap<QString, QString> > ActionPropertiesMap;

static ActionPropertiesMap extractActionProperties( const QDomDocument &doc )
{
  ActionPropertiesMap properties;

  QDomElement actionPropElement = doc.documentElement().namedItem( "ActionProperties" ).toElement();

  if ( actionPropElement.isNull() )
    return properties;

  QDomNode n = actionPropElement.firstChild();
  while(!n.isNull())
  {
    QDomElement e = n.toElement();
    n = n.nextSibling(); // Advance now so that we can safely delete e
    if ( e.isNull() )
      continue;

    if ( e.tagName().toLower() != "action" )
      continue;

    QString actionName = e.attribute( "name" );

    if ( actionName.isEmpty() )
      continue;

    QMap<QString, QMap<QString, QString> >::Iterator propIt = properties.find( actionName );
    if ( propIt == properties.end() )
      propIt = properties.insert( actionName, QMap<QString, QString>() );

    const QDomNamedNodeMap attributes = e.attributes();
    const uint attributeslength = attributes.length();

    for ( uint i = 0; i < attributeslength; ++i )
    {
      const QDomAttr attr = attributes.item( i ).toAttr();

      if ( attr.isNull() )
        continue;

      const QString name = attr.name();

      if ( name == "name" || name.isEmpty() )
        continue;

      (*propIt)[ name ] = attr.value();
    }

  }

  return properties;
}

static void storeActionProperties( QDomDocument &doc,
                            const ActionPropertiesMap &properties )
{
  QDomElement actionPropElement = doc.documentElement().namedItem( "ActionProperties" ).toElement();

  if ( actionPropElement.isNull() )
  {
    actionPropElement = doc.createElement( "ActionProperties" );
    doc.documentElement().appendChild( actionPropElement );
  }

  while ( !actionPropElement.firstChild().isNull() )
    actionPropElement.removeChild( actionPropElement.firstChild() );

  ActionPropertiesMap::ConstIterator it = properties.begin();
  ActionPropertiesMap::ConstIterator end = properties.end();
  for (; it != end; ++it )
  {
    QDomElement action = doc.createElement( "Action" );
    action.setAttribute( "name", it.key() );
    actionPropElement.appendChild( action );

    QMap<QString, QString> attributes = (*it);
    QMap<QString, QString>::ConstIterator attrIt = attributes.begin();
    QMap<QString, QString>::ConstIterator attrEnd = attributes.end();
    for (; attrIt != attrEnd; ++attrIt )
      action.setAttribute( attrIt.key(), attrIt.value() );
  }
}

static QString findVersionNumber( const QString &xml )
{
  enum { ST_START, ST_AFTER_OPEN, ST_AFTER_GUI,
               ST_EXPECT_VERSION, ST_VERSION_NUM} state = ST_START;
  for (int pos = 0; pos < xml.length(); pos++)
  {
    switch (state)
    {
      case ST_START:
        if (xml[pos] == '<')
          state = ST_AFTER_OPEN;
        break;
      case ST_AFTER_OPEN:
      {
        //Jump to gui..
        int guipos = xml.indexOf("gui", pos, Qt::CaseInsensitive/*case-insensitive*/);
        if (guipos == -1)
          return QString(); //Reject

        pos = guipos + 2; //Position at i, so we're moved ahead to the next character by the ++;
        state = ST_AFTER_GUI;
        break;
      }
      case ST_AFTER_GUI:
        state = ST_EXPECT_VERSION;
        break;
      case ST_EXPECT_VERSION:
      {
        int verpos =  xml.indexOf("version=\"", pos, Qt::CaseInsensitive /*case-insensitive*/);
        if (verpos == -1)
          return QString(); //Reject

        pos = verpos +  8; //v = 0, e = +1, r = +2, s = +3 , i = +4, o = +5, n = +6, = = +7, " = + 8
        state = ST_VERSION_NUM;
        break;
      }
      case ST_VERSION_NUM:
      {
        int endpos;
        for (endpos = pos; endpos <  xml.length(); endpos++)
        {
          if (xml[endpos].unicode() >= '0' && xml[endpos].unicode() <= '9')
            continue; //Number..
          if (xml[endpos].unicode() == '"') //End of parameter
            break;
          else //This shouldn't be here..
          {
            endpos = xml.length();
          }
        }

        if (endpos != pos && endpos < xml.length() )
        {
          QString matchCandidate = xml.mid(pos, endpos - pos); //Don't include " ".
          return matchCandidate;
        }

        state = ST_EXPECT_VERSION; //Try to match a well-formed version..
        break;
      } //case..
    } //switch
  } //for

  return QString();
}


KXmlGuiFileMerger::KXmlGuiFileMerger(const QStringList& files)
{
    QList<DocStruct> allDocuments;

    foreach (const QString &file, files) {
        DocStruct d;
        d.file = file;
        d.data = KXMLGUIFactory::readConfigFile( file );
        allDocuments.append( d );
    }

    QList<DocStruct>::const_iterator best = allDocuments.end();
    uint bestVersion = 0;

    QList<DocStruct>::const_iterator docIt = allDocuments.begin();
    const QList<DocStruct>::const_iterator docEnd = allDocuments.end();
    for (; docIt != docEnd; ++docIt ) {
        const QString versionStr = findVersionNumber( (*docIt).data );
        if ( versionStr.isEmpty() )
            continue;

        bool ok = false;
        uint version = versionStr.toUInt( &ok );
        if ( !ok )
            continue;
        //kDebug() << "FOUND VERSION " << version;

        if ( version > bestVersion ) {
            best = docIt;
            //kDebug() << "best version is now " << version;
            bestVersion = version;
        }
    }

    if ( best != docEnd ) {
        if ( best != allDocuments.begin() ) {
            QList<DocStruct>::iterator local = allDocuments.begin();

            if ( (*local).file.startsWith(KGlobal::dirs()->localkdedir()) ) {
                // load the local document and extract the action properties
                QDomDocument document;
                document.setContent( (*local).data );

                const ActionPropertiesMap properties = extractActionProperties( document );

                // in case the document has a ActionProperties section
                // we must not delete it but copy over the global doc
                // to the local and insert the ActionProperties section
                if ( !properties.isEmpty() ) {
                    // now load the global one with the higher version number
                    // into memory
                    document.setContent( (*best).data );
                    // and store the properties in there
                    storeActionProperties( document, properties );

                    (*local).data = document.toString();
                    // make sure we pick up the new local doc, when we return later
                    best = local;

                    // write out the new version of the local document
                    QFile f( (*local).file );
                    if ( f.open( QIODevice::WriteOnly ) )
                    {
                        const QByteArray utf8data = (*local).data.toUtf8();
                        f.write( utf8data.constData(), utf8data.length() );
                        f.close();
                    }
                } else {
                    const QString f = (*local).file;
                    const QString backup = f + QLatin1String( ".backup" );
                    QFile::rename( f, backup );
                }
            }
        }
        m_doc = (*best).data;
        m_file = (*best).file;
    } else if ( !files.isEmpty() ) {
        //kDebug() << "returning first one...";
        m_doc = (*allDocuments.begin()).data;
        m_file = (*allDocuments.begin()).file;
    }
}
