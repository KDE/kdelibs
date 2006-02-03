var box = new Widget("QFrame", 0);
var layout = new Layout("QVBoxLayout", box);

for ( var x = 0; x < 10; ++x)
{
	var lab = new Widget("QLabel", box);
	lab.text = "This is a test of box " + x;
	layout.addWidget(lab);
}

box.adjustSize();
box.show();

exec();
