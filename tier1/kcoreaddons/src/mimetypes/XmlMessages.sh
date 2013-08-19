function get_files
{
    echo kde.xml
}

function po_for_file
{
    case "$1" in
       kde.xml)
           echo xml_mimetypes.po
       ;;
    esac
}

function tags_for_file
{
    case "$1" in
       kde.xml)
           echo comment
       ;;
    esac
}

