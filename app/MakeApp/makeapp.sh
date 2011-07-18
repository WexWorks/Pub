#!/bin/bash

# makeapp.sh <MyApp> <mydomain>
#
# Create a new application using the template stored in HelloApp.tgz
#
# After extracting the contents of the tar file, the script recursively
# decends the application tree, changing directory and file names to
# match the application and domain arguments, and replacing the app and
# domain names in the contents of text files (e.g. code and project files).

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


# Called for each non-directory (e.g. text) file, replacing any instances
# of the OLD domain and app names with the NEW names from command line args
function replace_file_content {
  sed s/$OLDAPP/$APP/g $1 | sed s/$LCOLDAPP/$LCAPP/g | sed s/$OLDDOMAIN/$DOMAIN/g > $1.tmp
  rm $1
  mv $1.tmp $1
}

# Recursive function that decends the current tree, calling
# replace_file_content on any non-directories. 
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


# Extract the app from the tar file, rename directory and
# start the recursive descent to update directory and files.
if test -d MakeApp ; then
  tar xvzf MakeApp/$OLDAPP.tgz
  if test -d $OLDAPP ; then
    mv $OLDAPP $APP
    pushd $APP
    rename_files
    popd
    printf "%s\n" "Successfully created $DOMAIN.$APP"
  else
    printf "%s\n" "Cannot find template directory $OLDAPP"
  fi
else
  printf "%s\n" "Must be invoked from <ToT>/app"
fi

