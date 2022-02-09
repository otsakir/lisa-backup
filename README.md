# lbackup

A QT backup application with on-mount triggering. Aims at ease of use, minimal user effort and pluggable architecture.

Currently on alpha stage.


### How to use

In Lisa Backup, a separate backup operation is called a _task_. A  task copies files from one or more _sources_ to a single _destination_. Each source has a _method_ that tells whether everything should be copied (_All_) or a some sort criteria, like the existence of a file, should be applied (_Selective_) to the source tree. The underlying utility that will perform the backup is the _Action_. By default `rsync` is used but `git` is also a good options for repositories. The _Destination_ consists of two parts. The mount path of a _Device_ and a relative _Path_ that is appended to it. 

Tasks can be executed manually (_Run_) or triggered automatically when a storage medium is mounted. Such triggers need to be _Installed_ and rely on `systemd`. In both cases, a popup confirmation window will be shown before running the task. 

Lastly Don't forget to _Save_ a task before executing it. It will generate a .task file and respective backup bash script under ~/.lbackup. 


### Installation

Install QT dependencies and xterm from debian/ubuntu repository:

    sudo apt install libqt5core5a libqt5gui5 libqt5gui5-gles libqt5widgets5 xterm rsync
    
Download [latest lbackup release](releases/latest) in .deb format and install with:

    sudo dpkg --install sudo dpkg --install lbackup_{version}_amd64.deb

Note, `rsync`, `find`, `git` and `bash` should be available

### Build

To build without QT Creator, install dependencies:

    sudo apt install g++ make qtbase5-dev 

Then clone lbackup repo and:

    cd lbackup
    mkdir build
    cd build
    qmake ../LisaBackup.pro
    make

The application binary will be placed under App/ directory.
    

    



	
    
