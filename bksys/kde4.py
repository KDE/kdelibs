#! /usr/bin/env python

def exists(env):
	return True

def generate(env):
	if env['HELP']: return

        from SCons.Options import Options
        cachefile=env['CACHEDIR']+'kde4.cache.py'
        opts = Options(cachefile)
        opts.AddOptions(
                ('KDE4_ISCONFIGURED', ''),

                ('KDEDIR', ''),
                ('KDELIBPATH', 'path to the installed kde libs'),
                ('KDEINCLUDEPATH', 'path to the installed kde includes'),

		('LIBPATH_KDE4', ''),
		('RPATH_KDE4', ''),
		('INCLUDES_KDE4', ''),

                ('KDEBIN', 'inst path of the kde binaries'),
                ('KDEINCLUDE', 'inst path of the kde include files'),
                ('KDELIB', 'inst path of the kde libraries'),
                ('KDEMODULE', 'inst path of the parts and libs'),
                ('KDEDATA', 'inst path of the application data'),
                ('KDELOCALE', ''), ('KDEDOC', ''), ('KDEKCFG', ''),
                ('KDEXDG', ''), ('KDEXDGDIR', ''), ('KDEMENU', ''),
                ('KDEMIME', ''), ('KDEICONS', ''), ('KDESERV', ''),
                ('KDESERVTYPES', ''), ('KDEAPPS', ''),
        )
        opts.Update(env)

        if not env['HELP'] and (env['_CONFIGURE_'] or not env.has_key('KDE4_ISCONFIGURED')):
                env['_CONFIGURE_']=1
                import sys
                import os
                if sys.platform == 'darwin':
                        #sys.path.append('bksys'+os.sep+'osx')
                        #from detect_kde4 import detect
                        env.pprint('RED', 'Not implemented, see bksys/osx/detect_kde4.py')
                        env.Exit(1)
                else:
                        sys.path.append('bksys'+os.sep+'unix')
                        from detect_kde4 import detect
                detect(env)

	        env['KDE4_ISCONFIGURED']=1
		opts.Save(cachefile, env)

	import SCons.Defaults
	Builder=SCons.Builder.Builder

        ## KIDL file
        env['BUILDERS']['Kidl']=Builder(action= 'dcopidl $SOURCE > $TARGET || (rm -f $TARGET ; false)',
                        suffix='.kidl', src_suffix='.h')
        ## DCOP
        env['BUILDERS']['Dcop']=Builder(action='dcopidl2cpp --c++-suffix cpp --no-signals --no-stub $SOURCE',
                        suffix='_skel.cpp', src_suffix='.kidl')
        ## STUB
        env['BUILDERS']['Stub']=Builder(action= 'dcopidl2cpp --c++-suffix cpp --no-signals --no-skel $SOURCE',
                        suffix='_stub.cpp', src_suffix='.kidl')


        def KDEicon(lenv, icname='*', path='./', restype='KDEICONS', subdir=''):
                """Installs icons with filenames such as cr22-action-frame.png into
                KDE icon hierachy with names like icons/crystalsvg/22x22/actions/frame.png.

                Global KDE icons can be installed simply using env.KDEicon('name').
                The second parameter, path, is optional, and specifies the icons
                location in the source, relative to the SConscript file.

                To install icons that need to go under an applications directory (to
                avoid name conflicts, for example), use e.g.
                env.KDEicon('name', './', 'KDEDATA', 'appname/icons')"""

                if lenv.has_key('DUMPCONFIG'):
                        lenv.add_dump( "<icondir>\n" )
                        lenv.add_dump( "    <icondirent dir=\"%s\" subdir=\"%s\"/>\n" % (reldir(path), subdir) )
                        lenv.add_dump( "</icondir>\n" )
                        return

                type_dic = { 'action':'actions', 'app':'apps', 'device':'devices',
                        'filesys':'filesystems', 'mime':'mimetypes' }
                dir_dic = {
                'los'  :'locolor/16x16', 'lom'  :'locolor/32x32',
                'him'  :'hicolor/32x32', 'hil'  :'hicolor/48x48',
                'lo16' :'locolor/16x16', 'lo22' :'locolor/22x22', 'lo32' :'locolor/32x32',
                'hi16' :'hicolor/16x16', 'hi22' :'hicolor/22x22', 'hi32' :'hicolor/32x32',
                'hi48' :'hicolor/48x48', 'hi64' :'hicolor/64x64', 'hi128':'hicolor/128x128',
                'hisc' :'hicolor/scalable',
                'cr16' :'crystalsvg/16x16', 'cr22' :'crystalsvg/22x22', 'cr32' :'crystalsvg/32x32',
                'cr48' :'crystalsvg/48x48', 'cr64' :'crystalsvg/64x64', 'cr128':'crystalsvg/128x128',
                'crsc' :'crystalsvg/scalable'
                }

                iconfiles = []
                dir=SCons.Node.FS.default_fs.Dir(path).srcnode()
                mydir=SCons.Node.FS.default_fs.Dir('.')
                import glob
                for ext in ['png', 'xpm', 'mng', 'svg', 'svgz']:
                        files = glob.glob(str(dir)+'/'+'*-*-%s.%s' % (icname, ext))
                        for file in files:
                                iconfiles.append( file.replace(mydir.abspath, '') )
                for iconfile in iconfiles:
                        lst = iconfile.split('/')
                        filename = lst[ len(lst) - 1 ]
                        tmp = filename.split('-')
                        if len(tmp)!=3:
                                env.pprint('RED','WARNING: icon filename has unknown format: '+iconfile)
                                continue
                        [icon_dir, icon_type, icon_filename]=tmp
                        try:
                                basedir=getInstDirForResType(lenv, restype)
                                destdir = '%s/%s/%s/%s/' % (basedir, subdir, dir_dic[icon_dir], type_dic[icon_type])
                        except KeyError:
                                env.pprint('RED','WARNING: unknown icon type: '+iconfile)
                                continue
                        lenv.bksys_install(destdir, iconfile, icon_filename)

	from SCons.Script.SConscript import SConsEnvironment
	SConsEnvironment.KDEicon = KDEicon

