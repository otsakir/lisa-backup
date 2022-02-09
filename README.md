# lbackup

A QT backup application with on-mount triggering. Aims at ease of use, minimal user effort and pluggable architecture.

Currently on alpha stage.


### How it works

In Lisa Backup, a separate backup operation is called a _task_. A  task copies files from one or more _sources_ to a single _destination_. Each source has a _method_ that tells whether everything _(All)_ should be copied or a some sort criteria should be applied to the source tree _(Selective)_. The underlying utility that will perform the backup is the _Action_. By default `rsync` is used but `git` is also a good options for repositories. The _Destination_ consists of two parts. The mount path of a _Device_ and a relative _Path_ that is appended to it. 

Tasks can be executed manually _(Run)_ or triggered automatically when a storage medium is mounted _(On-mount triggering)_. Such triggers need to be _Installed_ and rely on `systemd`. In both cases, a popup confirmation window will be shown before running the task. 

Lastly, tasks should be _Saved_ before getting executed. Saving a task will generate a .task file and respective bash script under ~/.lbackup. 


### Installation

Install QT dependencies and xterm from debian/ubuntu repository:

    sudo apt install libqt5core5a libqt5gui5 libqt5gui5-gles libqt5widgets5 xterm rsync
    
Download [latest lbackup release](releases/latest) in .deb format and install with:

    sudo dpkg --install sudo dpkg --install lbackup_{version}_amd64.deb

Note, `rsync`, `find`, `git` and `bash` should be available

### Build

To build without QT Creator install dependencies:

    sudo apt install g++ make qtbase5-dev 

Clone lbackup repository and build it:

    cd lbackup
    mkdir build
    cd build
    qmake ../LisaBackup.pro
    make

The application binary will be placed under App/ directory.
    

    



	
    
