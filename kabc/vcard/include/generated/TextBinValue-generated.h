// XXX Automatically generated. DO NOT EDIT! XXX //

public:
TextBinValue();
TextBinValue(const TextBinValue&);
TextBinValue(const QCString&);
TextBinValue & operator = (TextBinValue&);
TextBinValue & operator = (const QCString&);
bool operator ==(TextBinValue&);
bool operator !=(TextBinValue& x) {return !(*this==x);}
bool operator ==(const QCString& s) {TextBinValue a(s);return(*this==a);} 
bool operator != (const QCString& s) {return !(*this == s);}

virtual ~TextBinValue();
void parse() {if(!parsed_) _parse();parsed_=true;assembled_=false;}

void assemble() {if(assembled_) return;parse();_assemble();assembled_=true;}

void _parse();
void _assemble();
const char * className() const { return "TextBinValue"; }

// End of automatically generated code           //
