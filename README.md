# lbackup

A QT backup application with on-mount triggering. Aims at ease of use, minimal user effort and pluggable architecture.

Currently on alpha stage.

### Overview

The application is based on the idea of backup tasks. A task consists of a number of sources (directories with selection criteria) and a destination (where to copy the date and when). To avoid selecting each and every directory to backup, a criteria based approach has been adopted instead. The criteria form a predicate. An algorithm to tell what parts of the source directory tree will be included. For example, you can search select all Git repos in a path by looking for the `.git` sub-directory.

Backup tasks can be manually initiated by pressing _Run_ or use a `trigger`. A systemd based mechanism has been imlemented to detect when a specific medium is mounted and start a task when that happens. This is On-Mount triggering. 

### Installation

Install QT dependencies and xterm from debian/ubuntu repository:

    sudo apt install libqt5core5a libqt5gui5 libqt5gui5-gles libqt5widgets5 xterm
    
Download [latest lbackup release](releases/latest) in .deb format and install with:

    sudo dpkg --install sudo dpkg --install lbackup_{version}_amd64.deb
    
