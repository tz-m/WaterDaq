M. Thiesse
24 October 2017

----------------------------PulseDaq---------------------------

Prerequisites:
	ROOT v6.10/06 (other versions may work, so far only tested with this one)
	CAENVMElib v2.50
	reasonably recent Linux distribution with dev headers
	C++11 or better (whatever is required by ROOT)
	
	Not sure about these, but better be safe:
	    CAENComm 1.2
	    CAENUSBdrvB 1.5.1

User_Settings.h
	Set board addresses
	Set channel numbers 
	    **Currently supports reading 1 MQDC32 channel with 2 VX1290A
	    channels. If you want something different, must think about 
	    re-writing the code.
	Set VX1290A window width
	Set VX1290A window offset 

Building:
	make

Running:
	./PulseDaq [options]
Options:
	-v	Verbose (optional)
	-d [N]  Delay between acquired pulses (in microseconds) (required)
	-n [N]  Number of pulses to acquire (required)


Notes:
	This program is developed specifically for the DAQ setup in Hicks D46 for reading out the charge and rise time of PMT pulses using a MQDC-32 and VX1290A controlled with V1718. Though, that is not to say the program can't be generalised for wider use. That would require extra thought and work which just isn't necessary right now. 
	PulseDaq.cc/h define the main program methods. I.e. the control of the flow of the program. Command line options are accepted for the most common data acquisition configuration parameters and debugging options. A ROOT TTree is outputted with the various measured values. Hopefully, the events from both VME modules are lined up. That is, the TDC values measured with the VX1290A correspond to the same pulse which the MQDC32 measures the charge. Can test this by lowering the duty cycle of the pulser so that the time between PMT pulses is on human noticeable time scales.
	User_Settings.h define the slightly lower level configuration parameters. Things that need to be set only once during an experiment, immediately following setup.
	Common.cc/h contain basic functions and header includes common to most, if not all, other source files. These should be independent of hardware or experiment settings.
	MQDC32.cc/h contain specific methods to read and write data to and from the Mesytec MQDC-32 via the CAENVMElib base functions. This also contains hardware addresses and controls the parsing of raw data from the hardware registers, which is entirely dependent on the specific information in the module documentation. I cannot stress enough how important every bit of the documentation is in understanding the reason behind the madness in this file. Also, Mesytec doesn't seem to document procedures very well, just definitions. Anyway, hopefully the files here are clear enough to understand how the module works.
	VX1290A.cc/h contain specific methods to read and write data to and from the CAEN VX1290A TDC via the CAENVMElib base functions. Same as previously, the hardware addresses and methods for accessing the data are included. CAEN documents their module a lot better.
	The hardest part of developing the DAQ here is knowing in advance when to expect the pulses, in order to setup the TDC settings properly and initiate triggers. Good luck!


For help with the TDC, download the CAENV1x90 code from Caen. Install (make && make install) the SDK first (to fix the -Wall,-soname error, just delete ",-soname" from the command in the makefile). Once that is done, build the Demo (make). To use it, 
    ./CAENV1190Demo -A0xEEEE -M"1, 0, 1, 0x7d0, 0xfc18, 0xfc18, 2, 2, 3, 0xffff, 0xffff, 0xffff, 0xffff" -TV1290A

Make sure the TDC trigger input is 50Ohm terminated, otherwise no triggers will be detected!!!

When setting the parameters on the MCFD-16 units, set the gain very low and the threshold high on all unused channels. Only set the correct gain and threshold for the channels you want to read. This is because there is no easy way to enable/disable channels on the MQDC-32, and having non-connected channels triggering on noise only causes large numbers of "Error" pulses (i.e. channel number != MQDC32_CHANNEL_CHARGE).
