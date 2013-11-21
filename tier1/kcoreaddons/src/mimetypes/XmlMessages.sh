function get_files
{
    echo kde5.xml
}

function po_for_file
{
    case "$1" in
       kde5.xml)
           echo xml_mimetypes.po
       ;;
    esac
}

function tags_for_file
{
    case "$1" in
       kde5.xml)
           echo comment
       ;;
    esac
}

