// Bootstrap the JS

Document.html = 'Hello JS World!!!!!';
Document.setReadOnly( true );
Document.append( 'still here?' );
Document.append( Document.textColor() );
Document.setTextColor('#ccaacc');
Document.append( 'still here?' );
Document.append( Document.textColor() );
Document.append( Document.currentFont() );
println( 'xxx' );
println( Document.toPlainText() );
