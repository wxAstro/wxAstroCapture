#!/bin/sh -e


###
# Script to get the essential for building wxWebcamDB stuff
# Run it with sudo 
# Tested on Kubuntu 10.04
#
# 20100709:BM
###

# code::blocks IDE
# Install Trust  - needs a yes from the console (I don't force this)
apt-get install jens-lody-debian-keyring 

# wxWidgets 
# key-import to apt's trusted keys: 
wget -q http://apt.wxwidgets.org/key.asc -O-  | apt-key add -

# Create a new sources file to get the codeblocks and wxwidgets from
cat >> /etc/apt/sources.list.d/mySources.list <<EOF
deb http://apt.jenslody.de/ any main
deb-src http://apt.jenslody.de/ any main
deb http://apt.wxwidgets.org/ lenny-wx main
EOF

# Update the dpkg database now
apt-get update

# some development tools
#  apt-get -y install komparator
#  use the one for KDE4 (Google for it)
apt-get -y install kdiff3
apt-get -y install kompare
apt-get -y install kdbg
# Code::Blocks IDE
apt-get -y install codeblocks

# general C++ build
apt-get -y install g++
apt-get -y install cmake
apt-get -y install upx-nrv
# Linux development headers
apt-get -y install linux-libc-dev
# usb development headers
apt-get -y install libusb-dev
# for wxWidgets build
apt-get -y install libgtk-directfb*
# fits IO lib and development headers + doc
apt-get -y install libcfitsio3*

#  Build EPM script expects:
#   epm installed
apt-get -y install epm
#   fltk libraries installed
#apt-get -y install libfltk1.1
#   type-handling installed
apt-get -y install type-handling
#   rpmbuild installed
apt-get -y install rpm


# planetarium software
apt-get -y install kstars



