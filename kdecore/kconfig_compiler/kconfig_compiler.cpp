// -*- Mode: C++; c-basic-offset: 2; indent-tabs-mode: nil; -*-
/*
    This file is part of KDE.

    Copyright (c) 2003 Cornelius Schumacher <schumacher@kde.org>
    Copyright (c) 2003 Waldo Bastian <bastian@kde.org>
    Copyright (c) 2003 Zack Rusin <zack@kde.org>

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

#include <qfile.h>
#include <qtextstream.h>
#include <qdom.h>
#include <qregexp.h>

#include <kaboutdata.h>
#include <kapplication.h>
#include <kdebug.h>
#include <klocale.h>
#include <kcmdlineargs.h>
#include <kglobal.h>
#include <kconfig.h>
#include <ksimpleconfig.h>
#include <kstandarddirs.h>

#include <iostream>

static const KCmdLineOptions options[] =
{
  { "d", 0, 0 },
  { "directory <dir>", I18N_NOOP("Directory to generate files in"), "." },
  { "+file.kcfg", I18N_NOOP("Input kcfg XML file"), 0 },
  { "+file.kcfgc", I18N_NOOP("Code generation options file"), 0 },
  KCmdLineLastOption
};


bool globalEnums;
bool itemAccessors;

class CfgEntry
{
  public:
    struct Choice
    {
      QString name;
      QString label;
      QString whatsThis;
    };

    CfgEntry( const QString &group, const QString &type, const QString &key,
              const QString &name, const QString &label,
              const QString &whatsThis, const QString &code,
              const QString &defaultValue, const QValueList<Choice> &choices,
              bool hidden )
      : mGroup( group ), mType( type ), mKey( key ), mName( name ),
        mLabel( label ), mWhatsThis( whatsThis ), mCode( code ),
        mDefaultValue( defaultValue ),
        mChoices( choices ), mHidden( hidden )
    {
    }

    void setGroup( const QString &group ) { mGroup = group; }
    QString group() const { return mGroup; }

    void setType( const QString &type ) { mType = type; }
    QString type() const { return mType; }

    void setKey( const QString &key ) { mKey = key; }
    QString key() const { return mKey; }

    void setName( const QString &name ) { mName = name; }
    QString name() const { return mName; }

    void setLabel( const QString &label ) { mLabel = label; }
    QString label() const { return mLabel; }

    void setWhatsThis( const QString &whatsThis ) { mWhatsThis = whatsThis; }
    QString whatsThis() const { return mWhatsThis; }

    void setDefaultValue( const QString &d ) { mDefaultValue = d; }
    QString defaultValue() const { return mDefaultValue; }

    void setCode( const QString &d ) { mCode = d; }
    QString code() const { return mCode; }

    void setMinValue( const QString &d ) { mMin = d; }
    QString minValue() const { return mMin; }

    void setMaxValue( const QString &d ) { mMax = d; }
    QString maxValue() const { return mMax; }

    void setParam( const QString &d ) { mParam = d; }
    QString param() const { return mParam; }

    void setParamName( const QString &d ) { mParamName = d; }
    QString paramName() const { return mParamName; }

    void setParamType( const QString &d ) { mParamType = d; }
    QString paramType() const { return mParamType; }

    void setChoices( const QValueList<Choice> &d ) { mChoices = d; }
    QValueList<Choice> choices() const { return mChoices; }

    void setParamValues( const QStringList &d ) { mParamValues = d; }
    QStringList paramValues() const { return mParamValues; }

    void setParamDefaultValues( const QStringList &d ) { mParamDefaultValues = d; }
    QString paramDefaultValue(int i) const { return mParamDefaultValues[i]; }

    void setParamMax( int d ) { mParamMax = d; }
    int paramMax() const { return mParamMax; }

    bool hidden() const { return mHidden; }

    void dump() const
    {
      kdDebug() << "<entry>" << endl;
      kdDebug() << "  group: " << mGroup << endl;
      kdDebug() << "  type: " << mType << endl;
      kdDebug() << "  key: " << mKey << endl;
      kdDebug() << "  name: " << mName << endl;
      kdDebug() << "  label: " << mLabel << endl;
// whatsthis
      kdDebug() << "  code: " << mCode << endl;
//      kdDebug() << "  values: " << mValues.join(":") << endl;
      
      if (!param().isEmpty())
      {
        kdDebug() << "  param name: "<< mParamName << endl;
        kdDebug() << "  param type: "<< mParamType << endl;
        kdDebug() << "  paramvalues: " << mParamValues.join(":") << endl;
      }
      kdDebug() << "  default: " << mDefaultValue << endl;
      kdDebug() << "  hidden: " << mHidden << endl;
      kdDebug() << "  min: " << mMin << endl;
      kdDebug() << "  max: " << mMax << endl;
      kdDebug() << "</entry>" << endl;
    }

  private:
    QString mGroup;
    QString mType;
    QString mKey;
    QString mName;
    QString mLabel;
    QString mWhatsThis;
    QString mCode;
    QString mDefaultValue;
    QString mParam;
    QString mParamName;
    QString mParamType;
    QValueList<Choice> mChoices;
    QStringList mParamValues;
    QStringList mParamDefaultValues;
    int mParamMax;
    bool mHidden;
    QString mMin;
    QString mMax;
};


static QString varName(const QString &n)
{
  QString result = "m"+n;
  result[1] = result[1].upper();
  return result;
}

static QString enumName(const QString &n)
{
  QString result = "Enum"+n;
  result[4] = result[4].upper();
  return result;
}

static QString setFunction(const QString &n)
{
  QString result = "set"+n;
  result[3] = result[3].upper();
  return result;
}


static QString getFunction(const QString &n)
{
  QString result = n;
  result[0] = result[0].lower();
  return result;
}


static void addQuotes( QString &s )
{
  if ( s.left( 1 ) != "\"" ) s.prepend( "\"" );
  if ( s.right( 1 ) != "\"" ) s.append( "\"" );
}

static QString quoteString( const QString &s )
{
  QString r = s;
  r.replace( "\\", "\\\\" );
  r.replace( "\"", "\\\"" );
  r.replace( "\r", "" );
  r.replace( "\n", "\\n\"\n\"" );
  return "\"" + r + "\"";
}

static QString dumpNode(const QDomNode &node)
{
  QString msg;
  QTextStream s(&msg, IO_WriteOnly );
  node.save(s, 0);

  msg = msg.simplifyWhiteSpace();
  if (msg.length() > 40)
    return msg.left(37)+"...";
  return msg;
}

static QString filenameOnly(QString path)
{
   int i = path.findRev('/');
   if (i >= 0)
      return path.mid(i+1);
   return path;
}

static void preProcessDefault( QString &defaultValue, const QString &name,
                               const QString &type,
                               const QValueList<CfgEntry::Choice> &choices,
                               QString &code )
{
    if ( type == "String" && !defaultValue.isEmpty() ) {
      addQuotes( defaultValue );

    } else if ( type == "Path" && !defaultValue.isEmpty() ) {
      addQuotes( defaultValue );

    } else if ( type == "StringList" && !defaultValue.isEmpty() ) {
      QTextStream cpp( &code, IO_WriteOnly | IO_Append );
      if (!code.isEmpty())
         cpp << endl;

      cpp << "  QStringList default" << name << ";" << endl;
      QStringList defaults = QStringList::split( ",", defaultValue );
      QStringList::ConstIterator it;
      for( it = defaults.begin(); it != defaults.end(); ++it ) {
        cpp << "  default" << name << ".append( QString::fromUtf8( \"" << *it << "\" ) );"
            << endl;
      }
      defaultValue = "default" + name;

    } else if ( type == "Color" && !defaultValue.isEmpty() ) {
      QRegExp colorRe("\\d+,\\s*\\d+,\\s*\\d+");
      if (colorRe.exactMatch(defaultValue))
      {
        defaultValue = "QColor( " + defaultValue + " )";
      }
      else
      {
        defaultValue = "QColor( \"" + defaultValue + "\" )";
      }

    } else if ( type == "Enum" ) {
      if ( !globalEnums ) {
        QValueList<CfgEntry::Choice>::ConstIterator it;
        for( it = choices.begin(); it != choices.end(); ++it ) {
          if ( (*it).name == defaultValue ) {
            defaultValue.prepend( enumName(name) + "::");
            break;
          }
        }
      }

    } else if ( type == "IntList" ) {
      QTextStream cpp( &code, IO_WriteOnly | IO_Append );
      if (!code.isEmpty())
         cpp << endl;

      cpp << "  QValueList<int> default" << name << ";" << endl;
      QStringList defaults = QStringList::split( ",", defaultValue );
      QStringList::ConstIterator it;
      for( it = defaults.begin(); it != defaults.end(); ++it ) {
        cpp << "  default" << name << ".append( " << *it << " );"
            << endl;
      }
      defaultValue = "default" + name;
    }
}


CfgEntry *parseEntry( const QString &group, const QDomElement &element )
{
  bool defaultCode = false;
  QString type = element.attribute( "type" );
  QString name = element.attribute( "name" );
  QString key = element.attribute( "key" );
  QString hidden = element.attribute( "hidden" );
  QString label;
  QString whatsThis;
  QString defaultValue;
  QString code;
  QString param;
  QString paramName;
  QString paramType;
  QValueList<CfgEntry::Choice> choices;
  QStringList paramValues;
  QStringList paramDefaultValues;
  QString minValue;
  QString maxValue;
  int paramMax = 0;

  QDomNode n;
  for ( n = element.firstChild(); !n.isNull(); n = n.nextSibling() ) {
    QDomElement e = n.toElement();
    QString tag = e.tagName();
    if ( tag == "label" ) label = e.text();
    else if ( tag == "whatsthis" ) whatsThis = e.text();
    else if ( tag == "min" ) minValue = e.text();
    else if ( tag == "max" ) maxValue = e.text();
    else if ( tag == "code" ) code = e.text();
    else if ( tag == "parameter" )
    {
      param = e.attribute( "name" );
      paramType = e.attribute( "type" );
      if ( param.isEmpty() ) {
        kdError() << "Parameter must have a name: " << dumpNode(e) << endl;
        return 0;
      }
      if ( paramType.isEmpty() ) {
        kdError() << "Parameter must have a type: " << dumpNode(e) << endl;
        return 0;
      }
      if ((paramType == "Int") || (paramType == "UInt"))
      {
         bool ok;
         paramMax = e.attribute("max").toInt(&ok);
         if (!ok)
         {
           kdError() << "Integer parameter must have a maximum (e.g. max=\"0\"): " << dumpNode(e) << endl;
           return 0;
         }
      }
      else if (paramType == "Enum")
      {
         QDomNode n2;
         for ( n2 = e.firstChild(); !n2.isNull(); n2 = n2.nextSibling() ) {
           QDomElement e2 = n2.toElement();
           if (e2.tagName() == "values")
           {
             QDomNode n3;
             for ( n3 = e2.firstChild(); !n3.isNull(); n3 = n3.nextSibling() ) {
               QDomElement e3 = n3.toElement();
               if (e3.tagName() == "value")
               {
                  paramValues.append( e3.text() );
               }
             }
             break;
           }
         }
         if (paramValues.isEmpty())
         {
           kdError() << "No values specified for parameter '" << param << "'." << endl;
           return 0;
         }
         paramMax = paramValues.count()-1;
      }
      else
      {
        kdError() << "Parameter '" << param << "' has type " << paramType << " but must be of type int, uint or Enum." << endl;
        return 0;
      }
    }
    else if ( tag == "default" )
    {
      if (e.attribute("param").isEmpty())
      {
        defaultValue = e.text();
        if (e.attribute( "code" ) == "true")
          defaultCode = true;
      }
    }
    else if ( tag == "choices" ) {
      QDomNode n2;
      for( n2 = e.firstChild(); !n2.isNull(); n2 = n2.nextSibling() ) {
        QDomElement e2 = n2.toElement();
        if ( e2.tagName() == "choice" ) {
          QDomNode n3;
          CfgEntry::Choice choice;
          choice.name = e2.attribute( "name" );
          if ( choice.name.isEmpty() ) {
            kdError() << "Tag <choice> requires attribute 'name'." << endl;
          }
          for( n3 = e2.firstChild(); !n3.isNull(); n3 = n3.nextSibling() ) {
            QDomElement e3 = n3.toElement();
            if ( e3.tagName() == "label" ) choice.label = e3.text();
            if ( e3.tagName() == "whatsthis" ) choice.whatsThis = e3.text();
          }
          choices.append( choice );
        }
      }
    }
  }

  if ( name.isEmpty() && key.isEmpty() ) {
    kdError() << "Entry must have a name or a key: " << dumpNode(element) << endl;
    return 0;
  }

  if ( key.isEmpty() ) {
    key = name;
  }

  if ( name.isEmpty() ) {
    name = key;
    name.replace( " ", QString::null );
  } else if ( name.contains( ' ' ) ) {
    kdWarning()<<"Entry '"<<name<<"' contains spaces! <name> elements can't contain speces!"<<endl;
    name.remove( ' ' );
  }

  if (name.contains("$("))
  {
    if (param.isEmpty())
    {
      kdError() << "Name may not be parameterized: " << name << endl;
      return 0;
    }
  }
  else
  {
    if (!param.isEmpty())
    {
      kdError() << "Name must contain '$(" << param << ")': " << name << endl;
      return 0;
    }
  }

  if ( label.isEmpty() ) {
    label = key;
  }

  if ( type.isEmpty() ) type = "String"; // XXX : implicit type might be bad

  if (!param.isEmpty())
  {
    // Adjust name
    paramName = name;
    name.replace("$("+param+")", QString::null);
    // Lookup defaults for indexed entries
    for(int i = 0; i <= paramMax; i++)
    {
      paramDefaultValues.append(QString::null);
    }

    QDomNode n;
    for ( n = element.firstChild(); !n.isNull(); n = n.nextSibling() ) {
      QDomElement e = n.toElement();
      QString tag = e.tagName();
      if ( tag == "default" )
      {
        QString index = e.attribute("param");
        if (index.isEmpty())
           continue;

        bool ok;
        int i = index.toInt(&ok);
        if (!ok)
        {
          i = paramValues.findIndex(index);
          if (i == -1)
          {
            kdError() << "Index '" << index << "' for default value is unknown." << endl;
            return 0;
          }
        }

        if ((i < 0) || (i > paramMax))
        {
          kdError() << "Index '" << i << "' for default value is out of range [0, "<< paramMax<<"]." << endl;
          return 0;
        }

        QString tmpDefaultValue = e.text();

        if (e.attribute( "code" ) != "true")
           preProcessDefault(tmpDefaultValue, name, type, choices, code);

        paramDefaultValues[i] = tmpDefaultValue;
      }
    }
  }

  if (!defaultCode)
  {
    preProcessDefault(defaultValue, name, type, choices, code);
  }

  CfgEntry *result = new CfgEntry( group, type, key, name, label, whatsThis,
                                   code, defaultValue, choices,
                                   hidden == "true" );
  if (!param.isEmpty())
  {
    result->setParam(param);
    result->setParamName(paramName);
    result->setParamType(paramType);
    result->setParamValues(paramValues);
    result->setParamDefaultValues(paramDefaultValues);
    result->setParamMax(paramMax);
  }
  result->setMinValue(minValue);
  result->setMaxValue(maxValue);

  return result;
}

/**
  Return parameter declaration for given type.
*/
QString param( const QString &type )
{
    if ( type == "String" )           return "const QString &";
    else if ( type == "StringList" )  return "const QStringList &";
    else if ( type == "Font" )        return "const QFont &";
    else if ( type == "Rect" )        return "const QRect &";
    else if ( type == "Size" )        return "const QSize &";
    else if ( type == "Color" )       return "const QColor &";
    else if ( type == "Point" )       return "const QPoint &";
    else if ( type == "Int" )         return "int";
    else if ( type == "UInt" )        return "uint";
    else if ( type == "Bool" )        return "bool";
    else if ( type == "Double" )      return "double";
    else if ( type == "DateTime" )    return "const QDateTime &";
    else if ( type == "Int64" )       return "Q_INT64";
    else if ( type == "UInt64" )      return "Q_UINT64";
    else if ( type == "IntList" )     return "const QValueList<int> &";
    else if ( type == "Enum" )        return "int";
    else if ( type == "Path" )        return "const QString &";
    else if ( type == "Password" )    return "const QString &";
    else {
        kdError() <<"kconfig_compiler does not support type \""<< type <<"\""<<endl;
        return "QString"; //For now, but an assert would be better
    }
}

