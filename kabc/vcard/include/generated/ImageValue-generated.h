// XXX Automatically generated. DO NOT EDIT! XXX //

public:
ImageValue();
ImageValue(const ImageValue&);
ImageValue(const QCString&);
ImageValue & operator = (ImageValue&);
ImageValue & operator = (const QCString&);
bool operator ==(ImageValue&);
bool operator !=(ImageValue& x) {return !(*this==x);}
bool operator ==(const QCString& s) {ImageValue a(s);return(*this==a);} 
bool operator != (const QCString& s) {return !(*this == s);}

virtual ~ImageValue();
void parse() {if(!parsed_) _parse();parsed_=true;assembled_=false;}

void assemble() {if(assembled_) return;parse();_assemble();assembled_=true;}

void _parse();
void _assemble();
const char * className() const { return "ImageValue"; }

// End of automatically generated code           //
