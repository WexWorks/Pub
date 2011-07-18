#!/bin/bash

if [ $# != 2 ] ; then
  printf "%s\n" "$0 AppName domainname"
  exit
fi


APP=$1
DOMAIN=$2
LCAPP=${APP,,}
OLDAPP=HelloApp
OLDDOMAIN=HelloAppDomain
LCOLDAPP=${OLDAPP,,}

function replace_file_content {
  sed s/$OLDAPP/$APP/g $1 | sed s/$LCOLDAPP/$LCAPP/g | sed s/$OLDDOMAIN/$DOMAIN/g > $1.tmp
  rm $1
  mv $1.tmp $1
}

function rename_files {
  declare filelist=`ls -A`
  for file in $filelist; do
    declare newfile=${file/$OLDAPP/$APP}
    newfile=${newfile/$LCOLDAPP/$LCAPP}
    newfile=${newfile/$OLDDOMAIN/$DOMAIN}
    if [[ $file != $newfile ]] ; then
      mv $file $newfile
    fi

    if test -d $newfile ; then      
      pushd $newfile
      rename_files
      popd
    else
      replace_file_content $newfile
    fi
  done
}


tar xvzf $OLDAPP.tgz
if test -d $OLDAPP ; then
  mv $OLDAPP $APP
  cd $APP
  rename_files
else
  printf "%s\n" "Cannot find template directory $OLDAPP"
fi

