CXX=`root-config --cxx`
CXXFLAGS=`root-config --cflags` -g -Wall 
##CXXFLAGS=`root-config --cflags` -pg
LDFLAGS=`root-config --ldflags`
##LDFLAGS=`root-config --ldflags` -pg
LDLIBS=`root-config --glibs`
SOURCES=MesyDaq.c mesqdc32.c
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=MesyDaq mesqdc32

all: $(SOURCES) $(EXECUTABLE)

MesyDaq: MesyDaq.o
	$(CXX) $(LDFLAGS) -o $@ $^ $(LDLIBS) 

mesqdc32: mesqdc32.o
	$(CXX) $(LDFLAGS) -o $@ $^ $(LDLIBS)

.c.o:
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(CFLAGS) -W -Wall -c $<

clean:
	rm -f ./*~ $(OBJECTS) ./MesyDaq
