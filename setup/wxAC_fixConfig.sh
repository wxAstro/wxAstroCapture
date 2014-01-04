#!/bin/bash
#
# 20080224BM
# Fixup for the new wxAstroCapture config file path under Linux
#
# checks for ~/.wxAstroCapture   config file 
# creates a new directory .wxAstroCapture if it does not exist
# moves the config file into the directory and renames it properly

# main
{
	if [ ! -d ~/.wxAstroCapture ]; then
		if [ -f ~/.wxAstroCapture ]; then
			mv ~/.wxAstroCapture /tmp/wxACFIX.CCC
			mkdir ~/.wxAstroCapture
			mv /tmp/wxACFIX.CCC ~/.wxAstroCapture/wxAstroCapture.conf
			echo "config file updated"
		fi
	else
		echo "config file update not required"
	fi
	echo "$0 has finished"
}
 
