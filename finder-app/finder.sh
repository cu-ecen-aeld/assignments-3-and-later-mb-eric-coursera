#!/bin/sh
exp_num_arg=2

if ! [ $# -eq ${exp_num_arg} ]; then
    echo "ERROR: Invalid number of Arguments."
    echo "Total number of arguments should be ${exp_num_arg}"
    echo "The order of the arguments should be:"
    echo "   1) File directory path"
    echo "   2) String to be searched in the specified directory path"
    exit 1
fi

filesdir=$1
searchstr=$2

if ! [ -d ${filesdir} ]; then
    echo "ERROR: ${filesdir} does not exist!"
    exit 1
fi

X=`ls ${filesdir} | wc -l`
Y=`grep ${searchstr} ${filesdir}/* 2>/dev/null | wc -l`
echo "The number of files are ${X} and the number of matching lines are ${Y}"
