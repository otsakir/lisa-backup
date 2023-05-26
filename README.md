# Lisa Backup

A QT backup application with on-mount triggering. Aims at ease of use, minimal user effort and pluggable architecture.

Currently on alpha stage.


### How it works

Backup operations are called _tasks_. A  task copies files from one or more _sources_ to a single _destination_. Each source has a _method_ that tells whether it should copy everything _(All)_ or apply filtering criteria to the source tree _(Selective)_. The underlying utility that will perform the backup is the _Action_. `rsync` is the preselected action but `git` is also a good option for repositories. `auto` action will try guess which is a better fit per directory matched and use that. The _Destination_ consists of two parts. The mount path of a _Device_ and a relative _Path_ that is appended to it. 

Tasks can be executed manually _(Run)_ or triggered automatically when a storage medium is mounted _(On-mount triggering)_. Such triggers need to be _Installed_ and rely on `systemd`. In both cases, a popup confirmation window will be shown before running the task. 

Lastly, tasks should be _Saved_ before getting executed. Use the _Task_ menu. Saving a task will generate a .task file and respective bash script under ~/.lbackup. 


### Installation

Install Qt 5 dependencies and make sure _xterm_, _xmessage_ and _rsync_ commands are available. Qt 5 is usually already installed in recent linux distros. 

For ubuntu/debian based systems:

    $ sudo apt install libqt5core5a libqt5gui5 libqt5gui5-gles libqt5widgets5 xterm rsync
    
Download [latest lbackup release](https://github.com/otsakir/lbackup/releases/latest) in .tar.gz format, extract and run the installation script:

    wget https://github.com/otsakir/lbackup/releases/download/v0.2.0/lbackup_0.2.0_amd64.tar.gz
    tar zxvf lbackup_0.2.0_amd64.tar.gz 
    cd lbackup/
    sudo ./install.sh 

Start the application from the current directory:

    $ ./lbackup 

Note, lbackup relies on **systemd** so it will only work on distros that come with it. Also, `rsync`, `find` and  `git` should be available.

### Build [WIP]

To build without QT Creator install dependencies:

    sudo apt install g++ make qtbase5-dev 

Clone lbackup repository and build it:

    cd lbackup
    mkdir build
    cd build
    qmake ../LisaBackup.pro
    make

The application binary will be placed under App/ directory. There is an [Developer manual](https://github.com/otsakir/lbackup/wiki/Developers-manual) which might help.
    

    



	
    
