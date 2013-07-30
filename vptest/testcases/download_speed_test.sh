#!/bin/bash

# param 1 size of the file to download
# param 2 the file to write the output too

# Size (in MB) of file to download if not given as parameter.
DEFAULT_FILESIZE=10
 
#-------------------------------------------------------------------------------
 
if [ $# -eq 1 ];
then FILESIZE=$1
else FILESIZE=$DEFAULT_FILESIZE
fi

FILEURL="http://speedtest.cambrium.nl/${FILESIZE}mb.bin"

echo -e "\n[\e[0;34m+\e[0m] Testing download speed (${FILESIZE}MB file)...\n"
#wget  --output-document=/dev/null  $FILEURL

if [ $# -eq 2 ];
then wget  $FILEURL --output-document=/dev/null 2>>$2
else wget  $FILEURL --output-document=/dev/null
fi


#pass or fail	
OUT=$?
if [[ OUT -eq 0 ]]
then
	exit 0
else	
	exit 1
fi

#exit 0



#===============================================================================
#EOF
