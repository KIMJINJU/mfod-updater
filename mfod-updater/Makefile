DIR_CORE    = src/core/
DIR_DS	    = src/ds/
DIR_ETC	    = src/etc/
DIR_GUI     = src/gui/
DIR_MODULES = src/modules/
DIR_OBJECTS	= obj/
DIR_OUT		= out/

all: core ds etc gui modules copy_objects link_objects copy_binary

core:
	cd $(DIR_CORE); $(MAKE) core

ds:
	cd $(DIR_DS); $(MAKE) ds

etc:
	cd $(DIR_ETC); $(MAKE) etc

gui:
	cd $(DIR_GUI); $(MAKE) gui

modules:
	cd $(DIR_MODULES); $(MAKE) modules

copy_objects:
	cp $(DIR_CORE)*.o $(DIR_OBJECTS)
	cp $(DIR_DS)*.o $(DIR_OBJECTS)
	cp $(DIR_ETC)*.o $(DIR_OBJECTS)
	cp $(DIR_GUI)*.o $(DIR_OBJECTS)
	cp $(DIR_MODULES)*.o $(DIR_OBJECTS)

link_objects:
	cd $(DIR_OBJECTS); $(MAKE) link_objects

copy_binary:
	cp $(DIR_OBJECTS)mfod-updater $(DIR_OUT)
	sudo cp $(DIR_OBJECTS)mfod-updater /opt/imx6-resource/rootfs/mfod-mmcblk0p2/root/mfod
	#cp $(DIR_OBJECTS)amod-mainapp /home/shpark/workspace/imx6-bsp/tftpboot


clean:
	cd $(DIR_CORE); $(MAKE) clean
	cd $(DIR_DS); $(MAKE) clean
	cd $(DIR_ETC); $(MAKE) clean
	cd $(DIR_GUI); $(MAKE) clean
	cd $(DIR_MODULES); $(MAKE) clean
	cd $(DIR_OBJECTS); $(MAKE) clean
