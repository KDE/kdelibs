var console = new Form("console.ui", this);
connect( console.commandLine, "returnPressed()", this, "callCommand()" );
console.show();
exec();

function callCommand( )
{
	console.outputConsole.html += console.commandLine.text + "<HR>";
	console.commandLine.text = "";
}
