#!/bin/bash
#
# Debian Package Build Script
# must be run with fakeroot  !!!
# i.e.   $> fakeroot bash ./wxAstroCapture.sh
# 20070528/BM
# 20080919/BM update for i386 _and_ amd64 architecture
# 20090308/BM added rules files of libusb usage
#
#
# assuming the following folder and file structure
# ./Debian_control_master.txt  - the master control file
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
#   debhelp scripts installed: sudo apt-get install debhelper
#   dpkg installed
#   md5sum installed
#   fakeroot installed: sudo apt-get install fakeroot
#   type-handling installed: sudo apt-get install type-handling
#   alien installed: sudo apt-get install alien
#
# useful reference:
# http://tldp.org/HOWTO/html_single/Debian-Binary-Package-Building-HOWTO/
# http://www.grymoire.com/Unix/Sed.html
#
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

# relative to setup/UX  path

BROOT=./deb_root    	# Build root folder name
DESTDIR=../redist	# where to put the final package

#  ~/  means our home dir....

# File Source setup use env var WXASTROCAPTURE_HOME= to overwrite the given path
PROJPATH=${WXASTROCAPTURE_HOME-~/proj}/wxWebcamDB
# Docu folder from where to install
DOCPATH=${PROJPATH}/setup/doc
# Path to Binaries to install
BINPATH=${PROJPATH}/UX/bin/Release

# used to determine 32 or 64 bit architecture
MACHINE=`uname -m`
# fix i686 reply with a more generic i386 architecture
if [ "$MACHINE" = "i686" ] ; then 
	MACHINE="i386"
fi
ARCHITECTURE=`type-handling $MACHINE linux`

## copy the files needed (we are in build root now)
get_Files ()
{
# copy each file like the template
  if [ ! -f "${BINPATH}/wxAstroCapture" ]; then
    echo "E: get_Files(): could not find file - ${BINPATH}/wxAstroCapture"
    return -1  # did not find file ERROR EXIT
  fi
 cp "${BINPATH}/wxAstroCapture" ./debian/tmp/${BINNAME}  	# copy into temp location
 echo "get_Files(): dh_install - ${BINNAME}"
 dh_install  ./debian/tmp/${BINNAME}  usr/bin		# and install it from there
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
	cp "${PROJPATH}/${rFile}" ./debian/tmp/${rFile}  	# copy into temp location
	
 echo "get_Files(): dh_install - ${rFile}"
 dh_install  ./debian/tmp/${rFile}  etc/udev/rules.d		# and install it from there
# next copy section

# regular exit
  return 0
}

# http://standards.freedesktop.org/menu-spec/1.0/
make_Desktop()
{
# get program icon
# http://standards.freedesktop.org/icon-theme-spec/latest/ar01s07.html
  mkdir -p ./debian/tmp/
  cp ${DOCPATH}/${BINNAME}32x32.png ./debian/tmp/${BINNAME}.png
  dh_install debian/tmp/${BINNAME}.png usr/share/icons/hicolor/32x32/apps
  cp ${DOCPATH}/${BINNAME}48x48.png ./debian/tmp/${BINNAME}.png
  dh_install debian/tmp/${BINNAME}.png usr/share/icons/hicolor/48x48/apps
  cp ${DOCPATH}/${BINNAME}64x64.png ./debian/tmp/${BINNAME}.png
  dh_install debian/tmp/${BINNAME}.png usr/share/icons/hicolor/64x64/apps

# the app.desktop file that makes the menu entry
# http://standards.freedesktop.org/desktop-entry-spec/latest/ar01s05.html
  cat > ./debian/${PACKAGE}.desktop <<EOF
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
dh_install debian/${PACKAGE}.desktop usr/share/applications
}


# write up the X11 menu string used for that package (see Debian menufile(5) )
make_Menu()
{
  cat > ./debian/${PACKAGE}.menu <<EOF
?package(${PACKAGE}):needs=X11 section=Apps/Science title="${PROGNAME}" command="/usr/bin/${BINNAME}"
EOF
}

# make the control file inplace here
make_Control()
{
  echo " making the control file now...."
  cat > ./debian/control <<EOF
Source: ${PACKAGE}
Maintainer: ${MAINTAINER}

Package: ${PACKAGE}
Section: science
Priority: optional
Architecture: ${ARCHITECTURE}
Depends:
Description: wxWidgets based Webcam Capture Program
 wxAstroCapture is used for capturing deep sky 
 and planetary objects with telescopes

EOF
}

