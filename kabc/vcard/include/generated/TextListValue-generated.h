// XXX Automatically generated. DO NOT EDIT! XXX //

public:
TextListValue();
TextListValue(const TextListValue&);
TextListValue(const QCString&);
TextListValue & operator = (TextListValue&);
TextListValue & operator = (const QCString&);
bool operator ==(TextListValue&);
bool operator !=(TextListValue& x) {return !(*this==x);}
bool operator ==(const QCString& s) {TextListValue a(s);return(*this==a);} 
bool operator != (const QCString& s) {return !(*this == s);}

virtual ~TextListValue();
void parse() {if(!parsed_) _parse();parsed_=true;assembled_=false;}

void assemble() {if(assembled_) return;parse();_assemble();assembled_=true;}

void _parse();
void _assemble();
const char * className() const { return "TextListValue"; }

// End of automatically generated code           //
