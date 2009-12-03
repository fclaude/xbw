
CPP=g++
LXMLFLAGS=-I/usr/include/libxml++-2.6 -I/usr/lib/libxml++-2.6/include -I/usr/include/libxml2 -I/usr/include/glibmm-2.4 -I/usr/lib/glibmm-2.4/include -I/usr/include/sigc++-2.0 -I/usr/lib/sigc++-2.0/include -I/usr/include/glib-2.0 -I/usr/lib/glib-2.0/include  -lxml++-2.6 -lxml2 -lglibmm-2.4 -lgobject-2.0 -lsigc-2.0 -lglib-2.0

LCDSFLAGS=-I./libcds/includes/ 
LCDSLIB=./libcds/lib/libcds.a 

#CPPFLAGS= -g3 -Wall 
CPPFLAGS= -O3 -Wall 

all:
	cd libcds; make
	$(CPP) $(CPPFLAGS) $(LXMLFLAGS) $(LCDSFLAGS) -o parser parser.cpp
	$(CPP) $(CPPFLAGS) $(LCDSFLAGS) -c xbw.cpp
	$(CPP) $(CPPFLAGS) $(LCDSFLAGS) -o engine engine.cpp xbw.o $(LCDSLIB)
	gcc -o gen_xml gen_xml.c

clean:
	cd libcds; make clean
	rm -f gen_xml engine xbw.o parser
