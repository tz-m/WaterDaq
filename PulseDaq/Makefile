CXX=`root-config --cxx`
CXXFLAGS=`root-config --cflags` -g -I/usr/include -Wall
LDFLAGS=`root-config --ldflags`
LDLIBS=`root-config --glibs` -lCAENVME
SOURCES=PulseDaq.cc MQDC32.cc VX1290A.cc Common.cc User_Settings.cc
OBJECTS=$(SOURCES:.cc=.o)
EXECUTABLE=PulseDaq

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CXX) $(LDFLAGS) -o $@ $^ $(LDLIBS) 

.cc.o:
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(CFLAGS) -W -c $<

clean:
	rm -f ./*~ $(OBJECTS) ./PulseDaq
