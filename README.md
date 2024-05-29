# Lisa Backup

A backup application for the Linux Desktop written in C++/Qt. Aims at ease of use, minimal user effort and, hopefully, pluggable architecture.

Currently on beta stage.


### Usage

Organize your backup in units called _tasks_. Select a set of _source_ directories for each task and pick a _destination_. Click the "play" button to copy the files. This is the basic use case that rsyncs everything from source to destination. 

For more advanced usage:

  * Switch to "Selective" `Method` for a smarter way to select which subdirectories to copy from the source. For example, pick only those that follow a specific naming convention or that contain a specific file.
  * Enable "Triggering". It monitors when new volumes are mounted and performs a backup task when that happens. Combine this with "Keep running in system tray" option from "Settings" for better UX.
  * Finally, switch between "rsynch" and "git-bundle" Action to control what type of backup operation should be applied to this source directory. For git repositories this may prove more appropriate (though slower).


### Internals

Lisa Backup relies on powerfull commandline tools to do the actual work. Namely, `find`, `rsync`, and `git` are used. Make sure you have these available in your system.

Application state constists of \*.task files where most of task-specific information is stored and Qt configuration files that keep the _Settings_ and _Triggering_ preferences. You can find .task files under `~/.lbackup` directory. Note, they are binary. You've been warned :-).


### Installation

Download [latest lbackup release](https://github.com/otsakir/lisa-backup/releases/latest) in .tar.gz format, extract and execute:

    $ wget https://github.com/otsakir/lisa-backup/releases/download/v0.4.0/lisa-backup_0.4.0_amd64.tar.gz
    $ tar zxvf lisa-backup_0.4.0_amd64.tar.gz
    $ cd lisa-backup/
    $ ./LisaBackup

This is a dynamically linked binary and depends on Qt 5. Make sure it is installed on your system. For ubuntu based linux flavors you'll need `libqt5core5q`, `libqt5gui5`, `libqt5widgets5`, `libqt5dbus5` packages. 


##### Build from source 

[TBD]


### Bugs

Lisa Backup is in beta version and minor bugs are expected be found. Bug reports are greatly appreciated. The best way to communicate such feedback is by creating a github issue with the "bug" label. Thanks in advance!


### Contact

Mastodon - @otsakir@fosstodon.org

















    

    



	
    
