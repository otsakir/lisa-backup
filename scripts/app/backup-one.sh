#!/bin/bash

# create a bundle file for a git repository. 
# example: ./backup-one.sh /home/nando/Projects/garden /tmp
# 	   bundles repo to  /tmp/garden.gitbundle file


ACTION="rsync"

while getopts 'a:h' opt; do
  case "$opt" in
    a)    
	ACTION="$OPTARG"
      ;;
    h)
      echo "Usage: $(basename $0) [-a action] destpath repopath"
      exit 0
      ;;

    :)
      echo -e "Option requires an argument.\nUsage: $(basename $0) [-a action] destpath repopath"
      exit 1
      ;;

    ?)
      echo -e "Invalid command option.\nUsage: $(basename $0) [-a action] destpath repopath"
      exit 1
      ;;
  esac
done
shift "$(($OPTIND -1))"


if [ $# -lt 2 ]; then
	echo "Usage: $(basename $0) [-a action] destpath repopath"
	exit 1
fi
destpath="$1"
shift 1
reporoot="$1"


reponame=`echo $1 | sed -e 's/.*\///'` 
if [ -z "$reponame" ]; then
	echo "No valid repo name parsed: $reponame"
	exit 2
fi

while true 
do
	case "$ACTION" in
		auto)
			if [ -d "$reporoot/.git" ]; then 
				ACTION=gitbundle
				continue
			else
				ACTION=rsync
				continue
			fi
			;;
		rsync)
			echo "rsync -avzh  \"$reporoot\" \"$destpath\""
			rsync -avzh "$reporoot" "$destpath"

			break
			;;
		gitbundle)

			bundlefile="$destpath/$reponame".gitbundle
			cwd=`pwd`
			echo "creating bundle for '$reporoot' repo into $bundlefile . Repo name: '$reponame'."
			mkdir -p "$destpath"

			echo "cd \"$reporoot\" && git bundle create \"$bundlefile\""
			error_out=`cd "$reporoot" 2>&1 > /dev/null && git bundle create "$bundlefile" --all 2>&1 > /dev/null && cd "$cwd" 2>&1 > /dev/null`
			if [ $? -ne 0 ]; then
			    echo  "[source '$reporoot'] " $error_out 1>&2
			fi

			break
			;;
		*)
			echo "Invalid action: '$ACTION'. It should be one of rsync|gitbundle|auto." 1>&2
			exit 1
			;;
	esac 
done


