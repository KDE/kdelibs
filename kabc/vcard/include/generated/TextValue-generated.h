// XXX Automatically generated. DO NOT EDIT! XXX //

public:
TextValue();
TextValue(const TextValue&);
TextValue(const QCString&);
TextValue & operator = (TextValue&);
TextValue & operator = (const QCString&);
bool operator ==(TextValue&);
bool operator !=(TextValue& x) {return !(*this==x);}
bool operator ==(const QCString& s) {TextValue a(s);return(*this==a);} 
bool operator != (const QCString& s) {return !(*this == s);}

virtual ~TextValue();
void parse() {if(!parsed_) _parse();parsed_=true;assembled_=false;}

void assemble() {if(assembled_) return;parse();_assemble();assembled_=true;}

void _parse();
void _assemble();
const char * className() const { return "TextValue"; }

// End of automatically generated code           //