/**
  Actual C++ storage type for given type.
*/
QString cppType( const QString &type )
{
    if ( type == "String" )           return "QString";
    else if ( type == "StringList" )  return "QStringList";
    else if ( type == "Font" )        return "QFont";
    else if ( type == "Rect" )        return "QRect";
    else if ( type == "Size" )        return "QSize";
    else if ( type == "Color" )       return "QColor";
    else if ( type == "Point" )       return "QPoint";
    else if ( type == "Int" )         return "int";
    else if ( type == "UInt" )        return "uint";
    else if ( type == "Bool" )        return "bool";
    else if ( type == "Double" )      return "double";
    else if ( type == "DateTime" )    return "QDateTime";
    else if ( type == "Int64" )       return "Q_INT64";
    else if ( type == "UInt64" )      return "Q_UINT64";
    else if ( type == "IntList" )     return "QValueList<int>";
    else if ( type == "Enum" )        return "int";
    else if ( type == "Path" )        return "QString";
    else if ( type == "Password" )    return "QString";
    else {
        kdError()<<"kconfig_compiler does not support type \""<< type <<"\""<<endl;
        return "QString"; //For now, but an assert would be better
    }
}

QString defaultValue( const QString &type )
{
    if ( type == "String" )           return "\"\""; // Use empty string, not null string!
    else if ( type == "StringList" )  return "QStringList()";
    else if ( type == "Font" )        return "KGlobalSettings::generalFont()";
    else if ( type == "Rect" )        return "QRect()";
    else if ( type == "Size" )        return "QSize()";
    else if ( type == "Color" )       return "QColor(128, 128, 128)";
    else if ( type == "Point" )       return "QPoint()";
    else if ( type == "Int" )         return "0";
    else if ( type == "UInt" )        return "0";
    else if ( type == "Bool" )        return "false";
    else if ( type == "Double" )      return "0.0";
    else if ( type == "DateTime" )    return "QDateTime()";
    else if ( type == "Int64" )       return "0";
    else if ( type == "UInt64" )      return "0";
    else if ( type == "IntList" )     return "QValueList<int>()";
    else if ( type == "Enum" )        return "0";
    else if ( type == "Path" )        return "\"\""; // Use empty string, not null string!
    else if ( type == "Password" )    return "\"\""; // Use empty string, not null string!
    else {
        kdWarning()<<"Error, kconfig_compiler doesn't support the \""<< type <<"\" type!"<<endl;
        return "QString"; //For now, but an assert would be better
    }
}

