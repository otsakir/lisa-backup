
LBACKUP_VERSION := $(shell bash -c 'grep Version  lbackup-vanilla/DEBIAN/control | sed -e "s/Version:\s*//" -e "s/\s+//"')

default: debian installer

.PHONY:	debian scripts installer

lbackup:
	cp -r lbackup-vanilla lbackup

debian: lbackup scripts lbackup.deb doversion 

lbackup/usr/bin/lbackup: ../build/App/LisaBackup ../build/App/libLisaBackuplib.so
	mkdir -p lbackup/usr/bin
	cp ../build/App/LisaBackup lbackup/usr/bin/lbackup
	cp ../build/App/libLisaBackuplib.so lbackup/usr/bin/libLisaBackuplib.so

scripts: lbackup
	mkdir -p lbackup/usr/share/lbackup
	cp -r ../scripts/app/* lbackup/usr/share/lbackup/


lbackup.deb: lbackup/usr/bin/lbackup lbackup/DEBIAN/control 
	dpkg-deb --build --root-owner-group lbackup

doversion: lbackup.deb
	cp lbackup.deb lbackup_$(LBACKUP_VERSION)-1_amd64.deb

installer:
	rm -rf installer/lbackup
	[ -d installer/lbackup/scripts ] || mkdir -p installer/lbackup/scripts
	cp ../build/App/LisaBackup installer/lbackup/lbackup
	cp ../build/App/libLisaBackuplib.so installer/lbackup/
	cp ../scripts/installer/*.sh installer/lbackup/
	cp -r ../scripts/app/* installer/lbackup/scripts
	cd installer && tar zcvf lbackup_$(LBACKUP_VERSION)_amd64.tar.gz lbackup
	mv installer/lbackup_$(LBACKUP_VERSION)_amd64.tar.gz .
	
standalone:
	rm -rf standalone
	mkdir -p 	standalone/lbackup
	cp ../build/App/LisaBackup standalone/lbackup/
	cp ../build/App/libLisaBackuplib.so standalone/lbackup/
	cp ../scripts/standalone/* standalone/lbackup/
	cd standalone && tar zcvf lbackup_$(LBACKUP_VERSION)_standalone_amd64.tar.gz lbackup
	
clean:
	rm -f *.deb *.tar.gz
	rm -rf installer/
	rm -rf lbackup
	rm -rf standalone

	









