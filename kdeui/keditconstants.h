/*
 * $Id$
 */

/*
 * some definitions for load/save/saveAs error handling
 *
 * note, I prefix everthing with KEDIT_, mainly because of:
 *       some other people may want to use the KEdit class for other projects.
 *       the prefix makes sure, that we won't screw up some other #defines
 *
 */

#define KEDIT_ID_FILE_OPEN 1
#define KEDIT_ID_FILE_OPEN_URL 2
#define KEDIT_ID_FILE_SAVE 3
#define KEDIT_ID_FILE_SAVEAS 4
#define KEDIT_ID_FILE_SAVE_URL 5
#define KEDIT_ID_FILE_CLOSE 6
#define KEDIT_ID_FILE_NEWWIN 7
#define KEDIT_ID_FILE_QUIT 8

#define KEDIT_ID_EDIT_SEARCH 50
#define KEDIT_ID_EDIT_SEARCHAGAIN 51

#define KEDIT_ID_HELP_ABOUT 100
#define KEDIT_ID_HELP_HELP  101