QString itemType( const QString &type )
{
  QString t;

  t = type;
  t.replace( 0, 1, t.left( 1 ).upper() );

  return t;
}

static QString itemDeclaration(const CfgEntry *e)
{
  if (itemAccessors)
     return QString::null;

  return "  KConfigSkeleton::Item"+itemType( e->type() ) +
         "  *item" + e->name() + 
         ( (!e->param().isEmpty())?(QString("[%1]").arg(e->paramMax()+1)) : QString::null) + 
         ";\n";
}

static QString itemVar(const CfgEntry *e)
{
  if (itemAccessors)
     return varName( e->name() ) + "Item";

  return "item" + e->name();

}

QString newItem( const QString &type, const QString &name, const QString &key,
                 const QString &defaultValue, const QString &param = QString::null)
{
  QString t = "new KConfigSkeleton::Item" + itemType( type ) +
              "( currentGroup(), " + key + ", " + varName( name ) + param;
  if ( type == "Enum" ) t += ", values" + name;
  if ( !defaultValue.isEmpty() ) {
    t += ", ";
    if ( type == "String" ) t += defaultValue;
    else t+= defaultValue;
  }
  t += " );";

  return t;
}

QString paramString(const QString &s, const CfgEntry *e, int i)
{
  QString result = s;
  QString needle = "$("+e->param()+")";
  if (result.contains(needle))
  {
    QString tmp;
    if (e->paramType() == "Enum")
    {
      tmp = e->paramValues()[i];
    }
    else
    {
      tmp = QString::number(i);
    }

    result.replace(needle, tmp);
  }
  return result;
}

