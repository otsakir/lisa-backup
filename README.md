# Lisa Backup

A QT backup application with on-mount triggering. Aims at ease of use, minimal user effort and pluggable architecture.

Currently on late alpha stage.


### How it works

Backup operations are called _tasks_. A  task copies files from one or more _sources_ to a single _destination_. Each source has a _method_ that tells whether it should copy everything _(All)_ or apply filtering criteria to the source tree _(Selective)_. The underlying utility that will perform the backup is the _Action_. `rsync` is the preselected action but `git` is also a good option for repositories. `auto` action will try guess which is a better fit per directory matched and use that. _Destination_ is the path where the files will end up into.

Tasks can be executed manually _(Run)_ or triggered automatically when a storage medium is mounted _(Triggering)_.

Lastly, tasks data is persisted to a .task file which reside under ~/.lbackup. 


### Installation

Make sure Qt5 `gui`, `core` and `widgets` dependencies are available. Qt5 is usually installed by default in recent linux desktop distros. You'll also need `udisks2` package to enable on-mount trigerring. Note, `rsync`, `find` and  `git` should be available.

Next, download [latest lbackup release](https://github.com/otsakir/lbackup/releases/latest) in .tar.gz format, extract and run the installation script:

    wget https://github.com/otsakir/lbackup/releases/download/v0.3.0/lbackup_0.3.0_amd64.tar.gz
    tar zxvf lbackup_0.3.0_amd64.tar.gz 
    cd lbackup/
    sudo ./install.sh 

Start the application from the current directory:

    $ ./lbackup 


### Preview

If this sounds interesting, check this overview video on youtube:

https://www.youtube.com/watch?v=5hYTa77cXgs
    

    



	
    
