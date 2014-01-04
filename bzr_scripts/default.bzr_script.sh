#!/bin/bash

# REM PROJECT SETTING !!
#  define the remote location and ftp login (don't put the password here please!!!)
LOCATION="ftp://wxWebcamDB%40burri-web.org@burri-web.org/"

#  USER PREFERENCES
#  define the Viewer to use for Diff Output
VIEWER=/usr/bin/kwrite
# define the Diff Tool
D3DIFF=/usr/binkdiff3
#  define the temp file for e.g. the Diff output
TEMPFILE=/tmp/BZtmp.txt
#  This overwrites the default editor if needed
BZR_EDITOR=${VIEWER}

# the Reference Directory extension  i.e. Project is    foo   RefDir is then  ..\foo_Ref
REFDIR=_Ref


#  NO MORE CHANGES AFTER THIS POINT PLEASE

bINFO()
{
#  Information about the local branch
bzr info > ${TEMPFILE}
${VIEWER} ${TEMPFILE} 2>>/dev/null &
}

bSTAT()
{
#  Status of the local branch
bzr stat > ${TEMPFILE}
${VIEWER} ${TEMPFILE} 2>>/dev/null &
}

bLOG()
{
#  Logs the most recent things from the local branch
bzr log  > ${TEMPFILE}
${VIEWER} ${TEMPFILE} 2>>/dev/null &
}

bADD_F()
# arg1 is filename
{
#  Adds a file to the branch
#  a filename is required
bzr add $1
}

bDIFF_F()
# arg1 is filename
{
#  Makes a diff of the most recent changes
#  a filename is required
bzr diff $1 >${TEMPFILE}
${VIEWER} ${TEMPFILE} 2>>/dev/null &
}

bCOMMIT_F()
# arg1 is filename
{
#  Commits a file into the local branch
#  a filename is required
bzr commit  -m "via UX-CB script" $1
}

bCOMMIT_ALL()
{
#  Commits all into the local branch
bzr commit -m "via UX-CB script"
}

bMERGE()
# arg1 is project
{
# Merges the _Ref content into the project content
bzr merge ../$1${REFDIR}  >${TEMPFILE}
${VIEWER} ${TEMPFILE} 2>>/dev/null &
# trying to be clever and call the diff tool when a file.BASE exists only
	if [ -f $1.BASE ]; then
		${D3DIFF} $1.BASE $1.THIS $1.OTHER 2>>/dev/null &
	fi
}

bRESOLVE()
{
# Resolves all conflicts
bzr resolve
}

bPUSH()
# arg1 is project
{
#  This pushes the content to the _Ref directory
bzr push ../$1${REFDIR}
}


# SERVER TARGETED COMMANDS

bRLOG()
{
#  Logs the most recent things from the remote server
bzr log ${LOCATION}
}

bPUSH_REF()
# arg1 is project
{
# This pushes the content to the Server
pushd  ../$1${REFDIR}
bzr push ${LOCATION}
popd
}

bPULL_REF()
# arg1 is project
{
#  Pull the Server into Project_Ref
pushd  ../$1${REFDIR}
bzr pull --overwrite ${LOCATION}
popd
}

errout()
{
echo "Parameter wrong or missing"
}

# MAIN
# Arguments:
# $1  the project name (case sensitive for Linux)
# $2 the command
# $3 an optional filename
{
	if [ -d ../$1${REFDIR} ]; then

		case $2 in
			INFO)
				bINFO;;
			STAT)
				bSTAT;;
			LOG)
		 		bLOG;;
			ADD_F)
				bADD_F ${3?"missing parameter"};;
			DIFF_F)
				bDIFF_F ${3?"missing parameter"};;
			COMMIT_F)
				bCOMMIT_F ${3?"missing parameter"};;
			COMMIT_ALL)
				bCOMMIT_ALL;;
			MERGE)
				bMERGE ${1?"missing parameter"};;
			RESOLVE)
				bRESOLVE;;
			PUSH)
				bPUSH ${1?"missing parameter"};;

				# Server targeted commands
			RLOG)
		 		bRLOG;;
			PUSH_REF)
				bPUSH_REF ${1?"missing parameter"};;
			PULL_REF)
				bPULL_REF ${1?"missing parameter"};;
			*)
				errout
				exit 1;;
		esac
	
	else

		echo "$1_Ref directory does not exist!!"
		exit 1
	fi
	exit 0
}

