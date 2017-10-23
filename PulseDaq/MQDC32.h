#ifndef  MQDC32_H
#define  MQDC32_H



#define MQDC32_BASE    0x11110000

#define  MQDC32_EMPTY  1
#define  MQDC32_MAX_CHANNELS               32
#define  MQDC32_EVENT_READOUT_BUFFER            0x0000 /* R/W D32, D64 */

// Thershold memory
#define MQDC32_THRESHOLD_0             0x4000      /* R/W def:0 */
/*                                 up to 0x403E for threshold 31 */

// Address Registers
#define MQDC32_ADD_SOURCE              0x6000      /* R/W def:0 D16  */ 
#define MQDC32_ADD_REG                 0x6002      /* R/W def:0 D16  */ 
#define MQDC32_MODULE_ID               0x6004      /* R/W def:0xff D16 */ 
#define MQDC32_SOFT_RESET              0x6008      /* W  D16 */ 
#define MQDC32_FIRM_REV                0x600E      /* R   def:0x01.10  D16 */ 

// Interrupt
#define MQDC32_IRQ_LEVEL               0x6010      /* R/W  def:0 D16 */
#define MQDC32_IRQ_VECTOR              0x6012      /* RW def:0 */
#define MQDC32_IRQ_TEST                0x6014      /* W */
#define MQDC32_IRQ_RESET               0x6016      /* W */
#define MQDC32_IRQ_DATA_THRESHOLD      0x6018      /* RW def:1 */
#define MQDC32_MAX_TRANSFER_DATA       0x601A      /* RW def:1 */

// MCST, CBLT
#define MQDC32_CBLT_MCST_CTL           0x6020      /* R/W  def:0 */ 
#define MQDC32_CBLT_ADD                0x6022      /* R/W def:0xAA A31..A25 CBLT add */ 
#define MQDC32_MCST_ADD                0x6024      /* R   def:0xbb A31..A25 MCST add */ 

// Fifo handling
#define MQDC32_BUF_DATA_LEN            0x6030      /* R  */ 
#define MQDC32_DATA_LEN_FMT            0x6032      /* R/W def:2 */ 
#define MQDC32_READOUT_RESET           0x6034      /* W */ 
#define MQDC32_MULTIEVENT              0x6036      /* R/W def:0 */ 
#define MQDC32_MARKING_TYPE            0x6038      /* R/W def:0 */  
#define MQDC32_START_ACQ               0x603A      /* R/W def:1 */ 
#define MQDC32_FIFO_RESET              0x603C      /* W */ 
#define MQDC32_DATA_READY              0x603E      /* R */ 

// Operation mode
#define MQDC32_BANK_OPERATION          0x6040      /* R/W def:0 */ 
#define MQDC32_QDC_RESOLUTION          0x6042	     /* R/W def:0 */ 
#define MQDC32_OFFSET_BANK0            0x6044      /* RW Chans 0-15, range 0-255, shifts 4k spectrum by +/- 1000 bins */
#define MQDC32_OFFSET_BANK1            0x6046      /* same as bank0 for chans 16-31 */
#define MQDC32_SLC_OFF                 0x6048      /* rw def:0 */
#define MQDC32_SKIP_OORANGE            0x604A      /* rw def:0 */
#define MQDC32_IGNORE_THRESHOLDS       0x604C      /* rw def:0 */

// Gate limit (see lookup table in docs)
#define MQDC32_LIMIT_BANK0             0x6050      /* rw 4=4.5ns, 255=nolim, bank0 */
#define MQDC32_LIMIT_BANK1             0x6052      /* rw same for bank1 */

// Experiment Trigger
#define MQDC32_EXP_TRIG_DELAY_BANK0    0x6054      /* rw def:0 */
#define MQDC32_EXP_TRIG_DELAY_BANK1    0x6056      /* rw def:0 */

// I/O  
#define MQDC32_INPUT_COUPLING          0x6060      /* R/W def:b000 */ 
#define MQDC32_ECL_TERM                0x6062      /* R/W def:b11000 */ 
#define MQDC32_ECL_GATE1_OSC           0x6064      /* R/W def:0 */ 
#define MQDC32_ECL_FC_RESET            0x6066      /* R/W def:1 */ 
#define MQDC32_ECL_GATE_SELECT         0x6068      /* R/W def:0 */ 
#define MQDC32_NIM_GATE1_OSC           0x606A      /* R/W def:0 */ 
#define MQDC32_NIM_FC_RESET            0x606C      /* R/W def:0 */ 
#define MQDC32_NIM_BUSY                0x606E      /* R/W def:0 */ 

// Test pulser
#define MQDC32_TESTPULSER              0x6070      /* R/W def:0 */ 

