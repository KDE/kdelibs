#!/usr/bin/env kjscmd

// Load the view
var uiLoader = new QUiLoader();
var viewer = uiLoader.load('docviewer.ui', this);
var text = viewer.findChild('textBrowser');

// Uncommenting pretty much any bit of this crashes it...
/* doLink = function(url) { */
/*   workaround = url.toString(); */
/*   workaround = workaround.slice( workaround.lastIndexOf('#') ); */
/*   println('XXX'+workaround); */
/*   text.scrollToAnchor( workaround ); */
/* } */

/* url = new QUrl(); // Even just this line alone can crash... */
/* println('OK' + url); */
/* text.setSource(url); */
/* text.connect(text, 'anchorClicked(const QUrl &)', this, 'doLink()'); */

var html = '<h1>KJSEmbed Documentation</h1>';

// Generate TOC
html += '<h2>Contents</h2><ul>'
for( i in this ) {
    html += '<li><a href="' + i + '">' + i + '</a>';
}
html += '</ul>';

// Object Docs
for( var i in this ) {
    println( 'Doing ' + i );
    if ( i == 'connect' || i == 'Application' || i == 'viewer' || i == 'text'
	 || i == 'html' || i == 'inst' || i == 'uiLoader' )
      continue;

    html += '<a name="' + i + '"><h2>' + i + '</h2></a><ul>';

    // Create an instance
    try {
      var inst = eval( 'new ' + i + '()' );
    
      for( j in inst ) {
	html += '<li>' + j;
      }
      html += '</ul>';
    }
    catch( err ) {
      html += '<li>Could not create</ul>';
    }
}

text.html = html;

viewer.show();
exec();
