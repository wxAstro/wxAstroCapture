#!/bin/bash
#
# EPM Package Build Script
# must be run with fakeroot  !!!
# i.e.   $> fakeroot bash ./wxAstroCapture.sh
# 20070528/BM
# 20080919/BM update for i386 _and_ amd64 architecture
# 20081206/BM derived from debian script for EPM
# 20090308/BM added rules files of libusb usage
# 20091107/BM add pre/post inst for udev processing under with udevadm (if exist)
# 20091110/BM fixed udevadm mess
# 20100709/BM added -g to epm to explicitely avoid stripping
# 20100709/BM fixed rpmdefault dirs that have changed since 8.04 (WHY?????)
# 20101023/BM added rpmdefault package build legacy flag in rpm macro section
#
#
# assuming the following folder and file structure
# ./UX
#    ./deb_root
#
# ../doc/readme.txt	whatever may help
# ../doc/license.txt	the copyright text
# ../doc/relnotes.txt	!!! Debian conform changelog (user date -R for the date stamp)
#
# will produce a Debian package  PACKAGE_VERSION_TAG.deb
# i.e. wxWebcamDB_0.1-1_test.deb
#
#  expects:
#   epm installed
#   fltk libraries installed
#   type-handling installed
#   rpmbuild installed
#
# useful reference:
# http://tldp.org/HOWTO/html_single/Debian-Binary-Package-Building-HOWTO/
# http://www.grymoire.com/Unix/Sed.html
# http://www.epmhome.org/epm-book.html
#

## Program and Package name
# shown porgname may be mixed case only
PROGNAME=wxAstroCapture
# lowercase for packages and applications mandatory
BINNAME=wxastrocapture
PACKAGE=wxastrocapture
# entries in control file
MAINTAINER='C.Arnholm  M.Burri <wxAstroCapture@yahoogroups.com>'
#VERSION is taken from the changelog i.e. relnotes.txt file
#  make sure it is updtodate and according the rules !!!!


## Path of our files

#  ~/  means our home dir....

# File Source setup use env var WXASTROCAPTURE_HOME= to overwrite the given path
PROJPATH=${WXASTROCAPTURE_HOME-~/proj}/wxWebcamDB
# Docu folder from where to install
DOCPATH=${PROJPATH}/setup/doc
# Path to Binaries to install
BINPATH=${PROJPATH}/UX/bin/Release
DESTDIR=${PROJPATH}/setup/redist	# where to put the final package

# relative to setup/UX  path

BROOT=./deb_root    	# Build root folder name

# used to determine 32 or 64 bit architecture
MACHINE=`uname -m`
# fix i686 reply with a more generic i386 architecture
if [ "$MACHINE" = "i686" ] ; then 
	MACHINE="i386"
fi
ARCHITECTURE=`type-handling $MACHINE linux`
VERSION=0.0.0

# EPM target variables
epmfile=${PACKAGE}.list
srcdir=./debian
dstdir=./build
# newly used for RPM buidling
rpmmacrofile=~/.rpmmacros


prefix=/usr
exec_prefix=/usr
bindir=${exec_prefix}/bin
datadir=/usr/share
docdir=${datadir}/doc/${PACKAGE}
libdir=/usr/lib
mandir=/usr/share/man
menudir=${datadir}/menu
desktopdir=${datadir}/applications
icondir=${datadir}/icons



# derive relnotes.txt (changelog) from version.h
make_Relnotes()
{
	echo " create changelog now...."
	if [ ! -f "${PROJPATH}/version.h" ]; then
		echo "missing ${PROJPATH}/version.h"
		return -1
	fi
	# retrieve changelog content (output with DOS CRLF for Win builds
 	cat ${PROJPATH}/version.h | tr -d "\r" | \
	awk    '/^\/\/END[ ]*changelog*/{IT=0} \
		IT==1{print(substr($0,3))} \
		/^\/\/START[ ]*changelog*/{IT=1}' | awk '{sub(/$/,"\r");print}'  > ${DOCPATH}/relnotes.txt
	return $?
}

