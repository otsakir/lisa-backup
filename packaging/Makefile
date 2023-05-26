
LBACKUP_VERSION := $(shell bash -c 'grep Version  lbackup/DEBIAN/control | sed -e "s/Version:\s*//" -e "s/\s+//"')

default: debian installer

.PHONY:	debian scripts installer


debian:	scripts lbackup.deb doversion 

lbackup/usr/bin/lbackup: ../build/App/App
	mkdir -p lbackup/usr/bin
	cp ../build/App/App lbackup/usr/bin/lbackup

scripts:
	mkdir -p lbackup/usr/share/lbackup
	cp -r ../scripts/app/* lbackup/usr/share/lbackup/


lbackup.deb: lbackup/usr/bin/lbackup lbackup/DEBIAN/control 
	dpkg-deb --build --root-owner-group lbackup

doversion: lbackup.deb
	cp lbackup.deb lbackup_$(LBACKUP_VERSION)-1_amd64.deb

installer:
	rm -rf installer/lbackup
	[ -d installer/lbackup/scripts ] || mkdir -p installer/lbackup/scripts
	cp ../build/App/App installer/lbackup/lbackup
	cp ../scripts/installer/*.sh installer/lbackup/
	cp -r ../scripts/app/* installer/lbackup/scripts
	cd installer && tar zcvf lbackup_$(LBACKUP_VERSION)_amd64.tar.gz lbackup
	mv installer/lbackup_$(LBACKUP_VERSION)_amd64.tar.gz .
	









