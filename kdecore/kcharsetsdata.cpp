/* This file is part of the KDE libraries
    Copyright (C) 1997 Jacek Konieczny (jajcus@zeus.polsl.gliwice.pl)
    $Id$

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

#include "kcharsetsdata.h"
#include <qdir.h>
#include <qfile.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <qfontinfo.h>
#include <qintdict.h>
#include <kapp.h>
#include <ksimpleconfig.h>
#include <qregexp.h>
#include <qstrlist.h>
#include <X11/Xlib.h>

#ifdef KCH_DEBUG
inline void kchdebug(const char *msg,...){
    va_list ap;
    va_start( ap, msg );                // use variable arg list
    vfprintf( stderr, msg, ap );
    va_end( ap );                    
}    
#endif

KCharsetConverterData::KCharsetConverterData(const KCharsetEntry * inputCharset
                       ,const KCharsetEntry * outputCharset,int flags){

  kchdebug("Creating converter from %s to %s...",inputCharset,outputCharset);
  tempResult=new KCharsetConversionResult();
  inAmps=( (flags&KCharsetConverter::INPUT_AMP_SEQUENCES)!=0 );
  outAmps=( (flags&KCharsetConverter::OUTPUT_AMP_SEQUENCES)!=0 );
  if ( kcharsetsData == 0 ) fatal("KCharsets not initialized!");
  isOK=initialize(inputCharset,outputCharset);
  kchdebug("done");				   
}

KCharsetConverterData::KCharsetConverterData(const KCharsetEntry * inputCharset
                                             ,int flags){

  kchdebug("Creating converter from %s...",inputCharset);
  tempResult=new KCharsetConversionResult();
  inAmps=( (flags&KCharsetConverter::INPUT_AMP_SEQUENCES)!=0 );
  outAmps=( (flags&KCharsetConverter::OUTPUT_AMP_SEQUENCES)!=0 );
  if ( kcharsetsData == 0 ) fatal("KCharsets not initialized!");
  isOK=initialize(inputCharset,0);
  kchdebug("done");
}

KCharsetConverterData::~KCharsetConverterData(){

  if (tempResult) delete tempResult;
  if (convFromUniDict) delete convFromUniDict;
}

bool KCharsetConverterData::initialize(const KCharsetEntry * inputCharset
				      ,const KCharsetEntry * outputCharset){
					   
  convTable=0;
  convToUniDict=0;
  convFromUniDict=0;
  input=inputCharset;
  if (!input) {
    kchdebug("Couldn't set input charset to %s\n",inputCharset);
    return FALSE;
  }  
  if (outputCharset==0) output=kcharsetsData->conversionHint(input);
  else output=outputCharset;
  if (!output) {
    kchdebug("Couldn't set output charset to %s\n",outputCharset);
    return FALSE;
  }
  
  setInputSettings();
  setOutputSettings();

  if ( !input || !output || input==output ) conversionType=NoConversion;
  else if (unicodeIn && unicodeOut) conversionType=UnicodeUnicode;
  else if (unicodeIn){
    conversionType=FromUnicode;
    kchdebug("Conversion: from unicode\n");
    if (!createFromUnicodeDict()) return FALSE;
  }  
  else if (unicodeOut){
    conversionType=ToUnicode;
    kchdebug("Conversion: to unicode\n");
    if (!getToUnicodeTable()) return FALSE;
  }  
  else{
    conversionType=EightToEight;
    kchdebug("Conversion: no unicode\n");
    if (!getToUnicodeTable()) return FALSE;
    if (!createFromUnicodeDict()) return FALSE;
  } 
  return TRUE;
}					   

bool KCharsetConverterData::getToUnicodeTable(){

  convTable=kcharsetsData->getToUnicodeTable(input);
  if (!convTable){
    convToUniDict=kcharsetsData->getToUnicodeDict(input);
    if (!convToUniDict){
      kchdebug("Couldn't get conversion table nor dictionary\n");
      return FALSE;
    }
  }
  return TRUE;
}

void KCharsetConverterData::setInputSettings(){

  const char *name=input->name;
  
//  if ( ! stricmp(name,"unicode-1-1-utf-7") ){
//    warning("Sorry, UTF-7 encoding is not supported yet\n");
//    inputEnc=UTF7;
//    inBits=0;
//    unicodeIn=TRUE;
//  }  
//  else
  if ( ! stricmp(name,"unicode-1-1-utf-8") ){
    inputEnc=UTF8;
    inBits=0;
    unicodeIn=TRUE;
  }  
  else if ( ! stricmp(name,"unicode") ){
    inputEnc=none;
    inBits=16;
    unicodeIn=TRUE;
  }
  else if ( ! stricmp(name,"us-ascii") ){
    inputEnc=none;
    inBits=7;
    unicodeIn=FALSE;
  }
  else{
    inputEnc=none;
    inBits=8;
    unicodeIn=FALSE;
  } 
}

void KCharsetConverterData::setOutputSettings(){

  const char *name=output->name;
  
//  if ( ! stricmp(name,"unicode-1-1-utf-7") ){
//    warning("Sorry, UTF-7 encoding is not supported yet\n");
//    outputEnc=UTF7;
//    outBits=0;
//    unicodeOut=TRUE;
//  }  
//  else
  if ( ! stricmp(name,"unicode-1-1-utf-8") ){
    outputEnc=UTF8;
    outBits=0;
    unicodeOut=TRUE;
  }  
  else if ( ! stricmp(name,"unicode-1-1") ){
    warning("Sorry, Unicode probably doesn't work (except UTF-8)\n");
    outputEnc=none;
    outBits=16;
    unicodeOut=TRUE;
  }
  else if ( ! stricmp(name,"us-ascii") ){
    outputEnc=none;
    outBits=7;
    unicodeOut=FALSE;
  }
  else{
    outputEnc=none;
    outBits=8;
    unicodeOut=FALSE;
  } 
}


bool KCharsetConverterData::decodeUTF7(const char*,unsigned int &,int &){

  return FALSE;
}

bool KCharsetConverterData::decodeUTF8(const char*str,unsigned int &code
                                      ,int &extrachars){
  code=0;
  extrachars=0;
  unsigned char chr=*str;
//  kchdebug("str: ");
//  for(int i=0;i<6 && str[i];i++)
//    kchdebug("%02x ",(int)(unsigned char)str[i]);
//  kchdebug("\n");
  if ( (chr&0x80)==0 ){
    code=chr&0x7f;
    extrachars=0;
  }
  else if ( (chr&0xe0)==0xc0 ){
    code=chr&0x1f;
    extrachars=1;
  }
  else if ( (chr&0xf0)==0xe0 ){
    code=chr&0x0f;
    extrachars=2;
  }  
  else if ( (chr&0xf8)==0xf0 ){
    code=chr&0x07;
    extrachars=3;
  }  
  else if ( (chr&0xfc)==0xf8 ){
    code=chr&0x03;
    extrachars=4;
  }  
  else if ( (chr&0xfe)==0xf8 ){
    code=chr&0x01;
    extrachars=5;
  }  
  else {
    warning("Invalid UTF-8 sequence %2x%2x...!",(int)chr,(int)str[1]);
    return FALSE;
  }  

  int chars=extrachars;
  while(chars>0){
    str++;
    code<<=6;
//    kchdebug("Code: %4x char: %2x masked char: %2x\n",code,*str,(*str)&0x3f);
    code|=(*str)&0x3f;
    chars--;
  }
  return TRUE;
}

bool KCharsetConverterData::encodeUTF7(unsigned int,QString &){

  return FALSE;
}

bool KCharsetConverterData::encodeUTF8(unsigned int code,QString &result){

  if (code<0x80){
    result+=(char)code;
    return TRUE;
  }  
//  kchdebug("Code: %4x\n",code);
  int octets=2;
  unsigned mask1=0xc0;
  unsigned mask2=0x1f;
  unsigned range=0x800;
  int left=24;
  while(code>range){
    if (range>=0x40000000){
      warning("Unicode value too big!");
      return FALSE;
    }
    mask2=(mask2>>1)&0x80;
    mask1>>=1;
    range<<=5;
    octets++;
    left-=6;
  }
//  kchdebug("octets: %i  mask1: %x mask2: %x range: %x left: %i\n"
//            ,octets,mask1,mask2,range,left);
  unsigned char chr=((code>>((octets-1)*6))&mask2)|mask1;
//  kchdebug("Chars: %02x ",chr);
  result+=chr;
  octets--;
  unsigned int tmp=(code<<left)&0xffffffff;
  while(octets>0){
    chr=((tmp>>24)&0x3f)|0x80;
//    kchdebug("%02x ",chr);
    result+=chr;
    tmp<<=6;
    octets--;
  }
//  kchdebug("\n");
  return TRUE;
}

void KCharsetConverterData::convert(const char *str,KCharsetConversionResult &r){

   convert(str,r,0);
}

void KCharsetConverterData::convert(const char *str,QList<KCharsetConversionResult> &rl){
unsigned unicode;

  rl.clear();
  while(str){
     KCharsetConversionResult *l=new KCharsetConversionResult;
     kchdebug("Created result: %p\n",l);
     str=convert(str,*l,&unicode);
     rl.append(l); 
     if (unicode){
         KCharsetConversionResult *l=new KCharsetConversionResult;
         kchdebug("Created result: %p\n",l);
         kcharsetsData->convert(unicode,*l);
         rl.append(l); 
     }
  }
}
   
const char * KCharsetConverterData::convert(const char * str
                                 ,KCharsetConversionResult &result
				 ,unsigned *pUnicode) {

  kchdebug("Setteing result charset to %p ",&output);
  kchdebug("(%s)\n",(const char *)output);
  result.cCharset=output;
  kchdebug("----- %s ----- => ",str);
  if (!isOK) return 0;
  if (conversionType == NoConversion ){
    result.cText=str;
    return 0;
  }
  result.cText="";
  
  int i;
  int tmp;
  unsigned *ptr=0;
  unsigned index=0;
  unsigned unicode=0;
  unsigned chr=0;
  
  for(i=0;(inBits<=8)?str[i]:(str[i]&&str[i+1]);){
    chr=0;
    index=0;
    unicode=0;
    if (inAmps && str[i]=='&'){
       kchdebug("Amperstand found\n");
       unicode=kcharsetsData->decodeAmp(str+i,tmp);
       kchdebug("i=%i characters: %i code:%4x\n",i,tmp,unicode);
       if (tmp>0) i+=tmp-1;
    } 
    if (unicode==0) switch(inputEnc){
       case UTF7:
         if (decodeUTF7(str+i,unicode,tmp)) i+=tmp;
	 else unicode=(unsigned char)str[i];
	 break;
       case UTF8:
         if (decodeUTF8(str+i,unicode,tmp)) i+=tmp;
	 else unicode=(unsigned char)str[i];
	 break;
       default:
         if (inBits<=8) index=(unsigned char)str[i];
	 else if (inBits==16) index=(((unsigned char)str[i++])<<8)+(unsigned char)str[i];
	 break;
    }
    kchdebug("Got index: %x\n",index);
    if (index>0 || unicode>0) switch(conversionType){
       case ToUnicode:
         if (unicode>0) chr=unicode;
	 else
           if (convTable)
	     chr=convTable[index];
	   else if (convToUniDict) {
             ptr=(*convToUniDict)[index];
	     if (ptr) chr=*ptr;
	     else chr=0;
	   }  
         if (chr==0 && index>0 && index<0x20)
            chr=index; // control characters - do not change
	 break;
       case FromUnicode:
         ptr=(*convFromUniDict)[unicode];
	 if (ptr) chr=*ptr;
	 else chr=0;
         if (unicode>0 && unicode<0x20)
            chr=index; // control characters - do not change
	 break;
       case UnicodeUnicode:
         chr=unicode;
	 break;
       default:
         if (unicode==0)
           if (convTable)
  	     unicode=convTable[index];
	   else{
             ptr=(*convToUniDict)[index];
 	     if (ptr) unicode=*ptr;
	     else unicode=0;
	   }  
//         kchdebug("Converted to unicode: %4x\n",index);
	 if (unicode){
            ptr=(*convFromUniDict)[unicode];
  	    if (ptr) chr=*ptr;
	    else chr=0;
	 }   
	 else chr=0;
         if (chr==0 && index>0 && index<0x20)
            chr=index; // control characters - do not change
         break;
    }
//    kchdebug("Converted to: %x\n",chr);
    if (outputEnc==UTF8) encodeUTF8(chr,result.cText);
    else if (outputEnc==UTF7) encodeUTF7(chr,result.cText);
    else if (chr==0)
      if (unicode && pUnicode){
        *pUnicode=unicode;
        i++;
        if (inBits>8 && str[i]) i++;
        result.cCharset=output;
	if (str[i]) return str+i;
	else return 0;
      }
      else if (unicode && unicode<0x20) result.cText+=(char)unicode;
      else if (outAmps){
	if (unicode) result.cText+="&#"+QString().setNum(unicode)+';';
	else result.cText+="?";
      }  
      else result.cText+="?";
    else
      if (outBits==16){
        result.cText+=(char)(chr>>8);
	result.cText+=(char)(chr&255);
      }
      else result.cText+=(char)chr;
      
    i++;
    if (inBits>8 && str[i]) i++;
  }
  kchdebug("----- %s -----\n",(const char *)result);
  if (pUnicode) *pUnicode=0;
  return 0;
}


const KCharsetConversionResult & KCharsetConverterData::convert(unsigned code){
unsigned chr;
const unsigned *ptr;

   kchdebug("KCCS:convert(code) ");

   if (convFromUniDict){
     ptr=(*convFromUniDict)[code];
     if (!ptr) chr=0;
     else chr=*ptr;
   }  
   else chr=0;
   
   if (chr==0){
     kcharsetsData->convert(code,*tempResult);
     if (!tempResult->cText.isEmpty()) return *tempResult;
   }  

   if (chr==0)
      if (outAmps){
	if (code) tempResult->cText+="&#"+QString().setNum(code)+';';
	else tempResult->cText+="?";
      }  
      else tempResult->cText+="?";
   else tempResult->cText=chr;
   
   return *tempResult;
}

const KCharsetConversionResult & KCharsetConverterData::convertTag(
                                     const char *tag,int &l){
  
 kchdebug("Converting: %s\n",(const char *)tag);
 return convert(kcharsetsData->decodeAmp(tag,l));
}


bool KCharsetConverterData::createFromUnicodeDict(){

  QIntDict<unsigned> * dict=new QIntDict<unsigned>;
  dict->setAutoDelete(TRUE);
  const unsigned *tbl=kcharsetsData->getToUnicodeTable(output);
  if (tbl)
    for(int i=0;i<(1<<outBits);i++)
      dict->insert(tbl[i],new unsigned(i));
  else{
    QIntDict<unsigned> * dict2=kcharsetsData->getToUnicodeDict(output);
    if (!dict2){
      kchdebug("Couldn't get to unicode table for %s\n",output->name);
      delete dict;
      return FALSE;
    }
    QIntDictIterator<unsigned> it(*dict2);
    while( it.current() ){
      dict->insert(*it.current(),new unsigned(it.currentKey()));
      ++it;
    }
  }
  convFromUniDict=dict;
  return TRUE;
}

const char * KCharsetConverterData::outputCharset()const{

  return output->name;
}

/////////////////////////////////////////////////

KCharsetsData::KCharsetsData(){

  displayableCharsDict=0;
  tempResult=new KCharsetConversionResult;

  QString fileName=KApplication::kde_configdir() + "/charsets";
  kchdebug("Reading config from %s...\n",(const char *)fileName);
  config=new KSimpleConfig(fileName);
  config->setGroup("general");
  QString i18dir = config->readEntry("i18ndir");
  if (i18dir) scanDirectory(i18dir);
  kchdebug("Creating alias dictionary...\n");
  KEntryIterator *it=config->entryIterator("aliases");
  if ( it )
  {
      while( it->current() ){
	const char*alias=it->currentKey();
	kchdebug(" %s -> ",alias);
	const char*name=it->current()->aValue;
	kchdebug(" %s:",name);
	KCharsetEntry *ce=varCharsetEntry(name);
	if (ce){
	    aliases.insert(alias,ce);
	    kchdebug("ok\n");
	}
	else kchdebug("not found\n");
	++(*it);
      }  
  }
  kchdebug("done!\n");
}

void KCharsetsData::scanDirectory(const char *path){

  kchdebug("Scanning directory: %s\n",path);
  QDir d(path);
  if ( ! d.exists() ) return;
  d.setFilter(QDir::Files);
  d.setSorting(QDir::Name);
  const QFileInfoList *list=d.entryInfoList();
  QFileInfoListIterator it(*list);
  QFileInfo *fi;
  while( (fi=it.current()) ){
    QString name=fi->fileName();
    QString alias=name.copy();
    int comma=alias.find(',');
    if (comma) alias.remove(comma,alias.length()-comma);
    else alias="";
    if (!charsetEntry(alias) && !charsetEntry(name)){
      KCharsetEntry *entry=new KCharsetEntry;
      char *ptr=new char [fi->fileName().length()+1];
      strcpy(ptr,name);
      entry->name=ptr;
      entry->qtCharset=QFont::AnyCharSet;
      entry->toUnicode=0;
      entry->registered=FALSE;
      entry->toUnicodeDict=0;
      i18nCharsets.insert(name.lower(),entry);
      if (alias!="") aliases.insert(alias,entry);
    }	
    ++it;
  }
}

void KCharsetsData::createDictFromi18n(KCharsetEntry *e){

  kchdebug("Creating unicode dict for %s\n",e->name);
  config->setGroup("general");
  QString dir=config->readEntry("i18ndir");
  kchdebug("Dir: %s\n",(const char *)dir);
  QString filename=dir+'/'+e->name;
  kchdebug("Trying to open file %s\n",(const char *)filename);
  QFile f(filename);
  if (!f.open(IO_ReadOnly)) return;
  QTextStream t(&f);
  QString l;
  do{
    l=t.readLine();
  }while(!t.eof() && l!="CHARMAP");
  if (t.eof()){
    f.close();
    return;
  }
  
  QIntDict<unsigned> *dict=new QIntDict<unsigned>;
  dict->setAutoDelete(TRUE);
  
  char codeBuf[20];
  char unicodeBuf[10];
  unsigned unicode;
  unsigned code;
  
  while(!t.eof()){
    l=t.readLine();
    if (l=="END CHARMAP") break;
    sscanf(l,"%*s %16s %8s %*s",codeBuf,unicodeBuf);
    sscanf(unicodeBuf,"<U%x>",&unicode);
    code=0;
    if ( sscanf(codeBuf,"/x%X",&code) < 1 )
      code=codeBuf[0];
    kchdebug("(%s %s) %x->%x\n",codeBuf,unicodeBuf,code,unicode);
    dict->insert(code,new unsigned(unicode));	
  }
  e->toUnicodeDict=dict;
  defaultCh=charsetEntry("us-ascii");
}
  
KCharsetsData::~KCharsetsData(){

  if (tempResult) delete tempResult;
  QDictIterator<KCharsetEntry> it(i18nCharsets);
  KCharsetEntry *e;
  while( (e=it.current()) ){
    if (e->toUnicodeDict) delete e->toUnicodeDict;
    if (e->name) delete e->name;
    delete e;
  }
  if (displayableCharsDict) delete displayableCharsDict;
  delete config;
}

KCharsetEntry * KCharsetsData::varCharsetEntry(const char *name){

  for(int i=0;charsets[i].name;i++){
    if ( stricmp(name,charsets[i].name) == 0 ){
      kchdebug("Found!\n");
      return charsets+i;
    }  
  }  
  KCharsetEntry *e=i18nCharsets[QString(name).lower()];
  if (!e){
     kchdebug("Searchin in aliases...\n");
     e=aliases[QString(name).lower()];
  }   
  return e;
}

const KCharsetEntry * KCharsetsData::charsetEntry(int index){

  int i;
  for(i=0;charsets[i].name;i++)
    if ( i==index ) return charsets+i;
    
  QDictIterator<KCharsetEntry> it(i18nCharsets);
  KCharsetEntry *e;
  while( (e=it.current()) ){
    if (i==index) return e;
    ++i;
    ++it;
  }
  return 0;
}

const KCharsetEntry * KCharsetsData::charsetEntry(QFont::CharSet qtCharset){

  int i;
  for(i=0;charsets[i].name;i++)
    if ( charsets[i].qtCharset==qtCharset ) return charsets+i;
    
  return 0;
}

bool KCharsetsData::setDefaultCharset(const KCharsetEntry *charset){

  if (charset){
    defaultCh=charset;
    return TRUE;
  }
  return FALSE;
}

QString KCharsetsData::charsetFace(const KCharsetEntry *charset
                                   ,const QString &face){

  config->setGroup("faces");
  const char *faceStr=config->readEntry(charset->name);
  if (!faceStr) return face;
  QString newFace(faceStr);
  newFace.replace(QRegExp("\\*"),face);
  return newFace;
}

bool KCharsetsData::charsetOfFace(const KCharsetEntry * charset,const QString &face){
  kchdebug("Testing if face %s is of charset %s...",(const char *)face,
                                                               charset->name);
  config->setGroup("faces");
  const char *faceStr=config->readEntry(charset->name);
  kchdebug("%s...",faceStr);
  QRegExp rexp(faceStr,FALSE,TRUE);
  if (face.contains(rexp)){
    kchdebug("Yes, it is\n");
    return TRUE;
  }  
  kchdebug("No, it isn't\n");
  return FALSE;
}

const KCharsetEntry* KCharsetsData::charsetOfFace(const QString &face){

  kchdebug("Searching for charset for face %s...\n",(const char *)face);
  KEntryIterator * it=config->entryIterator("faces");
  if (!it) return 0;
  while( it->current() ){
    const char * faceStr=it->current()->aValue;
    if (!faceStr || faceStr[0]==0) return charsetEntry(it->currentKey());
    kchdebug("testing if it is %s (%s)...",(const char *)it->currentKey(),faceStr);
    QRegExp rexp(faceStr,FALSE,TRUE);
    kchdebug("regexp: %s face: %s\n",rexp.pattern(),(const char *)face);
    if (face.contains(rexp)){
      kchdebug("Yes, it is\n");
      return charsetEntry(it->currentKey());
    }  
    kchdebug("No, it isn't\n");
    ++(*it);
  }
  return 0;
}
  
const unsigned *KCharsetsData::getToUnicodeTable(const KCharsetEntry *charset){

  if (!charset) return 0;
  return charset->toUnicode;
}
  
QIntDict<unsigned> *KCharsetsData::getToUnicodeDict(const KCharsetEntry *charset){

  if (!charset) return 0;
  if (charset->toUnicodeDict == 0)
  	createDictFromi18n(varCharsetEntry(charset->name));
  return charset->toUnicodeDict;
}

const char *KCharsetsData::faceForCharset(const KCharsetEntry *charset){

  config->setGroup("faces");
  return config->readEntry(charset->name);
}

const KCharsetEntry *KCharsetsData::conversionHint(const KCharsetEntry *charset){

  QStrList list;
  kchdebug("Serching for conversion hint for %s\n",charset->name);
  config->setGroup("conversionHints");
  int n=config->readListEntry(charset->name,list);
  kchdebug("%i entries found\n",n);
  for(const char *hint=list.first();hint;hint=list.next()){
    kchdebug("Found: %s\n",hint);
    KCharsetEntry *ce=varCharsetEntry(hint);
    if (isDisplayable(ce)) return ce;    
  }
    
  return defaultCh;  
}

bool KCharsetsData::getFontList(QStrList*lst,QString xcharsetname){
char **fontNames;
int numFonts;
QString mask("-*-*-*-*-*-*-*-*-*-*-*-*-");
QString qfontname;
Display *kde_display;
  
  if (!lst) return FALSE;
  
  kde_display = XOpenDisplay( 0L );
  mask+=xcharsetname;
  fontNames = XListFonts(kde_display, mask, 32767, &numFonts);

  for(int i = 0; i < numFonts; i++){

    qfontname = "";
    qfontname = fontNames[i];
    int dash = qfontname.find ('-', 1, TRUE); // find next dash

    if (dash == -1) { // No such next dash -- this shouldn't happen.
                      // but what do I care -- lets skip it.
      continue;
    }

    // the font name is between the second and third dash so:
    // let's find the third dash:

    int dash_two = qfontname.find ('-', dash + 1 , TRUE);

    if (dash == -1) { // No such next dash -- this shouldn't happen.
                      // But what do I care -- lets skip it.
      continue;
    }

    // fish the name of the font info string
    qfontname = qfontname.mid(dash +1, dash_two - dash -1);
    lst->append(qfontname);
  }
  XFreeFontNames(fontNames);
}

bool KCharsetsData::isDisplayableHack(KCharsetEntry *charset){
QFont::CharSet qcharset=charset->qtCharset;

  QString face=faceForCharset(charset);
  if ( !face.isEmpty() ){
    if ( face.isEmpty()) return FALSE;
    QFont f(face);
    f.setCharSet(qcharset);
    f.setFamily(face);
    QFontInfo fi(f);
    kchdebug("fi.charset()=%i fi.family()=%s\n",fi.charSet(),fi.family());
    if (fi.family()!=face ) return FALSE;
    /* This face will work for this charset, remember it */
    if (!charset->good_family) charset->good_family=new QString;
    if (charset->good_family->isEmpty()) *(charset->good_family)=face;
    return TRUE;
  }  
  return FALSE; 
}