# derive VERSION form (changelog) from version.h
# #define MyAppVer "1.4-1"
make_Version()
{
	echo " derive VERSION now...."
	if [ ! -f "${PROJPATH}/version.h" ]; then
		echo "missing ${PROJPATH}/version.h"
		return -1
	fi
	# retrieve changelog content (output with DOS CRLF for Win builds
 	VERSION=`cat ${PROJPATH}/version.h | tr -d "\r\"" | \
	awk    '/^#define MyAppVer/{split($0,a);print(a[3])}' `
	echo " found Version as: ${VERSION}"

	return $?
}

# copy and name docu stuff as required for deb packages
make_Doc ()
{
	echo " copy docu files now...."
	# copy files, removing CRs (\r or 0x0D or \015 (octal) )
	if [ -f "${DOCPATH}/relnotes.txt" ]; then
		cat "${DOCPATH}/relnotes.txt" | tr -d "\r" > ${srcdir}/${PACKAGE}.changelog
	else
		echo "missing relnotes.txt file"
	fi
	
	if [ -f "${DOCPATH}/readme_ux.txt" ]; then
		cat "${DOCPATH}/readme_ux.txt" | tr -d "\r" > ${srcdir}/README.Debian
	else
		echo "missing readme_ux.txt file"
	fi
	
	if [ -f "${DOCPATH}/license.txt" ]; then
		cat "${DOCPATH}/license.txt" | tr -d "\r" > ${srcdir}/copyright
	else
		echo "missing license.txt file"
	fi
	
	echo " "
}


## copy the files needed (we are in build root now)
get_Files ()
{
# copy each file like the template
	echo " copy application files now...."
	if [ ! -f "${BINPATH}/wxAstroCapture" ]; then
	echo "E: get_Files(): could not find file - ${BINPATH}/wxAstroCapture"
	return -1  # did not find file ERROR EXIT
	fi
	cp "${BINPATH}/wxAstroCapture" ${srcdir}/${BINNAME}  	# copy into temp location
	
	echo "epm add - ${BINNAME}"
cat >> ${epmfile} <<EOF
f 755 root root ${bindir}/${BINNAME} ${srcdir}/${BINNAME} nostrip()
EOF
# next copy section

# regular exit
  return 0
}

## copy the files needed (we are in build root now)
get_RulesFile ()
{
rFile=$1
# copy each file like the template
echo " copy udev rules files now...."
	if [ ! -f "${PROJPATH}/${rFile}" ]; then
	echo "E: get_RulesFile(): could not find file - ${PROJPATH}/${rFile}"
	return -1  # did not find file ERROR EXIT
	fi
	cp "${PROJPATH}/${rFile}" ${srcdir}/${rFile}  	# copy into temp location
	
	echo "epm add - ${rFile}"
cat >> ${epmfile} <<EOF
f 644 root root /etc/udev/rules.d/${rFile} ${srcdir}/${rFile} nostrip()
EOF
# next copy section

# regular exit
  return 0
}

# http://standards.freedesktop.org/menu-spec/1.0/
make_Desktop()
{
# get program icon
	echo " copy icon files now...."
# http://standards.freedesktop.org/icon-theme-spec/latest/ar01s07.html
	cp ${DOCPATH}/${BINNAME}32x32.png ${srcdir}/${BINNAME}32.png
	cp ${DOCPATH}/${BINNAME}48x48.png ${srcdir}/${BINNAME}48.png
	cp ${DOCPATH}/${BINNAME}64x64.png ${srcdir}/${BINNAME}64.png

#  png2xpm ${srcdir}/${BINNAME}48.png > ${srcdir}/${BINNAME}48.xpm

#install files
	echo "epm add - ICONS"
cat >> ${epmfile} <<EOF
f 644 root root ${icondir}/hicolor/32x32/apps/${BINNAME}.png ${srcdir}/${BINNAME}32.png
f 644 root root ${icondir}/hicolor/48x48/apps/${BINNAME}.png ${srcdir}/${BINNAME}48.png
f 644 root root ${icondir}/hicolor/64x64/apps/${BINNAME}.png ${srcdir}/${BINNAME}64.png
EOF

# the app.desktop file that makes the menu entry
# http://standards.freedesktop.org/desktop-entry-spec/latest/ar01s05.html
	echo " create desktop file now...."
cat > ${srcdir}/${PACKAGE}.desktop <<EOF
[Desktop Entry]
Version=1.0
Encoding=UTF-8
Name=${PROGNAME}
Comment="wxAstroCapture is used for capturing deep sky and planetary objects with telescopes"
Exec=${BINNAME}
Terminal=false
Icon=${BINNAME}.png
X-MultipleArgs=false
Type=Application
Categories=Education;Science;Astronomy
EOF

#install file
	echo "epm add - DESKTOP"
cat >> ${epmfile} <<EOF
f 644 root root ${desktopdir}/${PACKAGE}.desktop ${srcdir}/${PACKAGE}.desktop
EOF
}