// Module RC-bus for communication to the MSCF16
#define MQDC32_RC_BUSNO                0x6080      /* R/W def:0 */ 
#define MQDC32_RC_MODNUM               0x6082      /* R/W def:0 */ 
#define MQDC32_RC_OPCODE               0x6084      /* R/W  */ 
#define MQDC32_RC_ADDRESS              0x6086      /* R/W  */ 
#define MQDC32_RC_DATA                 0x6088      /* R/W  */ 
#define MQDC32_RC_SENT_RTN_STAT        0x608A      /* R  def:0 */ 

// CTRA
#define MQDC32_RESET_CTL_AB            0x6090      /* R/W */
#define MQDC32_EV_CTL_LOW              0x6092      /* R/W */
#define MQDC32_EV_CTL_HIGH             0x6094      /* R/W */
#define MQDC32_TS_SOURCE               0x6096      /* R/W */
#define MQDC32_TS_DIVIDER              0x6098      /* R/W */
#define MQDC32_TS_COUNTER_LOW          0x609C      /* R */
#define MQDC32_TS_COUNTER_HIGH         0x609E      /* R */

// CTRB
#define MQDC32_ADC_BUSY_TIME_LOW       0x60A0      /* R */
#define MQDC32_ADC_BUSY_TIME_HIGH      0x60A2      /* R */
#define MQDC32_GATE1_TIME_LOW          0x60A4      /* R */
#define MQDC32_GATE1_TIME_HIGH         0x60A6      /* R */
#define MQDC32_TIME_0                  0x60A8      /* R */
#define MQDC32_TIME_1                  0x60AA      /* R */
#define MQDC32_TIME_2                  0x60AC      /* R */
#define MQDC32_STOP_CTR                0x60AE      /* R/W */

// Multiplicity filter
#define MQDC32_HIGH_LIMIT_BANK0        0x60B0      /* rw def:32 */
#define MQDC32_LOW_LIMIT_BANK0         0x60B2      /* rw def:0  */
#define MQDC32_HIGH_LIMIT_BANK1        0x60B4      /* rw def:16 */
#define MQDC32_LOW_LIMIT_BANK1         0x60B6      /* rw def:0  */

#define MQDC32_ACQ_STOP                     0
#define MQDC32_ACQ_START                    1
#define MQDC32_MARK_TIME                    1
#define MQDC32_MARK_EVENT                   0
#define DONE                                0

#include "Common.h"

CVErrorCodes MQDC32_Read_Register(int32_t Handle, uint32_t address, uint32_t *data);
CVErrorCodes MQDC32_Write_Register(int32_t Handle, uint32_t address, uint32_t data);
CVErrorCodes MQDC32_Read_BLT(int32_t Handle, uint32_t address, uint32_t *data);
CVErrorCodes MQDC32_Setup(int32_t Handle);
CVErrorCodes MQDC32_Reset_Data_Buffer(int32_t Handle);
CVErrorCodes MQDC32_Read_Word(int32_t Handle, uint32_t * data);

bool MQDC32_IsHeader(uint32_t word);
bool MQDC32_IsData(uint32_t word);
bool MQDC32_IsEoE(uint32_t word);

struct MQDC32_Header {
  uint32_t num_words;
  uint32_t fill;
  uint32_t module_id;
  uint32_t subheader;
  uint32_t hsig;
  void Print() { 
    std::cout << "Header: num_words=" << std::setw(4) << num_words 
	      << " fill=" << std::setw(1) << fill 
	      << " module_id=" << std::setw(3) << module_id 
	      << " subheader=" << std::setw(2) << subheader 
	      << " hsig=" << std::setw(1) << hsig << std::endl; 
  }
};

struct MQDC32_Data {
  uint32_t adc;
  uint32_t overflow;
  uint32_t channel;
  uint32_t fix;
  uint32_t dsig;
  void Print() {
    std::cout << "adc=" << std::setw(4) << adc 
	      << " overflow=" << std::setw(1) << overflow 
	      << " channel=" << std::setw(2) << channel 
	      << " fix=" << std::setw(3) << fix 
	      << " dsig=" << std::setw(1) << dsig << std::endl; 
  }
};

struct MQDC32_EoE {
  uint32_t timestamp;
  uint32_t esig;
  void Print() { 
    std::cout << "timestamp=" << std::setw(10) << timestamp 
	      << " esig=" << std::setw(1) << esig << std::endl; 
  }
};

void MQDC32_ParseHeaderWord(uint32_t word, MQDC32_Header * head);
void MQDC32_ParseDataWord(uint32_t word, MQDC32_Data * data);
void MQDC32_ParseEoEWord(uint32_t word, MQDC32_EoE * eoe);

struct MQDC32_EnableChannel {
  std::vector<bool> ch;
MQDC32_EnableChannel() : ch(32,false) {}
  bool q(uint32_t c) {
    if (c<=31) return ch.at(c);
    return false;
  }
};

CVErrorCodes MQDC32_ReadEvent(int32_t handle, MQDC32_Header * head, MQDC32_Data * data, MQDC32_EoE * eoe, MQDC32_EnableChannel chans);

#endif  //  MQDC32_H
