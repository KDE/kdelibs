// XXX Automatically generated. DO NOT EDIT! XXX //

public:
EmailParam();
EmailParam(const EmailParam&);
EmailParam(const QCString&);
EmailParam & operator = (EmailParam&);
EmailParam & operator = (const QCString&);
bool operator ==(EmailParam&);
bool operator !=(EmailParam& x) {return !(*this==x);}
bool operator ==(const QCString& s) {EmailParam a(s);return(*this==a);} 
bool operator != (const QCString& s) {return !(*this == s);}

virtual ~EmailParam();
void parse() {if(!parsed_) _parse();parsed_=true;assembled_=false;}

void assemble() {if(assembled_) return;parse();_assemble();assembled_=true;}

void _parse();
void _assemble();
const char * className() const { return "EmailParam"; }

// End of automatically generated code           //