bool KCharsetsData::isDisplayable(KCharsetEntry *charset){

  QFont::CharSet qcharset=charset->qtCharset;
  kchdebug("qtcharset=%i\n",qcharset);
 
  /* Qt doesn't support this charset. We must use the hack */
  if (qcharset==QFont::AnyCharSet && strcmp(charset->name,"us-ascii")!=0)
       return isDisplayableHack(charset);
  
  QFont f;
  f.setCharSet(qcharset);
  QFontInfo fi(f);
  kchdebug("fi.charset()=%i\n",fi.charSet());
  if (qcharset!=QFont::AnyCharSet && fi.charSet()!=qcharset){ /* It doesn't work, maybe Qt bug*/
    /* Is a good family known for this charset? */
    if (charset->good_family){
       if (charset->good_family->isEmpty()) /* no good_family is known */
         return  isDisplayableHack(charset);
       f.setFamily(*charset->good_family);
       f.setCharSet(qcharset);
       return TRUE;
    }
    QStrList lst;
    getFontList(&lst,toX(charset->name));
    charset->good_family=new QString;
    for (const char* fm = lst.first(); fm; fm = lst.next()) {
       f.setCharSet(qcharset);
       f.setFamily(fm);
       QFontInfo fi(f);
       if (fi.charSet()==qcharset){
	  *(charset->good_family)=fm;
	  return TRUE;
       }
    }
    /* All this does not work, try the hack */
    return  isDisplayableHack(charset);
  }
  return TRUE;
}

