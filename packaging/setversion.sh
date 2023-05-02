#!/bin/bash

sed -ri  "s/Version:\\s+.*/Version: $1/" lbackup/DEBIAN/control
sed -ri "s/LBACKUP_VERSION\\s+.*/LBACKUP_VERSION \"$1\"/" ../Lib/conf.h
