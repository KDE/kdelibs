
var doc = new QDomDocument("MyXML");
var root = doc.createElement("MyXML");
doc.appendChild(root);

var tag = doc.createElement("Greeting");
tag.setAttribute("Property", "Hello World");
root.appendChild( tag );

var text = doc.createTextNode("SomeText");
text.setNodeValue("Some text value");
tag.appendChild( text );

println("Generated doc: " + doc);

var myxml = "<MyXML><Greeting Property=\"Hello World\"/></MyXML>";
var newDoc = new QDomDocument("MyXML");
newDoc.setContent(myxml);
newDoc.documentElement().setAttribute("Property", "Goodbye World");
println("Read doc:" + newDoc );


var nodeList = root.elementsByTagName( "Greeting" );
for( x = 0; x < nodeList.length(); ++x)
{
	var elem =  nodeList.item(x).toElement();
	elem.setAttribute("New Property", 100 );
	elem.setAttribute("Next Property", new QColor("blue") );
}

println("Modified doc:" + doc );
