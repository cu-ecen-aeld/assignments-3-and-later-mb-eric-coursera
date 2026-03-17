#!/bin/sh
# Tester script for assignment 1 and assignment 2
# Author: Siddhant Jajoo

set -e
set -u

#
#        b. Modify your finder-test.sh to run with necessary files found in the PATH.
#
#                i. In other words, you should be able to run /path/to/script/finder-test.sh 
#                   and the script should run successfully, assuming all executables are in 
#                   the PATH and config files are at /etc/finder-app/conf. This change will 
#                   be added to your assignment 3 and later source repository.  The point of 
#                   this step is to ensure you can run the finder-test.sh script using 
#                   /usr/bin/finder-test.sh and all scripts and executables needed by 
#                   finder-test.sh will be located on the target qemu rootfs.
#



NUMFILES=10
WRITESTR=AELD_IS_FUN
WRITEDIR=/tmp/assignment4-result.txt
CONFDIR_ROOT=/etc/finder-app
#CONFDIR_ROOT=$(dirname $(realpath $0))
username=$(cat ${CONFDIR_ROOT}/conf/username.txt)


if [ $# -lt 3 ]
then
	echo "Using default value ${WRITESTR} for string to write"
	if [ $# -lt 1 ]
	then
		echo "Using default value ${NUMFILES} for number of files to write"
	else
		NUMFILES=$1
	fi	
else
	NUMFILES=$1
	WRITESTR=$2
	WRITEDIR=/tmp/aeld-data/$3
fi

MATCHSTR="The number of files are ${NUMFILES} and the number of matching lines are ${NUMFILES}"

echo "Writing ${NUMFILES} files containing string ${WRITESTR} to ${WRITEDIR}"

rm -rf "${WRITEDIR}"

# create $WRITEDIR if not assignment1
assignment=`cat ${CONFDIR_ROOT}/conf/assignment.txt`

if [ $assignment != 'assignment1' ]
then
	mkdir -p "$WRITEDIR"

	#The WRITEDIR is in quotes because if the directory path consists of spaces, then variable substitution will consider it as multiple argument.
	#The quotes signify that the entire string in WRITEDIR is a single string.
	#This issue can also be resolved by using double square brackets i.e [[ ]] instead of using quotes.
	if [ -d "$WRITEDIR" ]
	then
		echo "$WRITEDIR created"
	else
		exit 1
	fi
fi
echo "Removing the old writer utility and compiling as a native application"
#make clean
#make

for i in $( seq 1 $NUMFILES)
do
	writer "$WRITEDIR/${username}$i.txt" "$WRITESTR"
done

OUTPUTSTRING=$(finder.sh "$WRITEDIR" "$WRITESTR")

# remove temporary directories
rm -rf /tmp/aeld-data

set +e
echo ${OUTPUTSTRING} | grep "${MATCHSTR}"
if [ $? -eq 0 ]; then
	echo "success"
	exit 0
else
	echo "failed: expected  ${MATCHSTR} in ${OUTPUTSTRING} but instead found"
	exit 1
fi
