#ifndef KFILE_H
#define KFILE_H


/**
 * Just a namespace for some enums
 */

class KFile
{
public:
    /**
     * Modes of operation for the dialog.
     * @li @p File - Get a single file name from the user.
     * @li @p Directory - Get a directory name from the user.
     * @li @p Files - Get multiple file names from the user.
     * @li @p ExistingOnly - Never return a filename which does not exist yet
     * @li @p LocalOnly - Don't return remote filenames
     */
    enum Mode {
	File         = 1,
	Directory    = 2,
	Files        = 4,
	ExistingOnly = 8,
	LocalOnly    = 16
    };

    enum FileView {
	Default         = 0,
	Simple          = 1,
	Detail          = 2,
	PreviewContents = 4,
	PreviewInfo     = 8,
	SeparateDirs    = 16
    };

    enum SelectionMode {
	Single      = 1,
	Multi       = 2,
	Extended    = 4,
	NoSelection = 8
    };


    /**
     * Increasing - greater indices mean bigger values
     * Decrease -  greater indicies mean smaller values
     */
    enum SortMode {
	Increasing,
	Decreasing
    };

};

#endif // KFILE_H
