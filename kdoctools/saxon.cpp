#include <stdio.h>
#include <kinstance.h>
#include <kstddirs.h>
#include <xslt.h>
#include <qfile.h>

int main(int argc, char **argv)
{
    if (argc != 2) {
        fprintf(stderr, "usage: %s XML\n", argv[0]);
        return 1;
    }

    KInstance ins("saxon");
    fillInstance(ins);

    QString command = QString::fromLatin1("java com.icl.saxon.StyleSheet %1 %2").arg(argv[1]).arg(locate("dtd", "customization/kde-chunk.xsl"));

    QString output;

    FILE *pipe = popen(command.latin1(), "r");
    QFile xmlFile;
    xmlFile.open(IO_ReadOnly, pipe);
    char buffer[5001];

    int len;
    while ((len = xmlFile.readBlock(buffer, 5000)) != 0) {
        buffer[len] = 0;
        output += buffer;
    }
    xmlFile.close();
    pclose(pipe);

    if (output.isEmpty()) {
        fprintf(stderr, "unable to parse %s\n", argv[1]);
        return(1);
    }

    int index = 0;
    while (true) {
        index = output.find("<FILENAME ", index);
        if (index == -1)
            break;
        int filename_index = index + strlen("<FILENAME filename=\"");

        QString filename = output.mid(filename_index,
                                      output.find("\"", filename_index) -
                                      filename_index);

        QString filedata = splitOut(output, index);
        QFile file(filename);
        file.open(IO_WriteOnly);
        file.writeBlock(filedata.latin1(), filedata.length());
        file.close();

        index += 8;
    }
}