# derive relnotes.txt (changelog) from version.h
make_Relnotes()
{
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


# copy and name docu stuff as required for deb packages
make_Doc ()
{
  echo " copy docu files now...."
# copy files, removing CRs (\r or 0x0D or \015 (octal) )
  if [ -f "${DOCPATH}/relnotes.txt" ]; then
    cat "${DOCPATH}/relnotes.txt" | tr -d "\r" > ./debian/${PACKAGE}.changelog
  else
   echo "missing relnotes.txt file"
  fi

  if [ -f "${DOCPATH}/readme_ux.txt" ]; then
    cat "${DOCPATH}/readme_ux.txt" | tr -d "\r" > ./debian/README.Debian
  else
   echo "missing readme_ux.txt file"
  fi

  if [ -f "${DOCPATH}/license.txt" ]; then
    cat "${DOCPATH}/license.txt" | tr -d "\r" > ./debian/copyright
  else
   echo "missing license.txt file"
  fi

  echo " "
}


### MAIN
{
	echo " "
	echo "$0:  Debian Package build script for ${PROGNAME} "
	echo " "

	echo "WXASTROCAPTURE_HOME = ${WXASTROCAPTURE_HOME-"not set"}"
	echo "Project Path : $PROJPATH"
	echo "Docu Path    : $DOCPATH"
	echo "Binary Path  : $BINPATH"
	echo " "

	# sanity checks
	if [ "$USER" = "root" ]; then
		echo "$0 shall not be run with sudo rights !!"
		exit -1
	fi
	if [ ! -f "`which dpkg-deb`" ]; then
	  echo "$0: dpkg program missing "
	  exit 1
	fi
	if [ ! -f "`which dh_testroot`" ]; then
	  echo "$0: dh_testroot program missing "
	  echo "use: sudo apt-get install debhelper"
	  exit 1
	fi
	if [ ! -f "`which alien`" ]; then
	  echo "$0: alien program missing "
	  echo "use: sudo apt-get install alien"
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

## here we start processing

# cleanup remanents
	rm -f *.deb
	rm -f *.md5

	pushd ${BROOT} >/dev/null  # dive into buildroot

## start with a clean environment
	dh_clean
	rm -rf ./debian
	rm -f ./*
	mkdir ./debian
	echo 5 > debian/compat	# set script gen compatibility level (see man debhelper)

# get all files into the ./debian folder - dh scripts will take it there and distribute them
	make_Control; if [ $? != 0 ]; then exit -1 ; fi
	make_Relnotes;  if [ $? != 0 ]; then exit -1 ; fi
	make_Doc; if [ $? != 0 ]; then exit -1 ; fi
	make_Menu; if [ $? != 0 ]; then exit -1 ; fi
	make_Desktop; if [ $? != 0 ]; then exit -1 ; fi

## using debhelper now
	echo "  running dh scripts now"
	echo "  dh_testdir"
	dh_testdir ; if [ $? != 0 ]; then exit -1 ; fi
	echo "  dh_testroot"
	dh_testroot ; if [ $? != 0 ]; then exit -1 ; fi
	echo "  dh_installchangelogs"
	dh_installchangelogs ; if [ $? != 0 ]; then exit -1 ; fi
	echo "  dh_installdocs"
	dh_installdocs ; if [ $? != 0 ]; then exit -1 ; fi

	get_Files; if [ $? != 0 ]; then exit -1 ; fi
	get_RulesFile "55-atik.rules"; if [ $? != 0 ]; then exit -1 ; fi
	get_RulesFile "55-shoestring.rules"; if [ $? != 0 ]; then exit -1 ; fi

	echo "  dh_installmenu"
	dh_installmenu ; if [ $? != 0 ]; then exit -1 ; fi
	echo "  dh_desktop"
	dh_desktop ; if [ $? != 0 ]; then exit -1 ; fi
## dh_strip destroys our compressed executable, don't use it!!
#	echo "  dh_strip"
#	dh_strip ; if [ $? != 0 ]; then exit -1 ; fi
	echo "  dh_compress"
	dh_compress
	echo "  dh_fixperms"
	dh_fixperms ; if [ $? != 0 ]; then exit -1 ; fi
	echo "  dh_installdeb"
	dh_installdeb ; if [ $? != 0 ]; then exit -1 ; fi
#	echo "  dh_shlibdeps"
#	dh_shlibdeps ; if [ $? != 0 ]; then exit -1 ; fi
	echo "  dh_gencontrol"
	dh_gencontrol; if [ $? != 0 ]; then exit -1 ; fi
	echo "  dh_md5sums"
	dh_md5sums ; if [ $? != 0 ]; then exit -1 ; fi
	echo "  dh_builddeb"
	dh_builddeb  ; if [ $? != 0 ]; then exit -1 ; fi

	popd >/dev/null	# from build root - back in UX now

#	here we should have the package built
#	package_version_arch.deb
	PNAME=`ls ${PACKAGE}_*.deb`
	echo " "
	echo "build finished----------------------------------------------------"
	echo "Debian Package Info:....... "
	dpkg-deb --info ${PNAME}
	echo "Debian Package Contents:....... "
	dpkg-deb --contents ${PNAME}
	echo " "

	md5sum ${PNAME} > ${PNAME}.md5

#	Make RPM package
	echo "Making RPM package using alien, please enter sudo password for alien"
	sudo alien -r -c  ${PNAME}



	if [ -f "${DESTDIR}/${PNAME}" ]; then
	  echo "!!! Package ${PNAME} already exists in ${DESTDIR}, cannot overwrite!"
	  echo " the new package is left in ./UX you may find it there..."
	  exit 1
	else
	  mv ${PNAME}* ${DESTDIR}
	  mv *.rpm ${DESTDIR}
	  echo "  Package ${PNAME} and *.rpm is in ${DESTDIR} now"
	fi
	popd >/dev/null # from UX
	echo " "
	echo "---END $0 "
	echo " "
	exit 0  # finally
}
#EOF
