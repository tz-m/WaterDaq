CXX=`root-config --cxx`
CXXFLAGS=`root-config --cflags` -g -I/usr/include -Wall
LDFLAGS=`root-config --ldflags`
LDLIBS=`root-config --glibs` -lCAENVME
SOURCES=MesyDaq.cc MQDC32.cc
OBJECTS=$(SOURCES:.cc=.o)
EXECUTABLE=MesyDaq

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CXX) $(LDFLAGS) -o $@ $^ $(LDLIBS) 

.cc.o:
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(CFLAGS) -W -c $<

clean:
	rm -f ./*~ $(OBJECTS) ./MesyDaq
