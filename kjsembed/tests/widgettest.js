var button = new Widget("QPushButton", this );
button.height = 100;
button.width = 100;
button.text = "Test";
print("Object " + button );
for( y in button )
        print("Button method: " + y);

button.show();
exec();
