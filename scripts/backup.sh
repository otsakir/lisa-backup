#!/bin/bash

if [[ $# -lt 2 ]]; then
	echo "not enough arguments"
	exit 1
fi

find "$1" -name "$2" -print '{}';
