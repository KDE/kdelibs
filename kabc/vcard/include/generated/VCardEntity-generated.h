// XXX Automatically generated. DO NOT EDIT! XXX //

public:
VCardEntity();
VCardEntity(const VCardEntity&);
VCardEntity(const QCString&);
VCardEntity & operator = (VCardEntity&);
VCardEntity & operator = (const QCString&);
bool operator ==(VCardEntity&);
bool operator !=(VCardEntity& x) {return !(*this==x);}
bool operator ==(const QCString& s) {VCardEntity a(s);return(*this==a);} 
bool operator != (const QCString& s) {return !(*this == s);}

virtual ~VCardEntity();
void parse() {if(!parsed_) _parse();parsed_=true;assembled_=false;}

void assemble() {if(assembled_) return;parse();_assemble();assembled_=true;}

void _parse();
void _assemble();
const char * className() const { return "VCardEntity"; }

// End of automatically generated code           //
