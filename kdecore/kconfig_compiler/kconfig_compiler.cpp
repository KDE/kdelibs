/*
    This file is part of KDE.

    Copyright (c) 2003 Cornelius Schumacher <schumacher@kde.org>
    Copyright (c) 2003 Waldo Bastian <bastian@kde.org>

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

#include <kaboutdata.h>
#include <kapplication.h>
#include <kdebug.h>
#include <klocale.h>
#include <kcmdlineargs.h>
#include <kglobal.h>
#include <kconfig.h>
#include <ksimpleconfig.h>
#include <kstandarddirs.h>

static const KCmdLineOptions options[] =
{
  { "+file.kfcg", I18N_NOOP("Input kcfg XML file."), 0 },
  { "+file.kcfgc", I18N_NOOP("Code generation options file."), 0 },
  KCmdLineLastOption
};


class CfgEntry
{
  public:
    CfgEntry( const QString &group, const QString &type, const QString &key,
              const QString &name, const QString &label, const QString &code,
              const QString &defaultValue, const QStringList &values )
      : mGroup( group ), mType( type ), mKey( key ), mName( name ),
        mLabel( label ), mCode( code ), mDefaultValue( defaultValue ), 
        mValues( values )
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

    void setDefaultValue( const QString &d ) { mDefaultValue = d; }
    QString defaultValue() const { return mDefaultValue; }

    void setCode( const QString &d ) { mCode = d; }
    QString code() const { return mCode; }

    void setParam( const QString &d ) { mParam = d; }
    QString param() const { return mParam; }

    void setParamType( const QString &d ) { mParamType = d; }
    QString paramType() const { return mParamType; }

    void setValues( const QStringList &d ) { mValues = d; }
    QStringList values() const { return mValues; }

    void setParamValues( const QStringList &d ) { mParamValues = d; }
    QStringList paramValues() const { return mParamValues; }

    void dump() const
    {
      kdDebug() << "<entry>" << endl;
      kdDebug() << "  group: " << mGroup << endl;
      kdDebug() << "  type: " << mType << endl;
      kdDebug() << "  key: " << mKey << endl;
      kdDebug() << "  name: " << mName << endl;
      kdDebug() << "  label: " << mLabel << endl;
      kdDebug() << "  code: " << mCode << endl;
      kdDebug() << "  values: " << mValues.join(":") << endl;
      kdDebug() << "  paramvalues: " << mParamValues.join(":") << endl;
      kdDebug() << "  default: " << mDefaultValue << endl;
      kdDebug() << "</entry>" << endl;
    }

  private:
    QString mGroup;
    QString mType;
    QString mKey;
    QString mName;
    QString mLabel;
    QString mCode;
    QString mDefaultValue;
    QString mParam;
    QString mParamType;
    QStringList mValues;
    QStringList mParamValues;
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

CfgEntry *parseEntry( const QString &group, const QDomElement &element )
{
  bool defaultCode = false;
  QString type = element.attribute( "type" );
  QString name = element.attribute( "name" );
  QString key = element.attribute( "key" );
  QString label;
  QString defaultValue;
  QString code;
  QString param;
  QString paramType;
  QStringList values;
  QStringList paramValues;
  int paramMin;
  int paramMax;

  QDomNode n;
  for ( n = element.firstChild(); !n.isNull(); n = n.nextSibling() ) {
    QDomElement e = n.toElement();
    QString tag = e.tagName();    
    if ( tag == "label" ) label = e.text();
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
      if ((paramType == "int") || (paramType == "uint"))
      {
         bool ok;
         paramMin = e.attribute("min").toInt(&ok);
         if (!ok)
         {
           kdError() << "Integer parameter must have a minimum (e.g. min=\"0\"): " << dumpNode(e) << endl;
           return 0;
         }
         paramMax = e.attribute("max").toInt(&ok);
         if (!ok)
         {
           kdError() << "Integer parameter must have a maximum (e.g. max=\"0\"): " << dumpNode(e) << endl;
           return 0;
         }
         paramValues.clear();
         for(int i = paramMin; i < paramMax; i++)
         {
            paramValues.append(QString("%1").arg(i));
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
    else if ( tag == "values" ) {
      QDomNode n2;
      for( n2 = e.firstChild(); !n2.isNull(); n2 = n2.nextSibling() ) {
        QDomElement e2 = n2.toElement();
        if ( e2.tagName() == "value" ) {
          values.append( e2.text() );
        }
      }
    }
  }
  
  if ( name.isEmpty() ) {
    kdError() << "Entry must have a name: " << dumpNode(element) << endl;
    return 0;
  }
  if (name.contains("$(") && param.isEmpty())
  {
    kdError() << "Name may not be parameterized: " << name << endl;
    return 0;
  }
  if ( key.isEmpty() ) {
    key = name;
  }

  if ( label.isEmpty() ) {
    label = key;
  }
  
  if ( type.isEmpty() ) type = "QString";

  if (!defaultCode)
  {
    if ( type == "QString" && !defaultValue.isEmpty() ) {
      addQuotes( defaultValue );

    } else if ( type == "Path" && !defaultValue.isEmpty() ) {
      addQuotes( defaultValue );

    } else if ( type == "QStringList" && !defaultValue.isEmpty() ) {
      QTextStream cpp( &code, IO_WriteOnly | IO_Append );
      if (!code.isEmpty())
         cpp << endl;

      cpp << "  QStringList default" << name << ";" << endl;
      QStringList defaults = QStringList::split( ",", defaultValue );
      QStringList::ConstIterator it;
      for( it = defaults.begin(); it != defaults.end(); ++it ) {
        cpp << "  default" << name << ".append( \"" << *it << "\" );"
            << endl;
      }
      defaultValue = "default" + name;

    } else if ( type == "QColor" ) {
      addQuotes( defaultValue );
      defaultValue = "QColor( " + defaultValue + " )";

    } else if ( type == "Enum" ) {
      if ( values.contains(defaultValue) ) {
        defaultValue.prepend( enumName(name) + "::");
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
  
  CfgEntry *result = new CfgEntry( group, type, key, name, label, code, defaultValue, values );
  if (!param.isEmpty())
  {
    result->setParam(param);
    result->setParamType(paramType);
  }
  return result;
}

/**
  Return parameter declaration for given type.
*/
QString param( const QString &type )
{
  if ( type == "QString" ) return "const QString &";
  else if ( type == "Path" ) return "const QString &";
  else if ( type == "Enum" ) return "int";
  else if ( type == "QStringList" ) return "const QStringList &";
  else if ( type == "QColor" ) return "const QColor &";
  else if ( type == "QFont" ) return "const QFont &";
  else if ( type == "QSize" ) return "const QSize &";
  else if ( type == "IntList" ) return "const QValueList<int> &";
  else return type;
}