void KCharsetsData::convert(unsigned code,KCharsetConversionResult &convResult){
unsigned chr;

   kchdebug("KCD:convert(code) %4X -> ",code);
   chr=0;

   kchdebug("Clearing result (was: %s)...\n",(const char *)convResult.cText);
   convResult.cText="";
   kchdebug("Clearing charset...\n");
   convResult.cCharset=charsetEntry("us-ascii");
   if (code>127){
     kchdebug("Hi code, dictonary needed, getting...\n");
     const QIntDict<KDispCharEntry> *dict=getDisplayableDict();
     kchdebug("Dictonary: %p\n",dict);
     KDispCharEntry *ptr=(*dict)[code];
     kchdebug("Entry: %p\n",ptr);
     if (ptr){
       chr=ptr->code;
       kchdebug("Setting charset to %s...\n",ptr->charset->name);
       convResult.cCharset=ptr->charset; 
       kchdebug("Setting text to code %2X...\n",chr);
       convResult.cText+=(unsigned char)chr;
     }
   }
   else{
     kchdebug("Setting text to code %2X...\n",code);
     convResult.cText+=(unsigned char)code;
   }   
   kchdebug("%s\n",(const char *)convResult);
}

unsigned KCharsetsData::decodeAmp(const char *seq,int &len){
  unsigned int i;

  kchdebug("Sequence: '%0.20s'\n",seq);

  if (*seq=='&') { seq++; len=1; }
  else len=0;
  
  if (*seq=='#'){
     char *endptr;
     unsigned num;
     if (*(seq+1) == 'x') 
         num =strtoul(seq+2,&endptr,16);
     else
         num =strtoul(seq+1,&endptr,10);
     
     kchdebug("Number: '%u'\n",num);
     if (*endptr==';') len+=endptr-seq+1;
     else len+=endptr-seq;
     return num;
  }   
  else
    for(i=0;i<CHAR_TAGS_COUNT;i++){
      KCharTags tag=tags[i];
      int l=strlen(tag.tag);
      if ( strncmp(seq,tag.tag,l)==0 ){
        if (seq[l]==';' && tag.tag[l-1]!=';') len+=l+1;
	else len+=l;
        return tag.code;	  
      }
    }
    
  len=0;
  return 0;
}

