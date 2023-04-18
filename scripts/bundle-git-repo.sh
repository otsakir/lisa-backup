#!/bin/bash

# create a bundle file for a git repository. 
# example: ./bundle-git-repo.sh /home/nando/Projects/garden /tmp
# 	   bundles repo to  /tmp/garden.gitbundle file

if [ $# -lt 2 ]; then
	echo "usage:	bundle-git-repo.sh {repopath} {destpath}"
	exit 1
fi
reporoot="$1"

reponame=`echo $reporoot | sed -e 's/.*\///'` 
if [ -z "$reponame" ]; then
	echo "no valid repo name parsed: $reponame"
	exit 2
fi


cwd=`pwd`
echo "creating bundle for '$reporoot' repo under $2. Repo name: '$reponame'."
mkdir -p "$2"

error_out=`cd "$reporoot" 2>&1 > /dev/null && git bundle create "$2/$reponame".gitbundle --all 2>&1 > /dev/null && cd "$cwd" 2>&1 > /dev/null`
if [ $? -ne 0 ]; then
    echo  "[source '$reporoot'] " $error_out 1>&2
fi
