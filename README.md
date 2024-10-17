# Lisa Backup

A backup application for the Linux Desktop written in C++/Qt. Aims at ease of use, minimal user effort and, hopefully, pluggable architecture.

Currently on beta stage.


### Usage

* First, create a new `task`.  Tasks are backup operations. Pick a meaningfull name. Something related to how often you intend to take this backup or to the contents of it. For example '_weekly_' or '_monthly-photos_' work well. 
* Add `source` directories to the task. By default, any source directory you select will be included.
* Select a `Destination` directory. That would be a path in your local filesystem or on a mounted device to copy (rsync) the files to.
* Save  your task and run it by pressing the 'play' button at the bottom. Any source directories should be copied to the destination

For more advanced usage:

  * Switch to "Selective" `Method` for a smarter way to select which subdirectories to copy from the source. For example, pick only those that follow a specific naming convention or that contain a specific file.
  * Enable "Triggering". It monitors when new volumes are mounted and performs a backup task when that happens. Combine this with "Keep running in system tray" option from "Settings" for better UX.
  * Finally, switch between "rsynch" and "git-bundle" Action to control what type of backup operation should be applied to this source directory. For git repositories this may prove more appropriate (though slower).


### Installation

Download [latest lbackup release](https://github.com/otsakir/lisa-backup/releases/latest) in .tar.gz format, extract and execute the launcher script.

    $ tar zxvf lbackup_0.5.0_standalone_amd64.tar.gz 
    $ cd lbackup/
    $ ./run-LisaBackup.sh

Check the 'Dependencies' section below if this doesn't work.

#### Dependencies

Lisa Backup relies on powerfull command-line tools to do the actual work. Make sure `find`, `rsync`, and `git` (optional) are accessible from your system PATH.

Also, since this is a dynamically linked binary and depends on Qt 5, make sure it is installed on your system. For ubuntu based linux flavors you'll need `libqt5core5q`, `libqt5gui5`, `libqt5widgets5`, `libqt5dbus5` packages. 


### Data files and settings

Tasks, are stored in .task files under `~/.lbackup` by default. Application settings (reachable under 'Settings' menu) and triggerring information is Qt specific but is typically found `~/.config/{username}/Lisa Backup.conf`




### Bugs & known issues

Lisa Backup is in beta version and minor bugs are expected be found. Bug reports are greatly appreciated. The best way to communicate such feedback is by creating a github issue with the "bug" label. Thanks in advance!

There are also these [known issues](https://github.com/otsakir/lisa-backup/issues?q=is%3Aopen+is%3Aissue+label%3Abug).


### Contact

Mastodon - @otsakir@fosstodon.org

















    

    



	
    