/**
  Actual C++ storage type for given type.
*/
QString cppType( const QString &type )
{
  if ( type == "Path" ) return "QString";
  else if ( type == "Enum" ) return "int";
  else if ( type == "IntList" ) return "QValueList<int>";
  else return type;
}

QString addFunction( const QString &type )
{
  QString f = "addItem";

  QString t;
  
  if ( type == "QString" || type == "QStringList" || type == "QColor" ||
       type == "QFont" || type == "QSize" ) {
    t = type.mid( 1 );
  } else {
    t = type;
    t.replace( 0, 1, t.left( 1 ).upper() );
  }

  f += t;
  
  return f;
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
    return "\""+group+"\"";
  
  return "QString(\""+paramString+"\")"+arguments;
}

int main( int argc, char **argv )
{
  KAboutData aboutData( "kconfig_compiler", I18N_NOOP("KDE .kcfg compiler"), "0.2",
  	I18N_NOOP("KConfig Compiler") , KAboutData::License_LGPL );
  aboutData.addAuthor( "Cornelius Schumacher", 0, "schumacher@kde.org" );
  aboutData.addAuthor( "Waldo Bastian", 0, "bastian@kde.org" );

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
  
  QString className = codegenConfig.readEntry("ClassName");
  QString inherits = codegenConfig.readEntry("Inherits");
  bool singleton = codegenConfig.readBoolEntry("Singleton", false);
  bool staticAccessors = singleton;
  bool customAddons = codegenConfig.readBoolEntry("CustomAdditions");
  QString memberVariables = codegenConfig.readEntry("MemberVariables");
  QStringList includes = codegenConfig.readListEntry("IncludeFiles");
  bool mutators = codegenConfig.readBoolEntry("Mutators");
  
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
  QStringList parameters;

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

  QFile header( headerFileName );
  if ( !header.open( IO_WriteOnly ) ) {
    kdError() << "Can't open '" << headerFileName << "for writing." << endl;
    return 1;
  }

  QTextStream h( &header );

  h << "// This file is generated by kconfig_compiler from " << args->url(0).fileName() << "." << endl;
  h << "// All changes you do to this file will be lost." << endl;

  h << "#ifndef " << className.upper() << "_H" << endl;
  h << "#define " << className.upper() << "_H" << endl << endl;

  // Includes
  QStringList::ConstIterator it;
  for( it = includes.begin(); it != includes.end(); ++it ) {
    h << "#include <" << *it << ">" << endl;
  }

  if ( includes.count() > 0 ) h << endl;

  h << "#include <kconfigskeleton.h>" << endl << endl;

  // Class declaration header
  h << "class " << className << " : public " << inherits << endl;
  h << "{" << endl;
  h << "  public:" << endl;

  // enums
  CfgEntry *e;
  for( e = entries.first(); e; e = entries.next() ) {
    QStringList values = e->values();
    if ( !values.isEmpty() ) {
      h << "    class " << enumName(e->name()) << endl;
      h << "    {" << endl;
      h << "      public:" << endl;
      h << "      enum { " << values.join( ", " ) << " };" << endl;
      h << "    };" << endl;
    }
    values = e->paramValues();
    if ( !values.isEmpty() ) {
      h << "    class " << enumName(e->param()) << endl;
      h << "    {" << endl;
      h << "      public:" << endl;
      h << "      enum { " << values.join( ", " ) << " };" << endl;
      h << "    };" << endl;
    }
  }

  h << endl;
  
  // Constructor or singleton accessor
  if ( !singleton ) {
    h << "    " << className << "(";
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
    if (mutators)
    {
      h << "    /**" << endl;
      h << "      Set " << e->label() << endl;
      h << "    */" << endl;    
      if (staticAccessors)
        h << "    static" << endl;
      h << "    void " << setFunction(n) << "( " << param( t ) << " v )" << endl;
      h << "    {" << endl;
      h << "      if (!" << This << "isImmutable( \"" << n << "\" ))" << endl;
      h << "        " << This << varName(n) << " = v;" << endl;
      h << "    }" << endl << endl;
    }
  
    // Accessor
    h << "    /**" << endl;
    h << "      Get " << e->label() << endl;
    h << "    */" << endl;
    if (staticAccessors)
      h << "    static" << endl;
    h << "    " << cppType(t) << " " << getFunction(n) << "()" << Const << endl;
    h << "    {" << endl;
    h << "      return " << This << varName(n) << ";" << endl;
    h << "    }" << endl;

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
  
  h << "  private:" << endl;
  
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
    h << "    " << cppType(e->type()) << " " << varName(e->name()) << ";" << endl;
  }

  if (customAddons)
  {
     h << "    // Include custom additions" << endl;
     h << "    #include \"" << filenameOnly(baseName) << "_addons.h\"" <<endl;
  }

  h << "};" << endl << endl;

  h << "#endif" << endl;


  header.close();

  QFile implementation( implementationFileName );
  if ( !implementation.open( IO_WriteOnly ) ) {
    kdError() << "Can't open '" << implementationFileName << "for writing."
              << endl;
    return 1;
  }

  QTextStream cpp( &implementation );


  cpp << "// This file is generated by kconfig_compiler from " << args->url(0).fileName() << "." << endl;
  cpp << "// All changes you do to this file will be lost." << endl << endl;

  cpp << "#include \"" << headerFileName << "\"" << endl << endl;

  // Static class pointer for singleton
  if ( singleton ) {
    cpp << "#include <kstaticdeleter.h>" << endl;

    cpp << endl;

    cpp << className << " *" << className << "::mSelf = 0;" << endl;
    cpp << "static KStaticDeleter<" << className << "> staticDeleter;" << endl << endl;

    cpp << className << " *" << className << "::self()" << endl;
    cpp << "{" << endl;
    cpp << "  if ( !mSelf ) {" << endl;
    cpp << "    staticDeleter.setObject( mSelf, new " << className << "() );" << endl;
    cpp << "    mSelf->readConfig();" << endl;
    cpp << "  }" << endl << endl;
    cpp << "  return mSelf;" << endl;
    cpp << "}" << endl << endl;
  }

  // Constructor
  cpp << className << "::" << className << "(" << endl;
  for (QStringList::ConstIterator it = parameters.begin();
       it != parameters.end(); ++it)
  {
     if (it != parameters.begin())
       cpp << ",";
     cpp << " const QString &" << *it;
  }
  cpp << " )" << endl;

  cpp << "  : " << inherits << "(";
  if ( !cfgFileName.isEmpty() ) cpp << " \"" << cfgFileName << "\" ";
  cpp << ")" << endl;

  // Store parameters
  for (QStringList::ConstIterator it = parameters.begin();
       it != parameters.end(); ++it)
  {
     cpp << "  , mParam" << *it << "(" << *it << ")" << endl;
  }

  cpp << "{" << endl;

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
      cpp << "  QStringList values" << e->name() << ";" << endl;
      QStringList values = e->values();
      QStringList::ConstIterator it;
      for( it =  values.begin(); it != values.end(); ++it ) {
        cpp << "  values" << e->name() << ".append( \"" << *it << "\" );" << endl; 
      }
      cpp << "  KConfigSkeleton::ItemEnum *item" << e->name() 
          << " = new KConfigSkeleton::ItemEnum( currentGroup(), " 
          << key << ", " << varName(e->name()) << ", values" << e->name();
      if ( !e->defaultValue().isEmpty() )
        cpp << ", " << e->defaultValue();
      cpp << " );" << endl;
      cpp << "  addItem( \"" << e->name() << "\", item" << e->name() << " );" << endl;
    } else {
      cpp << "  " << addFunction( e->type() ) << "( \"" << e->name() << "\", " << key << ", "
          << varName(e->name());
      if ( !e->defaultValue().isEmpty() )
        cpp << ", " << e->defaultValue();
      cpp << " );" << endl;
    }
  }

  cpp << "}" << endl << endl;

  // Destructor
  cpp << className << "::~" << className << "()" << endl;
  cpp << "{" << endl;
  if ( singleton ) {
    cpp << "  if ( mSelf == this )" << endl;
    cpp << "    staticDeleter.setObject( mSelf, 0, false );" << endl;
  }
  cpp << "}" << endl << endl;

  implementation.close();
}
