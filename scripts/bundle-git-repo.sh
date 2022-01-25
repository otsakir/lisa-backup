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

cd "$reporoot" && git bundle create "$2/$reponame".gitbundle --all && cd "$cwd"