# write up the X11 menu string used for that package (see Debian menufile(5) )
make_Menu()
{
#make menu entry
	echo " create menu file now...."
cat > ${srcdir}/${PACKAGE}.menu <<EOF
?package(${PACKAGE}):needs=X11 section=Apps/Science title="${PROGNAME}" command="/usr/bin/${BINNAME}"
EOF

#install file
	echo "epm add - MENU"
cat >> ${epmfile} <<EOF
f 644 root root ${menudir}/${PACKAGE} ${srcdir}/${PACKAGE}.menu
EOF
}

# make the control file inplace here
make_Control()
{
	echo " making the EPM control file now...."
# EPM file entries:
# type mode owner group destination source options  (EPM lines)
# f 755 root sys /usr/bin/foo foo
# c 644 root sys /etc/foo.conf foo.conf
# d 755 root sys /var/spool/foo -
# l 000 root sys /usr/bin/foobar foo
# f 0444 root sys /usr/share/doc/foo *.html

	echo "epm add - main EPM tags"
cat > ${epmfile} <<EOF
%product ${PACKAGE}
%copyright 2010 by ${MAINTAINER}, All Rights Reserved.
%vendor ${MAINTAINER}
%license ${srcdir}/copyright
%readme ${srcdir}/README.Debian
%description wxWidgets based Webcam Capture Program. wxAstroCapture is used for capturing deep sky and planetary objects with telescopes.
%version ${VERSION}
EOF

#install doc files
	echo "epm add - DOC"
cat >> ${epmfile} <<EOF
f 644 root root ${docdir}/changelog      ${srcdir}/${PACKAGE}.changelog
f 644 root root ${docdir}/copyright      ${srcdir}/copyright
f 644 root root ${docdir}/README.Debian  ${srcdir}/README.Debian
EOF

}


# make additions to the control file inplace here
# note the two flavors of udev handling ... GRRRRRRR
# newer Linux have no longer udevcontrol
make_PostActions()
{
#postinstall 
	echo "epm add - postinstall"
cat >> ${epmfile} <<EOF
%postinstall <<EOFX
if [ -x "\`which update-menus 2>/dev/null\`" ]; then update-menus ; fi
if [ -x "\`which udevcontrol 2>/dev/null\`" ]; then 
	udevcontrol --reload_rules 2>/dev/null
elif [ -x "\`which udevadm 2>/dev/null\`" ]; then 
	udevadm control --reload_rules 2>/dev/null
	udevadm control --reload-rules 2>/dev/null
fi
exit 0
EOFX
EOF

#postremove 
	echo "epm add - postremove"
cat >> ${epmfile} <<EOF
%postremove <<EOFX
if [ -x "\`which update-menus 2>/dev/null\`" ]; then update-menus ; fi
if [ -x "\`which udevcontrol 2>/dev/null\`" ]; then 
	udevcontrol --reload_rules 2>/dev/null
elif [ -x "\`which udevadm 2>/dev/null\`" ]; then 
	udevadm control --reload_rules 2>/dev/null
	udevadm control --reload-rules 2>/dev/null
fi
exit 0
EOFX
EOF
}


# make an rpm macro file to overwrite defaults of rpm installed
# this behavior changed since Kubuntu 8.04  ... GRRRRRRR
# leaves a file called ~/.rpmmacros  which we remove afterwards
# may change again when epm decides to use someting else than buildroot
# we need to make those in the rpmmacros as the ones in the script from epm
# are overruled by the rpm default macros which are leading to a wrong path
# epm would have to use --buildroot <path> but does not (yet ??)
make_RpmDefaults()
{
	echo "creating rpm default dirs macro"
# setting topdir to the rpm build head and further are below
#  the buildroot dir name is taken from the epm -vvvv  verbose output
cat > ${rpmmacrofile} <<EOF
%_unpackaged_files_terminate_build 0
%_topdir      `pwd`/${dstdir}/rpm
%_buildrootdir  %{_topdir}/buildroot
%buildroot  %{_buildrootdir}
EOF
}