QString paramString(const QString &group, const QStringList &parameters)
{
  QString paramString = group;
  QString arguments;
  int i = 1;
  for( QStringList::ConstIterator it = parameters.begin();
       it != parameters.end(); ++it)
  {
     if (paramString.contains("$("+*it+")"))
     {
       QString tmp;
       tmp.sprintf("%%%d", i++);
       paramString.replace("$("+*it+")", tmp);
       arguments += ".arg( mParam"+*it+" )";
     }
  }
  if (arguments.isEmpty())
    return "QString::fromLatin1( \""+group+"\" )";

  return "QString::fromLatin1( \""+paramString+"\" )"+arguments;
}

/* int i is the value of the parameter */
QString userTextsFunctions( CfgEntry *e, QString itemVarStr=QString::null, QString i=QString::null )
{
  QString txt;
  if (itemVarStr.isNull()) itemVarStr=itemVar(e);
  if ( !e->label().isEmpty() ) {
    txt += "  " + itemVarStr + "->setLabel( i18n(";
    if ( !e->param().isEmpty() )
      txt += quoteString(e->label().replace("$("+e->param()+")", i));
    else 
      txt+= quoteString(e->label());
    txt+= ") );\n";
  }
  if ( !e->whatsThis().isEmpty() ) {
    txt += "  " + itemVarStr + "->setWhatsThis( i18n(";
    if ( !e->param().isEmpty() )
      txt += quoteString(e->whatsThis().replace("$("+e->param()+")", i));
    else 
      txt+= quoteString(e->whatsThis());
    txt+=") );\n";
  }
  return txt;
}

