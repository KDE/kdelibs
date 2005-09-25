#! /usr/bin/env python

import re

def exists(env):
	return True

def generate(env):
	if env['HELP']: return

	from SCons.Tool import Tool
        Tool('qt4', ['./bksys']).generate(env)

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

		# the main KDE libraries - the naming is important here, see genobj.uselib
		('LIBPATH_KDECORE', ''),
		('LIBPATH_KIO', ''),
		('LIBPATH_KDEUI', ''),

		('CPPPATH_KDECORE', ''),
		('CPPPATH_KIO', ''),
		('CPPPATH_KDEUI', ''),

		('LIB_KDECORE', ''),
		('LIB_KIO', ''),
		('LIB_KDEUI', ''),

		('KCONFIGCOMPILER', ''),
		('DCOPIDL', ''), ('DCOPIDL2CPP', ''),
		('UIC3_PRE_INCLUDE', '')
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

		dest=open(env.join(env['_BUILDDIR_'], 'config-kde.h'), 'w')
		dest.write('/* kde configuration */\n')
		dest.write(('#define KDELIBSUFF "%s"\n') % env['LIBSUFFIXEXT']);
		dest.close()
		env['_CONFIG_H_'].append('kde')
		
	        env['KDE4_ISCONFIGURED']=1
		env['UIC3_PRE_INCLUDE']="#include <kdialog.h>\n#include <klocale.h>\n"

		opts.Save(cachefile, env)
		
	import SCons.Defaults
	Builder=SCons.Builder.Builder

	creation_string = "%screating%s $TARGET.path" % (env['BKS_COLORS']['BLUE'], env['BKS_COLORS']['NORMAL'])

        ## KIDL file
	kidl_str='$DCOPIDL $SOURCE > $TARGET || (rm -f $TARGET ; false)'
	if not env['_USECOLORS_']: creation_string=""
	kidl_action=env.Action(kidl_str, creation_string)
        env['BUILDERS']['Kidl']=Builder(action=kidl_action, suffix='.kidl', src_suffix='.h')
        ## DCOP
	dcop_str='$DCOPIDL2CPP --c++-suffix cpp --no-signals --no-stub $SOURCE'
	dcop_action=env.Action(dcop_str, creation_string)
        env['BUILDERS']['Dcop']=Builder(action=dcop_action, suffix='_skel.cpp', src_suffix='.kidl')
        ## STUB
	stub_str='$DCOPIDL2CPP --c++-suffix cpp --no-signals --no-skel $SOURCE'
	stub_action=env.Action(stub_str, creation_string)
        env['BUILDERS']['Stub']=Builder(action=stub_action, suffix='_stub.cpp', src_suffix='.kidl')

	## DOCUMENTATION
	env['BUILDERS']['Meinproc']=Builder(action='$MEINPROC --check --cache $TARGET $SOURCE',suffix='.cache.bz2')

	def kcfg_buildit(target, source, env):
		comp='$KCONFIGCOMPILER -d%s %s %s' % (str(target[0].get_dir()), source[1].path, source[0].path)
		return env.Execute(comp)
	
	def kcfg_stringit(target, source, env):
		print "processing %s to get %s and %s" % (source[0].name, target[0].name, target[1].name)
		
	def kcfgEmitter(target, source, env):
		adjustixes = SCons.Util.adjustixes
		bs = SCons.Util.splitext(str(source[0].name))[0]
		bs = os.path.join(str(target[0].get_dir()),bs)
		# .h file is already there
		target.append(bs+'.cpp')

		if not os.path.isfile(str(source[0])):
			lenv.pprint('RED','kcfg file given'+str(source[0])+' does not exist !')
			return target, source
		kfcgfilename=""
		kcfgFileDeclRx = re.compile("^[fF]ile\s*=\s*(.+)\s*$")
		for line in file(str(source[0]), "r").readlines():
			match = kcfgFileDeclRx.match(line.strip())
			if match:
				kcfgfilename = match.group(1).strip()
				break
		if not kcfgfilename:
			print 'invalid kcfgc file'
			return 0
		source.append(  env.join(str(source[0].get_dir()), kcfgfilename)  )
		return target, source

	env['BUILDERS']['Kcfg']=Builder(action=env.Action(kcfg_buildit, kcfg_stringit),
			emitter=kcfgEmitter, suffix='.h', src_suffix='.kcfgc')


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

	kcfg_ext = ['.kcfgc']
	header_ext = [".h", ".hxx", ".hpp", ".hh"]
	skel_ext = [".skel", ".SKEL"]
	stub_ext = [".stub", ".STUB"]

	def KDEfiles(lenv, target, source):
		""" Returns a list of files for scons (handles kde tricks like .skel) 
		It also makes custom checks against double includes like : ['file.ui', 'file.cpp']
		(file.cpp is already included because of file.ui) """

		q_object_search = re.compile(r'[^A-Za-z0-9]Q_OBJECT[^A-Za-z0-9]')
		def scan_moc(cppfile):
			addfile=None

			# try to find the header
			orifile=cppfile.srcnode().name
			bs=SCons.Util.splitext(orifile)[0]

			h_file=''
			dir=cppfile.dir
			for n_h_ext in header_ext:
				afile=dir.File(bs+n_h_ext)
				if afile.rexists():
					#h_ext=n_h_ext
					h_file=afile
					break
			# We have the header corresponding to the cpp file
			if h_file:
				h_contents = h_file.get_contents()
				if q_object_search.search(h_contents):
					# we know now there is Q_OBJECT macro
					reg = '\n\s*#include\s*("|<)'+str(bs)+'.moc("|>)'
					meta_object_search = re.compile(reg)
					#cpp_contents = open(file_cpp, 'rb').read()
					cpp_contents=cppfile.get_contents()
					if meta_object_search.search(cpp_contents):
						lenv.Moc(h_file)
					else:
						lenv.Moccpp(h_file)
						addfile=bs+'_moc.cpp'
						print "WARNING: moc.cpp for "+h_file.name+" consider using #include <file.moc> instead"
			return addfile
		src=[]
		kcfg_files=[]
		other_files=[]
		kidl=[]

		source_=lenv.make_list(source)

		# For each file, check wether it is a dcop file or not, and create the complete list of sources
		for file in source_:

			sfile=SCons.Node.FS.default_fs.File(str(file)) # why str(file) ? because ordinal not in range issues
			bs  = SCons.Util.splitext(file)[0]
			ext = SCons.Util.splitext(file)[1]
			if ext in skel_ext:
				if not bs in kidl:
					kidl.append(bs)
				lenv.Dcop(bs+'.kidl')
				lenv.Depends(bs+'_skel.cpp', lenv['DCOPIDL2CPP'])
				src.append(bs+'_skel.cpp')
			elif ext in stub_ext:
				if not bs in kidl:
					kidl.append(bs)
				lenv.Stub(bs+'.kidl')
				lenv.Depends(bs+'_stub.cpp', lenv['DCOPIDL2CPP'])
				src.append(bs+'_stub.cpp')
			elif ext == ".moch":
				lenv.Moccpp(bs+'.h')
				src.append(bs+'_moc.cpp')
			elif ext in kcfg_ext:
				name=SCons.Util.splitext(sfile.name)[0]
				hfile=lenv.Kcfg(file)
				cppkcfgfile=sfile.dir.File(bs+'.cpp')
				lenv.Depends(bs+'.cpp', lenv['KCONFIGCOMPILER'])
				src.append(bs+'.cpp')
			elif ext == ".dummy":
				lenv.EmptyFile(bs)
				src.append('dummy_'+bs+'.cpp')
			else:
				src.append(file)

		for base in kidl: lenv.Kidl(base+'.h')
		
		# Now check against typical newbie errors
		for file in kcfg_files:
			for ofile in other_files:
				if ofile == file:
					env.pprint('RED',"WARNING: You have included %s.kcfg and another file of the same prefix"%file)
					print "Files generated by kconfig_compiler (settings.h, settings.cpp) must not be included"
		return src

	#valid_targets = "program convenience shlib kioslave staticlib".split()
	#import qt4.generate.qt4obj
	from SCons.Script.SConscript import SConsEnvironment
	class kdeobj(SConsEnvironment.qt4obj):
		def __init__(self, val, senv=None):
			if senv: SConsEnvironment.qt4obj.__init__(self, val, senv)
			else: SConsEnvironment.qt4obj.__init__(self, val, env)
			#self.iskdelib=0
		def it_is_a_kdelib(self): self.iskdelib=1
		def execute(self):
			if self.executed: return
			if self.orenv.has_key('DUMPCONFIG'):
				self.executed=1
				self.xml()
				return
			if (self.type=='shlib' or self.type=='kioslave'):
				if self.iskdelib==1: self.instdir=self.orenv.getInstDirForResType('KDELIB')
				else:                self.instdir=self.orenv.getInstDirForResType('KDEMODULE')
			elif self.type=='program':
				self.instdir=self.orenv.getInstDirForResType('KDEBIN')
				self.perms=0755

			self.env=self.orenv.Copy()

			self.source = KDEfiles(self.env, self.joinpath(self.target), self.joinpath(self.source))
			SConsEnvironment.qt4obj.execute(self)

		def xml(self):
			dirprefix = reldir('.')
			if not dirprefix: dirprefix=self.dirprefix
			ret='<compile type="%s" dirprefix="%s" target="%s" cxxflags="%s" cflags="%s" includes="%s" linkflags="%s" libpaths="%s" libs="%s" vnum="%s" iskdelib="%s" libprefix="%s">\n' % (self.type, dirprefix, self.target, self.cxxflags, self.cflags, self.includes, self.linkflags, self.libpaths, self.libs, self.vnum, self.iskdelib, self.libprefix)
			if self.source:
				for i in self.orenv.make_list(self.source): ret+='    <source file="%s"/>\n' % i
			ret+="</compile>\n"
			self.orenv.add_dump(ret)

	from SCons.Script.SConscript import SConsEnvironment
	SConsEnvironment.KDEicon = KDEicon
	SConsEnvironment.kdeobj = kdeobj
