CXX=`root-config --cxx`
CXXFLAGS=`root-config --cflags` -g -Wall 
##CXXFLAGS=`root-config --cflags` -pg
LDFLAGS=`root-config --ldflags`
##LDFLAGS=`root-config --ldflags` -pg
LDLIBS=`root-config --glibs` -lAgMD2
SOURCES=DigiDaq.cc waveform.cc
OBJECTS=$(SOURCES:.cc=.o)
EXECUTABLE=DigiDaq waveform

all: $(SOURCES) $(EXECUTABLE)

DigiDaq: DigiDaq.o
	$(CXX) $(LDFLAGS) -lAgMD2 -o $@ $^ $(LDLIBS) 

waveform: waveform.o
	$(CXX) $(LDFLAGS) -lAgMD2 -o $@ $^ $(LDLIBS)

.cc.o:
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(CFLAGS) -W -Wall -c $<

clean:
	rm -f ./*~ $(OBJECTS) ./DigiDaq ./waveform



