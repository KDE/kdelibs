# README for KDE's Doxygen tools

Writing dox is beyond the scope of this README -- see the notes at
<http://techbase.kde.org/Policies/Library_Documentation_Policy>.
You can generate dox by hand -- without even having a configured
build directory -- as explained below. There is also documentation
for the special tags you can enter in Makefile.am anywhere
in a module to modify dox generation.

## Generating the documentation

The tool for generating dox is `src/doxygen.sh`.

Run it in the "top builddir" with one argument, the "top srcdir". Something
like this:

    cd /mnt/build/kdepim
    /path/to/apidox/src/doxygen.sh /mnt/src/kdepim

You can also build single subdirs (for instance, after updating some
dox and you don't want to rebuild for the entire module) by giving the
subdirectory _relative to the top srcdir_ as a second argument:

    /path/to/apidox/src/doxygen.sh /mnt/src/kdepim kpilot/lib

When generating dox for kdelibs, a tag file for Qt is also created.
The location of Qt is specified indirectly through $QTDOCDIR or,
if that is not set, $QTDIR, or otherwise guessed. You may explicitly
set the location of a pre-generated tag file with $QTDOCTAG. One
typical approach might be:

    QTDOCTAG=$QTDIR/doc/qt.tag QTDOCDIR=http://doc.trolltech.com/3.3/

Finally, there is a --no-recurse option for top-level generation
that avoids generating all the subdirectories as well. It also
suppresses cleaning up (rm -rf) of the dox direction beforehand.

Post-finally, there is a --no-modulename option that builds the
dox in "apidocs/" instead of "$(modulename)-apidocs". The former is
compatible with the KDE 3.4 build system, the latter is more convenient
for the installed dox.

Typically, this means $(top_builddir)/apidocs and something like
libfoo/html for the output. For the top-level dir, set relative-html
to "." .

### Placeholders

In non-top directories, both `<!-- menu -->` and `<!-- gmenu -->` are
calculated and replaced. Top directories get an empty `<!-- menu -->` if any.

`<!-- api_searchbox -->` gets replaced by api.kde.org's search box (actually,
by the contents of `api_searchbox.html`).

## "common" directory

The common directory contains data files for Doxygen generation. These are the
GENERIC files; any module may override them by putting specific replacements in
doc/api/ .  This allows modules to customize their appearance as desired.  The
files that may be overridden are:

- `api_searchbox.html`:
    Optional search box displayed after the rest of the footer menu. Turned on
    by --api-searchbox.

- `doxygen.css`:
    Stylesheet.

- `mainheader.html`:
    Header for front page of dox. This should not be terribly different from
    header.html.  It might contain special CSS for the footer.

- `mainfooter.html`:
    Footer for front page of dox. This should at least credit Doxygen [1] and
    point to the dox guidelines [2].

- `header.html`:
    Header file for regular pages.

- `footer.html`:
    Footer file for regular pages.

- `Doxyfile.global`:
    The global (brief) Doxyfile. For a long-style Doxyfile, see KDE PIM's
    doc/api/Doxyfile.pim.
