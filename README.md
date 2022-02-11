# Lisa Backup

A QT backup application with on-mount triggering. Aims at ease of use, minimal user effort and pluggable architecture.

Currently on alpha stage.


### How it works

Backup operations are called _tasks_. A  task copies files from one or more _sources_ to a single _destination_. Each source has a _method_ that tells whether it should copy everything _(All)_ or apply filtering criteria to the source tree _(Selective)_. The underlying utility that will perform the backup is the _Action_. By default `rsync` action is used but `git` is also a good options for repositories. The _Destination_ consists of two parts. The mount path of a _Device_ and a relative _Path_ that is appended to it. 

Tasks can be executed manually _(Run)_ or triggered automatically when a storage medium is mounted _(On-mount triggering)_. Such triggers need to be _Installed_ and rely on `systemd`. In both cases, a popup confirmation window will be shown before running the task. 

Lastly, tasks should be _Saved_ before getting executed. Use the _Task_ menu. Saving a task will generate a .task file and respective bash script under ~/.lbackup. 


### Installation

Install QT dependencies, xterm and rsync from debian/ubuntu repository:

    $ sudo apt install libqt5core5a libqt5gui5 libqt5gui5-gles libqt5widgets5 xterm rsync
    
Download [latest lbackup release](https://github.com/otsakir/lbackup/releases/latest) in .deb format and install

    $ sudo dpkg --install lbackup_{version}_amd64.deb

Start the application from console

    $ lbackup

`rsync`, `find` and  `git` should be available

### Build

To build without QT Creator install dependencies:

    sudo apt install g++ make qtbase5-dev 

Clone lbackup repository and build it:

    cd lbackup
    mkdir build
    cd build
    qmake ../LisaBackup.pro
    make

The application binary will be placed under App/ directory. To install copy it to /usr/bin/lbackup. You'll also need to copy the contents of scripts/ directory under /usr/share/lbackup/.
    

    



	
    
