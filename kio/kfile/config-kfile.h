#ifndef CONFIG_KFILE_H
#define CONFIG_KFILE_H

const int kfile_area = 250;

#define DefaultViewStyle QString::fromLatin1("SimpleView")
#define DefaultPannerPosition 40
#define DefaultMixDirsAndFiles false
#define DefaultShowStatusLine false
#define DefaultShowHidden false
#define DefaultCaseInsensitive true
#define DefaultDirsFirst true
#define DefaultSortReversed false
#define DefaultRecentURLsNumber 15
#define DefaultDirectoryFollowing true
#define ConfigGroup QString::fromLatin1("KFileDialog Settings")
#define RecentURLs QString::fromLatin1("Recent URLs")
#define RecentFiles QString::fromLatin1("Recent Files")
#define RecentURLsNumber QString::fromLatin1("Maximum of recent URLs")
#define RecentFilesNumber QString::fromLatin1("Maximum of recent files")
#define DialogWidth QString::fromLatin1("Width (%1)")
#define DialogHeight QString::fromLatin1("Height (%1)")
#define ConfigShowStatusLine QString::fromLatin1("ShowStatusLine")
#define AutoDirectoryFollowing QString::fromLatin1("Automatic directory following")
#define PathComboCompletionMode QString::fromLatin1("PathCombo Completionmode")
#define LocationComboCompletionMode QString::fromLatin1("LocationCombo Completionmode")
#define ShowSpeedbar QString::fromLatin1("Show Speedbar")

#endif
