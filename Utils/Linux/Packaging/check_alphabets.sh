#!/bin/sh
#
# Check whether alphabet files are up to date
#

SOURCE_DIR=/home/ftp/pub/www/dasher/download/alphabets
TARGET_DIR=/home/pjc51/files/work/projects/hci/dasher/dasher/Data/system.rc
EXCLUDE_FILE=/home/ftp/pub/www/dasher/download/alphabets/EXCLUDELIST

# First check for any files which have been removed:

for fn in $TARGET_DIR/a*xml $TARGET_DIR/c*xml $TARGET_DIR/*xsl; do
    filename=`basename $fn`
    if grep -q $filename $EXCLUDE_FILE || (echo $filename | grep -q ~\$) || !([[ -e $SOURCE_DIR/$filename ]]); then
	echo "$filename excluded"
    fi
done

# Now check for additions and changes

for fn in $SOURCE_DIR/a*xml $SOURCE_DIR/c*xml $SOURCE_DIR/*xsl; do
    filename=`basename $fn`
    if !(grep -q $filename $EXCLUDE_FILE) && !(echo $filename | grep -q ~\$) && !(echo $filename | grep -q bak\$); then
	if !([[ -e $TARGET_DIR/$filename ]]); then
	    echo "$filename added"
	else
	    if !(cmp -s $SOURCE_DIR/$filename $TARGET_DIR/$filename); then
		echo "$filename differs"
	    fi 
	fi
    fi
done
