function equalStrings(lhs, rhs) {
    return lhs == rhs
}

function equalArrays(lhs, rhs) {
    if (lhs.length != rhs.length) {
        return false;
    }

    for (var ix = 0; ix < lhs.length; ++ix) {
        if (typeof lhs[ix] != typeof rhs[ix]) {
            return false;
        }
        
        if (lhs[ix] != rhs[ix]) {
            return false
        }
    }

    return true;
}

function mimeTypeName() {
    return "image/png"
}

function equalMimeTypeName(lhs, rhs) {
    return equalStrings(lhs, rhs)
}

//function mimeTypeAliases() {
//    return [ "picture/png" ]
//}

//function equalMimeTypeAliases(lhs, rhs) {
//    return equalArrays(lhs, rhs)
//}

//function mimeTypeComment() {
//    return "PNG File";
//}

//function equalMimeTypeComment(lhs, rhs) {
//    return equalStrings(lhs, rhs)
//}

function mimeTypeGenericIconName() {
    return "/usr/share/icons/oxygen/64x64/mimetypes/image-x-generic.png"
}

function equalMimeTypeGenericIconName(lhs, rhs) {
    return equalStrings(lhs, rhs)
}

function mimeTypeIconName() {
    return "/usr/share/icons/oxygen/64x64/mimetypes/image-x-generic.png"
}

function equalMimeTypeIconName(lhs, rhs) {
    return equalStrings(lhs, rhs)
}

function mimeTypeGlobPatterns() {
    return [ "*.png" ]
}

function equalMimeTypeGlobPatterns(lhs, rhs) {
    return equalArrays(lhs, rhs)
}

function mimeTypeSuffixes() {
    return [ ".png" ]
}

function equalMimeTypeSuffixes(lhs, rhs) {
    return equalArrays(lhs, rhs)
}
