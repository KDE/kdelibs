var browser = new Widget("QTextBrowser", this);
browser.html = '<html><a href="qt://test">Test Link</a></html>';
connect( browser, "anchorClicked(const QUrl&)", this, "click(const QUrl&)");

browser.show();
exec();

function click( url )
{
	println("Clicked " + url );
}