# epm would have to use --buildroot <path> but does not (yet ??)
kill_RpmDefaults()
{
	echo "removing rpm default dirs macro"
rm ${rpmmacrofile} 
}

### MAIN
{
	echo " "
	echo "$0:  EPM Package build script for ${PROGNAME} "
	echo " "

	echo "WXASTROCAPTURE_HOME = ${WXASTROCAPTURE_HOME-"not set"}"
	echo "Project Path : $PROJPATH"
	echo "Docu Path    : $DOCPATH"
	echo "Binary Path  : $BINPATH"

	# we use i386 not another ix86 for the architecture (other could be x64 amd64 or so)
	if [ $ARCHITECTURE = "i486" ] ; then  ARCHITECTURE=i386 ; fi
	if [ $ARCHITECTURE = "i586" ] ; then  ARCHITECTURE=i386 ; fi
	if [ $ARCHITECTURE = "i686" ] ; then  ARCHITECTURE=i386 ; fi
	echo "Architecture : $ARCHITECTURE"
	echo " "

	# sanity checks
	if [ "$USER" = "root" ]; then
		echo "$0 shall not be run with sudo rights !!"
		exit -1
	fi
	if [ ! -f "`which epm`" ]; then
	  echo "$0: epm program missing "
	  echo "Use: sudo apt-get install epm"
	  exit 1
	fi
	if [ ! -f "`which type-handling`" ]; then
	  echo "$0: type-handling program missing "
	  echo "Use: sudo apt-get install type-handling"
	  exit 1
	fi
	if [ ! -f "`which rpmbuild`" ]; then
	  echo "$0: rpmbuild program missing "
	  echo "Use: sudo apt-get install rpm"
	  exit 1
	fi

	if [ ! -d "./UX" ]; then
	  echo "$0: UX build directory missing (./UX)"
	  exit 1
	fi

	# now proceed to the build directory (currently ./UX)
	pushd ./UX >/dev/null

	if [ ! -d "${BROOT}" ]; then
	  echo "$0: package root directory missing (${BROOT})"
	  exit 1
	fi



# cleanup remanents
	rm -f *.deb
	rm -f *.md5

	pushd ${BROOT} >/dev/null  # dive into buildroot

## start with a clean environment
	rm ${epmfile}
	rm -rf ./*
	mkdir ${srcdir}
	mkdir ${dstdir}

# get all files into the ./debian folder - dh scripts will take it there and distribute them
	make_Relnotes;  if [ $? != 0 ]; then exit -1 ; fi
	make_Version;   if [ $? != 0 ]; then exit -1 ; fi

	make_Doc; if [ $? != 0 ]; then exit -1 ; fi

	make_Control; if [ $? != 0 ]; then exit -1 ; fi

	get_Files; if [ $? != 0 ]; then exit -1 ; fi
	get_RulesFile "55-atik.rules"; if [ $? != 0 ]; then exit -1 ; fi
	get_RulesFile "55-shoestring.rules"; if [ $? != 0 ]; then exit -1 ; fi
	make_Menu; if [ $? != 0 ]; then exit -1 ; fi
	make_Desktop; if [ $? != 0 ]; then exit -1 ; fi

	make_PostActions; if [ $? != 0 ]; then exit -1 ; fi

## using EPM now
	echo "  "
	echo "running EPM now  ******  "

	echo "  "
	echo "running EPM for Portable-------------------------------------------------"
	echo "epm -a ${ARCHITECTURE}  --output-dir  ${dstdir}/portable ${PACKAGE} ${PACKAGE}.list"
	epm -g -a ${ARCHITECTURE}  --output-dir  ${dstdir}/portable ${PACKAGE} ${PACKAGE}.list
 	if [ $? != 0 ]; then echo "epm -f portable -- FAILED" ; exit -1; fi
#    	epm --setup-image ${srcdir}/${BINNAME}48.xpm ${PACKAGE}

#	package_version_arch.portable
	pushd ${dstdir}/portable >/dev/null  # dive into buildroot

		PNAME=`ls *.tar.gz`
		echo " "
		echo "Portable Package Contents:....... "
		tar -tvf  ${PNAME}
		echo " "
		md5sum ${PNAME} > ${PNAME}.md5
	
		if [ -f "${DESTDIR}/${PNAME}" ]; then
			echo "!!! Package ${PNAME} already exists in ${DESTDIR}, cannot overwrite!"
			echo " the new package is left in ${dstdir}/portable you may find it there..."
		else
			mv ${PNAME}* ${DESTDIR}
			echo "  Package ${PNAME} is in ${DESTDIR} now"
		fi
		echo "build finished----------------------------------------------------"
	popd >/dev/null # from build deb

#	package_version_arch.deb
	echo "  "
	echo "running EPM for Debian-------------------------------------------------"
	echo "epm -a ${ARCHITECTURE} -f deb --output-dir  ${dstdir}/deb ${PACKAGE} ${PACKAGE}.list"
	epm -g -a ${ARCHITECTURE} -f deb --output-dir  ${dstdir}/deb ${PACKAGE} ${PACKAGE}.list
 	if [ $? != 0 ]; then echo "epm -f deb -- FAILED"; exit -1; fi

	pushd ${dstdir}/deb >/dev/null  # dive into buildroot

		PNAME=`ls *.deb`
		echo " "
		echo "Debian Package Info:....... "
		dpkg-deb --info ${PNAME}
		echo "Debian Package Contents:....... "
		dpkg-deb --contents ${PNAME}
		echo " "
	
		md5sum ${PNAME} > ${PNAME}.md5
	
		if [ -f "${DESTDIR}/${PNAME}" ]; then
			echo "!!! Package ${PNAME} already exists in ${DESTDIR}, cannot overwrite!"
			echo " the new package is left in ${dstdir}/deb you may find it there..."
		else
			mv ${PNAME}* ${DESTDIR}
			echo "  Package ${PNAME} is in ${DESTDIR} now"
		fi
		echo "build finished----------------------------------------------------"
	popd >/dev/null # from build deb

#	package_version_arch.rpm
	echo "  "
	echo "running EPM RPM-------------------------------------------------"
	make_RpmDefaults  # 20100709:BM  fix dirs for RPM

	echo "epm -a ${ARCHITECTURE} -f rpm --output-dir  ${dstdir}/rpm ${PACKAGE} ${PACKAGE}.list"
#	epm -vvvv -g -a ${ARCHITECTURE} -f rpm --output-dir  ${dstdir}/rpm ${PACKAGE} ${PACKAGE}.list
	epm -g -a ${ARCHITECTURE} -f rpm --output-dir  ${dstdir}/rpm ${PACKAGE} ${PACKAGE}.list
	if [ $? != 0 ]; then echo "epm -f rpm -- FAILED"; exit -1 ; fi

  	kill_RpmDefaults  # 20100709:BM  remove the file created to fix things

	pushd ${dstdir}/rpm >/dev/null  # dive into buildroot

		PNAME=`ls *.rpm`
		echo " "
		echo "RPM Package Info:....... "
		rpm -qi -p ${PNAME}
		echo "RPM Package Contents:....... "
		rpm -ql -p ${PNAME}
		echo " "
	
		md5sum ${PNAME} > ${PNAME}.md5
	
		if [ -f "${DESTDIR}/${PNAME}" ]; then
			echo "!!! Package ${PNAME} already exists in ${DESTDIR}, cannot overwrite!"
			echo " the new package is left in ${dstdir}/rpm you may find it there..."
		else
			mv ${PNAME}* ${DESTDIR}
			echo "  Package ${PNAME} is in ${DESTDIR} now"
		fi
		echo "build finished----------------------------------------------------"
	popd >/dev/null # from build rpm

	popd >/dev/null	# from build root - back in UX now

	popd >/dev/null # from UX
	echo " "
	echo "---END $0 "
	echo " "
	exit 0  # finally
}
#EOF