void KCharsetsData::convertTag(const char *tag
                            ,KCharsetConversionResult &convResult
			    ,int &l){
  
  convert(decodeAmp(tag,l),convResult);
}

const QIntDict<KDispCharEntry> * KCharsetsData::getDisplayableDict(){

  if (displayableCharsDict) return displayableCharsDict;
  kchdebug("Generating dictonary for displayable characters\n");
  displayableCharsDict=new QIntDict<KDispCharEntry>;
  displayableCharsDict->setAutoDelete(TRUE);
  for(int i=0;charsets[i].name!=0;i++)
    if (charsets[i].toUnicode && isDisplayable(charsets+i)){
       kchdebug("Adding characters form %s\n",charsets[i].name);
       for(int j=0;j<256;j++){
          unsigned unicode=charsets[i].toUnicode[j];
          if ( !(*displayableCharsDict)[unicode] ){
             KDispCharEntry *e=new KDispCharEntry;
	     e->charset=charsets+i;
             e->code=j;
	     displayableCharsDict->insert(unicode,e);
	  }  
       }
    }    
 #ifdef KCH_DEBUG   
 displayableCharsDict->statistics();
 #endif
 return displayableCharsDict;
}

QString KCharsetsData::fromX(QString name){

  if ( strncmp(name,"iso",3)==0 ){
      name="iso-"+name.mid(3,100);
      return name;
  }
		      
  KEntryIterator *it=config->entryIterator("XNames");
  if ( it )
  {
      while( it->current() ){
	if (it->current()->aValue==name ) return it->currentKey();
	++(*it);
      }  
  }
  return ""; 
}

QString KCharsetsData::toX(QString name){

  if ( strncmp(name,"iso-",4)==0 ){
      name="iso"+name.mid(4,100);
      return name;
  }
  config->setGroup("XNames");
  return config->readEntry(name,"");
}
