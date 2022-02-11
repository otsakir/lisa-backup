
LBACKUP_VERSION := $(shell bash -c 'grep Version  lbackup/DEBIAN/control | sed -e "s/Version:\s*//" -e "s/\s+//"')

all: 	scripts lbackup.deb doversion 

lbackup/usr/bin/lbackup: ../build/App/App
	mkdir -p lbackup/usr/bin
	cp ../build/App/App lbackup/usr/bin/lbackup

scripts:
	mkdir -p lbackup/usr/share/lbackup
	cp -r ../scripts/* lbackup/usr/share/lbackup/


lbackup.deb: lbackup/usr/bin/lbackup lbackup/DEBIAN/control 
	dpkg-deb --build --root-owner-group lbackup


doversion: lbackup.deb
	cp lbackup.deb lbackup_$(LBACKUP_VERSION)-1_amd64.deb







