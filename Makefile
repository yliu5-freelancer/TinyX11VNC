VNCSERVER_LIBS=-lvncserver
XCB_LIBS=-lxcb -lxcb-xtest -lxcb-xfixes -lxcb-keysyms
LIBS=$(VNCSERVER_LIBS) $(XCB_LIBS)
TARGET_SERVER=x11-vncserver/simple-x11vnc

all: server
server:
	@echo -e "Building server ..."
	$(CC) ./x11-vncserver/*.c -o $(TARGET_SERVER) $(LIBS)	
	@echo -e "Done."
clean:
	rm -rf *.o $(TARGET_SERVER)