int main( int argc, char **argv )
{
  KAboutData aboutData( "kconfig_compiler", I18N_NOOP("KDE .kcfg compiler"), "0.3",
    I18N_NOOP("KConfig Compiler") , KAboutData::License_LGPL );
  aboutData.addAuthor( "Cornelius Schumacher", 0, "schumacher@kde.org" );
  aboutData.addAuthor( "Waldo Bastian", 0, "bastian@kde.org" );
  aboutData.addAuthor( "Zack Rusin", 0, "zack@kde.org" );
  aboutData.addCredit( "Reinhold Kainhofer", "Fix for parametrized entries", 
      "reinhold@kainhofer.com", "http://reinhold.kainhofer.com" );

  KCmdLineArgs::init( argc, argv, &aboutData );
  KCmdLineArgs::addCmdLineOptions( options );

  KInstance app( &aboutData );

  KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

  if ( args->count() < 2 ) {
    kdError() << "Too few arguments." << endl;
    return 1;
  }
  if ( args->count() > 2 ) {
    kdError() << "Too many arguments." << endl;
    return 1;
  }

  QString baseDir = QFile::decodeName(args->getOption("directory"));
  if (!baseDir.endsWith("/"))
    baseDir.append("/");

  QString inputFilename = args->url( 0 ).path();
  QString codegenFilename = args->url( 1 ).path();

  if (!codegenFilename.endsWith(".kcfgc"))
  {
    kdError() << "Codegen options file must have extension .kcfgc" << endl;
    return 1;
  }
  QString baseName = args->url( 1 ).fileName();
  baseName = baseName.left(baseName.length() - 6);

  KSimpleConfig codegenConfig( codegenFilename, true );

  QString nameSpace = codegenConfig.readEntry("NameSpace");
  QString className = codegenConfig.readEntry("ClassName");
  QString inherits = codegenConfig.readEntry("Inherits");
  bool singleton = codegenConfig.readBoolEntry("Singleton", false);
  bool staticAccessors = singleton;
  bool customAddons = codegenConfig.readBoolEntry("CustomAdditions");
  QString memberVariables = codegenConfig.readEntry("MemberVariables");
  QStringList headerIncludes = codegenConfig.readListEntry("IncludeFiles");
  QStringList mutators = codegenConfig.readListEntry("Mutators");
  bool allMutators = false;
  if ((mutators.count() == 1) && (mutators[0].lower() == "true"))
     allMutators = true;
  itemAccessors = codegenConfig.readBoolEntry( "ItemAccessors", false );
  bool setUserTexts = codegenConfig.readBoolEntry( "SetUserTexts", false );

  globalEnums = codegenConfig.readBoolEntry( "GlobalEnums", false );

  QFile input( inputFilename );

  QDomDocument doc;
  QString errorMsg;
  int errorRow;
  int errorCol;
  if ( !doc.setContent( &input, &errorMsg, &errorRow, &errorCol ) ) {
    kdError() << "Unable to load document." << endl;
    kdError() << "Parse error in " << args->url( 0 ).fileName() << ", line " << errorRow << ", col " << errorCol << ": " << errorMsg << endl;
    return 1;
  }

  QDomElement cfgElement = doc.documentElement();

  if ( cfgElement.isNull() ) {
    kdError() << "No document in kcfg file" << endl;
    return 1;
  }

  QString cfgFileName;
  bool cfgFileNameArg = false;
  QStringList parameters;
  QStringList includes;

  QPtrList<CfgEntry> entries;
  entries.setAutoDelete( true );

  QDomNode n;
  for ( n = cfgElement.firstChild(); !n.isNull(); n = n.nextSibling() ) {
    QDomElement e = n.toElement();

    QString tag = e.tagName();

    if ( tag == "include" ) {
      QString includeFile = e.text();
      if (!includeFile.isEmpty())
        includes.append(includeFile);

    } else if ( tag == "kcfgfile" ) {
      cfgFileName = e.attribute( "name" );
      cfgFileNameArg = e.attribute( "arg" ).lower() == "true";
      QDomNode n2;
      for( n2 = e.firstChild(); !n2.isNull(); n2 = n2.nextSibling() ) {
        QDomElement e2 = n2.toElement();
        if ( e2.tagName() == "parameter" ) {
          parameters.append( e2.attribute( "name" ) );
        }
      }

    } else if ( tag == "group" ) {
      QString group = e.attribute( "name" );
      if ( group.isEmpty() ) {
        kdError() << "Group without name" << endl;
        return 1;
      }
      QDomNode n2;
      for( n2 = e.firstChild(); !n2.isNull(); n2 = n2.nextSibling() ) {
        QDomElement e2 = n2.toElement();
        if ( e2.tagName() != "entry" ) continue;
        CfgEntry *entry = parseEntry( group, e2 );
        if ( entry ) entries.append( entry );
        else {
          kdError() << "Can't parse entry." << endl;
          return 1;
        }
      }
    }
  }

  if ( inherits.isEmpty() ) inherits = "KConfigSkeleton";

  if ( className.isEmpty() ) {
    kdError() << "Class name missing" << endl;
    return 1;
  }

  if ( singleton && !parameters.isEmpty() ) {
    kdError() << "Singleton class can not have parameters" << endl;
    return 1;
  }

  if ( singleton && cfgFileNameArg)
  {
    kdError() << "Singleton class can not use filename as argument." << endl;
    return 1;
  }

  if ( !cfgFileName.isEmpty() && cfgFileNameArg)
  {
    kdError() << "Having both a fixed filename and a filename as argument is not possible." << endl;
    return 1;
  }

  if ( entries.isEmpty() ) {
    kdWarning() << "No entries." << endl;
  }

#if 0
  CfgEntry *cfg;
  for( cfg = entries.first(); cfg; cfg = entries.next() ) {
    cfg->dump();
  }
#endif

  QString headerFileName = baseName + ".h";
  QString implementationFileName = baseName + ".cpp";
  QString cppPreamble; // code to be inserted at the beginnin of the cpp file, e.g. initialization of static values

  QFile header( baseDir + headerFileName );
  if ( !header.open( IO_WriteOnly ) ) {
    kdError() << "Can't open '" << headerFileName << "for writing." << endl;
    return 1;
  }

  QTextStream h( &header );

  h << "// This file is generated by kconfig_compiler from " << args->url(0).fileName() << "." << endl;
  h << "// All changes you do to this file will be lost." << endl;

  h << "#ifndef " << ( !nameSpace.isEmpty() ? nameSpace.upper() + "_" : "" )
    << className.upper() << "_H" << endl;
  h << "#define " << ( !nameSpace.isEmpty() ? nameSpace.upper() + "_" : "" )
    << className.upper() << "_H" << endl << endl;

  // Includes
  QStringList::ConstIterator it;
  for( it = headerIncludes.begin(); it != headerIncludes.end(); ++it ) {
    h << "#include <" << *it << ">" << endl;
  }

  if ( headerIncludes.count() > 0 ) h << endl;

  h << "#include <kconfigskeleton.h>" << endl << endl;

  if ( !nameSpace.isEmpty() )
    h << "namespace " << nameSpace << " {" << endl << endl;

  // Class declaration header
  h << "class " << className << " : public " << inherits << endl;
  h << "{" << endl;
  h << "  public:" << endl;

  // enums
  CfgEntry *e;
  for( e = entries.first(); e; e = entries.next() ) {
    QValueList<CfgEntry::Choice> choices = e->choices();
    if ( !choices.isEmpty() ) {
      QStringList values;
      QValueList<CfgEntry::Choice>::ConstIterator itChoice;
      for( itChoice = choices.begin(); itChoice != choices.end(); ++itChoice ) {
        values.append( (*itChoice).name );
      }
      if ( globalEnums ) {
        h << "    enum { " << values.join( ", " ) << " };" << endl;
      } else {
        h << "    class " << enumName(e->name()) << endl;
        h << "    {" << endl;
        h << "      public:" << endl;
        h << "      enum { " << values.join( ", " ) << ", COUNT };" << endl;
        h << "    };" << endl;
      }
    }
    QStringList values = e->paramValues();
    if ( !values.isEmpty() ) {
      if ( globalEnums ) {
        h << "    enum { " << values.join( ", " ) << " };" << endl;
        h << "    static const char* const " << enumName(e->param()) << "ToString[];" << endl;
        cppPreamble += "const char* const " + className + "::" + enumName(e->param()) + "ToString[] = " +
            "{ \"" + values.join( "\", \"" ) + "\" };\n";
      } else {
        h << "    class " << enumName(e->param()) << endl;
        h << "    {" << endl;
        h << "      public:" << endl;
        h << "      enum { " << values.join( ", " ) << ", COUNT };" << endl;
        h << "      static const char* const enumToString[];" << endl;
        h << "    };" << endl;
        cppPreamble += "const char* const " + className + "::" + enumName(e->param()) + "::enumToString[] = " +
            "{ \"" + values.join( "\", \"" ) + "\" };\n";
      }
    }
  }

  h << endl;

  // Constructor or singleton accessor
  if ( !singleton ) {
    h << "    " << className << "(";
    if (cfgFileNameArg)
       h << " KSharedConfig::Ptr config" << (parameters.isEmpty() ? " " : ", ");
    for (QStringList::ConstIterator it = parameters.begin();
         it != parameters.end(); ++it)
    {
       if (it != parameters.begin())
         h << ",";
       h << " const QString &" << *it;
    }
    h << " );" << endl;
  } else {
    h << "    static " << className << " *self();" << endl;
  }

  // Destructor
  h << "    ~" << className << "();" << endl << endl;

  QString This;
  QString Const;
  if (staticAccessors)
    This = "self()->";
  else
    Const = " const";

  for( e = entries.first(); e; e = entries.next() ) {
    QString n = e->name();
    QString t = e->type();

    // Manipulator
    if (allMutators || mutators.contains(n))
    {
      h << "    /**" << endl;
      h << "      Set " << e->label() << endl;
      h << "    */" << endl;
      if (staticAccessors)
        h << "    static" << endl;
      h << "    void " << setFunction(n) << "( ";
      if (!e->param().isEmpty())
        h << cppType(e->paramType()) << " i, ";
      h << param( t ) << " v )" << endl;
      h << "    {" << endl;
      h << "      if (!" << This << "isImmutable( QString::fromLatin1( \"";
      if (!e->param().isEmpty()) {
        h << e->paramName().replace("$("+e->param()+")", "%1") << "\" ).arg( ";
        if ( e->paramType() == "Enum" ) {
          h << "QString::fromLatin1( ";
          if (globalEnums) 
            h << enumName(e->name()) << "ToString[i]";
          else 
            h << enumName(e->param()) << "::enumToString[i]";
          h << " )";
        } else {
          h << "i";
        }
        h << " )";
      } else
        h << n << "\" )";
      h << " ))" << endl;
      h << "        " << This << varName(n);
      if (!e->param().isEmpty())
        h << "[i]";
      h << " = v;" << endl;
      h << "    }" << endl << endl;
    }

    // Accessor
    h << "    /**" << endl;
    h << "      Get " << e->label() << endl;
    h << "    */" << endl;
    if (staticAccessors)
      h << "    static" << endl;
    h << "    " << cppType(t) << " " << getFunction(n) << "(";
    if (!e->param().isEmpty())
      h << " " << cppType(e->paramType()) <<" i ";
    h << ")" << Const << endl;
    h << "    {" << endl;
    h << "      return " << This << varName(n);
    if (!e->param().isEmpty()) h << "[i]";
    h << ";" << endl;
    h << "    }" << endl;

    // Item accessor
    if ( itemAccessors ) {
      h << endl;
      h << "    /**" << endl;
      h << "      Get Item object corresponding to " << n << "()"
        << endl;
      h << "    */" << endl;
      h << "    Item" << itemType( e->type() ) << " *"
        << getFunction( n ) << "Item(";
      if (!e->param().isEmpty()) {
        h << " " << cppType(e->paramType()) << " i ";
      }
      h << ")" << endl;
      h << "    {" << endl;
      h << "      return " << itemVar(e);
      if (!e->param().isEmpty()) h << "[i]";
      h << ";" << endl;
      h << "    }" << endl;
    }

    h << endl;
  }

  // Static writeConfig method for singleton
  if ( singleton ) {
    h << "    static" << endl;
    h << "    void writeConfig()" << endl;
    h << "    {" << endl;
    h << "      static_cast<KConfigSkeleton*>(self())->writeConfig();" << endl;
    h << "    }" << endl;
  }

  h << "  protected:" << endl;

  // Private constructor for singleton
  if ( singleton ) {
    h << "    " << className << "();" << endl;
    h << "    static " << className << " *mSelf;" << endl << endl;
  }

  // Member variables
  if ( !memberVariables.isEmpty() && memberVariables != "private" ) {
    h << "  " << memberVariables << ":" << endl;
  }

  // Class Parameters
  for (QStringList::ConstIterator it = parameters.begin();
       it != parameters.end(); ++it)
  {
     h << "    QString mParam" << *it << ";" << endl;
  }

  QString group;
  for( e = entries.first(); e; e = entries.next() ) {
    if ( e->group() != group ) {
      group = e->group();
      h << endl;
      h << "    // " << group << endl;
    }
    h << "    " << cppType(e->type()) << " " << varName(e->name());
    if (!e->param().isEmpty())
    {
      h << QString("[%1]").arg(e->paramMax()+1);
    }
    h << ";" << endl;
  }

  h << endl << "  private:" << endl;
  if ( itemAccessors ) {
    for( e = entries.first(); e; e = entries.next() ) {
      h << "    Item" << itemType( e->type() ) << " *" << itemVar( e );
      if (!e->param().isEmpty() ) h << QString("[%1]").arg( e->paramMax()+1 );
      h << ";" << endl;
    }
  }

  if (customAddons)
  {
     h << "    // Include custom additions" << endl;
     h << "    #include \"" << filenameOnly(baseName) << "_addons.h\"" <<endl;
  }

  h << "};" << endl << endl;

  if ( !nameSpace.isEmpty() ) h << "}" << endl << endl;

  h << "#endif" << endl;


  header.close();

  QFile implementation( baseDir + implementationFileName );
  if ( !implementation.open( IO_WriteOnly ) ) {
    kdError() << "Can't open '" << implementationFileName << "for writing."
              << endl;
    return 1;
  }

  QTextStream cpp( &implementation );


  cpp << "// This file is generated by kconfig_compiler from " << args->url(0).fileName() << "." << endl;
  cpp << "// All changes you do to this file will be lost." << endl << endl;

  cpp << "#include \"" << headerFileName << "\"" << endl << endl;

  if ( setUserTexts ) cpp << "#include <klocale.h>" << endl << endl;

  // Includes
  for( it = includes.begin(); it != includes.end(); ++it ) {
    cpp << "#include <" << *it << ">" << endl;
  }

  // Header required by singleton implementation
  if ( singleton )
    cpp << "#include <kstaticdeleter.h>" << endl << endl;

  if ( !nameSpace.isEmpty() )
    cpp << "using namespace " << nameSpace << ";" << endl << endl;

  // Singleton implementation
  if ( singleton ) {
    cpp << className << " *" << className << "::mSelf = 0;" << endl;
    cpp << "static KStaticDeleter<" << className << "> static" << className << "Deleter;" << endl << endl;

    cpp << className << " *" << className << "::self()" << endl;
    cpp << "{" << endl;
    cpp << "  if ( !mSelf ) {" << endl;
    cpp << "    static" << className << "Deleter.setObject( mSelf, new " << className << "() );" << endl;
    cpp << "    mSelf->readConfig();" << endl;
    cpp << "  }" << endl << endl;
    cpp << "  return mSelf;" << endl;
    cpp << "}" << endl << endl;
  }
  
  if ( !cppPreamble.isEmpty() )
    cpp << cppPreamble << endl;

  // Constructor
  cpp << className << "::" << className << "( ";
  if (cfgFileNameArg)
     cpp << " KSharedConfig::Ptr config" << (parameters.isEmpty() ? " " : ", ");
  for (QStringList::ConstIterator it = parameters.begin();
       it != parameters.end(); ++it)
  {
     if (it != parameters.begin())
       cpp << ",";
     cpp << " const QString &" << *it;
  }
  cpp << " )" << endl;

  cpp << "  : " << inherits << "(";
  if ( !cfgFileName.isEmpty() ) cpp << " QString::fromLatin1( \"" << cfgFileName << "\" ";
  if ( cfgFileNameArg ) cpp << " config ";
  if ( !cfgFileName.isEmpty() ) cpp << ") ";
  cpp << ")" << endl;

  // Store parameters
  for (QStringList::ConstIterator it = parameters.begin();
       it != parameters.end(); ++it)
  {
     cpp << "  , mParam" << *it << "(" << *it << ")" << endl;
  }

  cpp << "{" << endl;

  // Needed in case the singleton class is used as baseclass for
  // another singleton.
  if ( singleton )
    cpp << "  mSelf = this;" << endl;

  group = QString::null;
  for( e = entries.first(); e; e = entries.next() ) {
    if ( e->group() != group ) {
      if ( !group.isEmpty() ) cpp << endl;
      group = e->group();
      cpp << "  setCurrentGroup( " << paramString(group, parameters) << " );" << endl << endl;
    }

    QString key = paramString(e->key(), parameters);
    if ( !e->code().isEmpty())
    {
      cpp << e->code() << endl;
    }
    if ( e->type() == "Enum" ) {
      cpp << "  QValueList<KConfigSkeleton::ItemEnum::Choice> values"
          << e->name() << ";" << endl;
      QValueList<CfgEntry::Choice> choices = e->choices();
      QValueList<CfgEntry::Choice>::ConstIterator it;
      for( it = choices.begin(); it != choices.end(); ++it ) {
        cpp << "  {" << endl;
        cpp << "    KConfigSkeleton::ItemEnum::Choice choice;" << endl;
        cpp << "    choice.name = QString::fromLatin1( \"" << (*it).name << "\" );" << endl;
        if ( setUserTexts ) {
          if ( !(*it).label.isEmpty() )
            cpp << "    choice.label = i18n(" << quoteString((*it).label) << ");" << endl;
          if ( !(*it).whatsThis.isEmpty() )
            cpp << "    choice.whatsThis = i18n(" << quoteString((*it).whatsThis) << ");" << endl;
        }
        cpp << "    values" << e->name() << ".append( choice );" << endl;
        cpp << "  }" << endl;
      }
    }
    cpp << itemDeclaration(e);
    if (e->param().isEmpty())
    {
      // Normal case
      cpp << "  " << itemVar(e) << " = "
          << newItem( e->type(), e->name(), key, e->defaultValue() ) << endl;

      if ( !e->minValue().isEmpty() )
        cpp << "  " << itemVar(e) << "->setMinValue(" << e->minValue() << ");" << endl;
      if ( !e->maxValue().isEmpty() )
        cpp << "  " << itemVar(e) << "->setMaxValue(" << e->maxValue() << ");" << endl;

      if ( setUserTexts )
        cpp << userTextsFunctions( e );

      cpp << "  addItem( " << itemVar(e);
      QString quotedName = e->name();
      addQuotes( quotedName );
      if ( quotedName != key ) cpp << ", QString::fromLatin1( \"" << e->name() << "\" )";
      cpp << " );" << endl;
    }
    else
    {
      // Indexed
      for(int i = 0; i <= e->paramMax(); i++)
      {
        QString defaultStr;
        QString itemVarStr(itemVar(e)+QString("[%1]").arg(i));
        
        if ( !e->paramDefaultValue(i).isEmpty() )
          defaultStr = e->paramDefaultValue(i);
        else if ( !e->defaultValue().isEmpty() )
          defaultStr = paramString(e->defaultValue(), e, i);
        else
          defaultStr = defaultValue( e->type() );

        cpp << "  " << itemVarStr << " = "
            << newItem( e->type(), e->name(), paramString(key, e, i), defaultStr, QString("[%1]").arg(i) )
            << endl;

        if ( setUserTexts )
          cpp << userTextsFunctions( e, itemVarStr, e->paramName() );

        // Make mutators for enum parameters work by adding them with $(..) replaced by the 
        // param name. The check for isImmutable in the set* functions doesn't have the param 
        // name available, just the corresponding enum value (int), so we need to store the 
        // param names in a separate static list!.
        cpp << "  addItem( " << itemVarStr << ", QString::fromLatin1( \"";
        if ( e->paramType()=="Enum" )
          cpp << e->paramName().replace( "$("+e->param()+")", "%1").arg(e->paramValues()[i] );
        else
          cpp << e->paramName().replace( "$("+e->param()+")", "%1").arg(i);
        cpp << "\" ) );" << endl;
      }
    }
  }

  cpp << "}" << endl << endl;

  // Destructor
  cpp << className << "::~" << className << "()" << endl;
  cpp << "{" << endl;
  if ( singleton ) {
    cpp << "  if ( mSelf == this )" << endl;
    cpp << "    static" << className << "Deleter.setObject( mSelf, 0, false );" << endl;
  }
  cpp << "}" << endl << endl;

  implementation.close();
}
