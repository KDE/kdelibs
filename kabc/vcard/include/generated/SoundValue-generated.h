// XXX Automatically generated. DO NOT EDIT! XXX //

public:
SoundValue();
SoundValue(const SoundValue&);
SoundValue(const QCString&);
SoundValue & operator = (SoundValue&);
SoundValue & operator = (const QCString&);
bool operator ==(SoundValue&);
bool operator !=(SoundValue& x) {return !(*this==x);}
bool operator ==(const QCString& s) {SoundValue a(s);return(*this==a);} 
bool operator != (const QCString& s) {return !(*this == s);}

virtual ~SoundValue();
void parse() {if(!parsed_) _parse();parsed_=true;assembled_=false;}

void assemble() {if(assembled_) return;parse();_assemble();assembled_=true;}

void _parse();
void _assemble();
const char * className() const { return "SoundValue"; }

// End of automatically generated code           //
