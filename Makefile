X11VNCSERVER_SRCPREFIX=x11-vncserver/src/vncserver
DISPLAYMANAGER_SRCPREFIX=x11-vncserver/src/tinydm
X11VNCSERVER_SRCFILES=$(X11VNCSERVER_SRCPREFIX)/*.c
DISPLAYMANAGER_SRCFILES=$(DISPLAYMANAGER_SRCPREFIX)/*.c

VNCSERVER_LIBS=-lvncserver
XCB_LIBS=-lxcb -lxcb-xtest -lxcb-xfixes -lxcb-keysyms -lxcb-damage
LIBS=$(VNCSERVER_LIBS) $(XCB_LIBS)

TARGET_SERVER=x11-vncserver/tiny-x11vnc
TARGET_TINYDM=x11-vncserver/tiny-dm
all: vncserver tinydm
tinydm:
	$(CC) $(DISPLAYMANAGER_SRCFILES) -o $(TARGET_TINYDM) $(LIBS) -g
vncserver:
	$(CC) $(X11VNCSERVER_SRCFILES) -o $(TARGET_SERVER) $(LIBS) -g

clean:
	rm -rf *.o $(TARGET_SERVER)
