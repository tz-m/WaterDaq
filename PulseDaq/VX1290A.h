#ifndef VX1290A_H
#define VX1290A_H

#define VX1290A_OUT_BUFFER_ADD 0x0000
#define VX1290A_CONTROL_ADD 0x1000
#define VX1290A_STATUS_ADD 0x1002
#define VX1290A_INT_LEVEL_ADD	0x100A
#define VX1290A_INT_VECTOR_ADD 0x100C
#define VX1290A_GEO_ADDRESS_ADD 0x100E
#define VX1290A_MCST_CBLT_ADDRESS_ADD	0x1010
#define VX1290A_MCST_CBLT_CTRL_ADD 0x1012
#define VX1290A_MOD_RESET_ADD	0x1014
#define VX1290A_SW_CLEAR_ADD 0x1016
#define VX1290A_SW_EVENT_RESET_ADD 0x1018
#define VX1290A_SW_TRIGGER_ADD 0x101A
#define VX1290A_EVENT_COUNTER_ADD 0x101C
#define VX1290A_EVENT_STORED_ADD 0x1020
#define VX1290A_ALMOST_FULL_LVL_ADD 0x1022
#define VX1290A_BLT_EVENT_NUM_ADD 0x1024
#define VX1290A_FW_REV_ADD 0x1026
#define VX1290A_TESTREG_ADD 0x1028
#define VX1290A_OUT_PROG_CTRL_ADD 0x102C
#define VX1290A_MICRO_ADD 0x102E
#define VX1290A_MICRO_HND_ADD	0x1030
#define VX1290A_SEL_FLASH_ADD	0x1032
#define VX1290A_FLASH_ADD 0x1034
#define VX1290A_COMP_SRAM_PAGE_ADD 0x1036
#define VX1290A_EVENT_FIFO_ADD 0x1038
#define VX1290A_EVENT_FIFO_STORED_ADD	0x103C
#define VX1290A_EVENT_FIFO_STATUS_ADD	0x103E
#define VX1290A_DUMMY32_ADD 0x1200
#define VX1290A_DUMMY16_ADD 0x1204
#define VX1290A_ROM_OUI_2_ADD	0x4024
#define VX1290A_ROM_OUI_1_ADD	0x4028
#define VX1290A_ROM_OUI_0_ADD	0x402C
#define VX1290A_ROM_VERSION_ADD 0x4030
#define VX1290A_ROM_BOARD_ID_2_ADD 0x4034
#define VX1290A_ROM_BOARD_ID_1_ADD 0x4038
#define VX1290A_ROM_BOARD_ID_0_ADD 0x403C
#define VX1290A_ROM_REVISION_3_ADD 0x4040
#define VX1290A_ROM_REVISION_2_ADD 0x4044
#define VX1290A_ROM_REVISION_1_ADD 0x4048
#define VX1290A_ROM_REVISION_0_ADD 0x404C
#define VX1290A_ROM_SERIAL_1_ADD 0x4080
#define VX1290A_ROM_SERIAL_0_ADD 0x4084

#define	VX1290A_TRG_MATCH_OPCODE 0x0000
#define	VX1290A_CONT_STORE_OPCODE 0x0100
#define	VX1290A_READ_ACQ_MOD_OPCODE 0x0200
#define	VX1290A_SET_KEEP_TOKEN_OPCODE 0x0300
#define	VX1290A_CLEAR_KEEP_TOKEN_OPCODE 0x0400
#define	VX1290A_LOAD_DEF_CONFIG_OPCODE 0x0500
#define	VX1290A_SAVE_USER_CONFIG_OPCODE 0x0600
#define	VX1290A_LOAD_USER_CONFIG_OPCODE 0x0700
#define	VX1290A_AUTOLOAD_USER_CONFIG_OPCODE 0x0800
#define	VX1290A_AUTOLOAD_DEF_CONFIG_OPCODE 0x0900

#define	VX1290A_SET_WIN_WIDTH_OPCODE 0x1000
#define	VX1290A_SET_WIN_OFFSET_OPCODE 0x1100
#define	VX1290A_SET_SW_MARGIN_OPCODE 0x1200
#define	VX1290A_SET_REJ_MARGIN_OPCODE 0x1300
#define VX1290A_EN_SUB_TRG_OPCODE 0x1400
#define	VX1290A_DIS_SUB_TRG_OPCODE 0x1500
#define	VX1290A_READ_TRG_CONF_OPCODE 0x1600

#define	VX1290A_SET_DETECTION_OPCODE 0x2200
#define	VX1290A_READ_DETECTION_OPCODE 0x2300
#define	VX1290A_SET_TR_LEAD_LSB_OPCODE 0x2400
#define	VX1290A_SET_PAIR_RES_OPCODE 0x2500
#define	VX1290A_READ_RES_OPCODE 0x2600
#define	VX1290A_SET_DEAD_TIME_OPCODE 0x2800
#define	VX1290A_READ_DEAD_TIME_OPCODE 0x2900

#define	VX1290A_EN_HEAD_TRAILER_OPCODE 0x3000
#define	VX1290A_DIS_HEAD_TRAILER_OPCODE 0x3100
#define	VX1290A_READ_HEAD_TRAILER_OPCODE 0x3200
#define	VX1290A_SET_EVENT_SIZE_OPCODE 0x3300
#define	VX1290A_READ_EVENT_SIZE_OPCODE 0x3400
#define	VX1290A_EN_ERROR_MARK_OPCODE 0x3500
#define	VX1290A_DIS_ERROR_MARK_OPCODE 0x3600
#define	VX1290A_EN_ERROR_BYPASS_OPCODE 0x3700
#define	VX1290A_DIS_ERROR_BYPASS_OPCODE 0x3800
#define	VX1290A_SET_ERROR_TYPES_OPCODE 0x3900
#define	VX1290A_READ_ERROR_TYPES_OPCODE 0x3A00
#define	VX1290A_SET_FIFO_SIZE_OPCODE 0x3B00
#define	VX1290A_READ_FIFO_SIZE_OPCODE 0x3C00

#define	VX1290A_EN_CHANNEL_OPCODE 0x4000
#define	VX1290A_DIS_CHANNEL_OPCODE 0x4100
#define	VX1290A_EN_ALL_CH_OPCODE 0x4200
#define	VX1290A_DIS_ALL_CH_OPCODE 0x4300
#define	VX1290A_WRITE_EN_PATTERN_OPCODE 0x4400
#define	VX1290A_READ_EN_PATTERN_OPCODE 0x4500
#define	VX1290A_WRITE_EN_PATTERN32_OPCODE 0x4600
#define	VX1290A_READ_EN_PATTERN32_OPCODE 0x4700

#define	VX1290A_SET_GLOB_OFFSET_OPCODE 0x5000
#define	VX1290A_READ_GLOB_OFFSET_OPCODE 0x5100
#define	VX1290A_SET_ADJUST_CH_OPCODE 0x5200
#define	VX1290A_READ_ADJUST_CH_OPCODE 0x5300
#define	VX1290A_SET_RC_ADJ_OPCODE 0x5400
#define	VX1290A_READ_RC_ADJ_OPCODE 0x5500
#define	VX1290A_SAVE_RC_ADJ_OPCODE 0x5600

#define	VX1290A_READ_TDC_ID_OPCODE 0x6000
#define	VX1290A_READ_MICRO_REV_OPCODE 0x6100
#define	VX1290A_RESET_DLL_PLL_OPCODE 0x6200

#define	VX1290A_WRITE_SETUP_REG_OPCODE 0x7000
#define	VX1290A_READ_SETUP_REG_OPCODE 0x7100
#define	VX1290A_UPDATE_SETUP_REG_OPCODE 0x7200
#define	VX1290A_DEFAULT_SETUP_REG_OPCODE 0x7300
#define	VX1290A_READ_ERROR_STATUS_OPCODE 0x7400
#define	VX1290A_READ_DLL_LOCK_OPCODE 0x7500
#define	VX1290A_READ_STATUS_STREAM_OPCODE 0x7600
#define	VX1290A_UPDATE_SETUP_TDC_OPCODE 0x7700

#define	VX1290A_WRITE_EEPROM_OPCODE 0xC000
#define	VX1290A_READ_EEPROM_OPCODE 0xC100
#define	VX1290A_MICROCONTROLLER_FW_OPCODE 0xC200
#define	VX1290A_WRITE_SPARE_OPCODE 0xC300
#define	VX1290A_READ_SPARE_OPCODE 0xC400
#define	VX1290A_EN_TEST_MODE_OPCODE 0xC500
#define	VX1290A_DIS_TEST_MODE_OPCODE 0xC600
#define	VX1290A_SET_TDC_TEST_OUTPUT_OPCODE 0xC700
#define	VX1290A_SET_DLL_CLOCK_OPCODE 0xC800
#define	VX1290A_READ_TDC_SETUP_SCAN_PATH_OPCODE 0xC800

#define VX1290A_READ_OK 0x2
#define VX1290A_WRITE_OK 0x1

#include "Common.h"
#include "User_Settings.h"

typedef struct {
  uint32_t DATA_READY : 1;
  uint32_t ALM_FULL : 1;
  uint32_t FULL : 1;
  uint32_t TRG_MATCH : 1;
  uint32_t HEADER_EN : 1;
  uint32_t TERM_ON : 1;
  uint32_t ERROR0 : 1;
  uint32_t ERROR1 : 1;
  uint32_t ERROR2 : 1;
  uint32_t ERROR3 : 1;
  uint32_t BERR : 1;
  uint32_t PURG : 1;
  uint32_t RES_0 : 1;
  uint32_t RES_1 : 1;
  uint32_t PAIR : 1;
  uint32_t TRIGGER_LOST : 1;
} Status;

void PrintStatus(Status stat);

CVErrorCodes VX1290A_Write_Register(int32_t Handle, uint32_t address, uint32_t data);
CVErrorCodes VX1290A_TouchWrite_OpCode(int32_t Handle, uint32_t opaddress, uint32_t data);
CVErrorCodes VX1290A_Write_OpCode(int32_t Handle, uint32_t opaddress);
CVErrorCodes VX1290A_Read_Register(int32_t Handle, uint32_t address, uint32_t * data);
CVErrorCodes VX1290A_TouchRead_OpCode(int32_t Handle, uint32_t opaddress, uint32_t * data);
CVErrorCodes VX1290A_Read_OpCode(int32_t Handle, uint32_t opaddress);
CVErrorCodes VX1290A_Read_Word(int32_t Handle, uint32_t * data);

CVErrorCodes VX1290A_Setup(int32_t Handle, Settings set);

CVErrorCodes VX1290A_Status(int32_t Handle, Status * status);
CVErrorCodes VX1290A_Clear(int32_t Handle);
CVErrorCodes VX1290A_Trigger(int32_t Handle);

bool VX1290A_IsGlobalHeader(uint32_t word);
bool VX1290A_IsGlobalTrailer(uint32_t word);
bool VX1290A_IsTDCHeader(uint32_t word);
bool VX1290A_IsTDCMeasurement(uint32_t word);
bool VX1290A_IsTDCError(uint32_t word);
bool VX1290A_IsTDCTrailer(uint32_t word);
bool VX1290A_IsGlobalTrigTime(uint32_t word);

struct VX1290A_GlobalHeader {
  uint32_t geo;
  uint32_t evt_cnt;
  uint32_t id;
  void Print() {
    std::cout << "geo=" << std::setw(2) << geo
	      << " evt_cnt=" << std::setw(7) << evt_cnt
	      << " id=" << std::bitset<5>(id) << std::endl;
  }
};

struct VX1290A_GlobalTrailer {
  uint32_t geo;
  uint32_t word_cnt;
  uint32_t status_tdcerr;
  uint32_t status_overflow;
  uint32_t status_triglost;
  uint32_t id;
  void Print() {
    std::cout << "geo=" << std::setw(2) << geo
	      << " word_cnt=" << std::setw(5) << word_cnt
	      << " status_tdcerr=" << std::bitset<1>(status_tdcerr)
	      << " status_overflow=" << std::bitset<1>(status_overflow)
	      << " status_triglost=" << std::bitset<1>(status_triglost)
	      << " id=" << std::bitset<5>(id) << std::endl;
  }
};

struct VX1290A_TDCHeader {
  uint32_t bunch_id;
  uint32_t event_id;
  uint32_t tdc;
  uint32_t id;
  void Print() {
    std::cout << "bunch_id=" << std::setw(4) << bunch_id
	      << " event_id=" << std::setw(4) << event_id
	      << " tdc=" << std::setw(1) << tdc
	      << " id=" << std::bitset<5>(id) << std::endl;
  }
};

struct VX1290A_TDCMeasurement {
  uint32_t tdc_meas;
  uint32_t channel;
  uint32_t leadtrail;
  uint32_t id;
  void Print() {
    std::cout << "tdc_meas=" << std::setw(7) << tdc_meas
	      << " channel=" << std::setw(2) << channel
	      << " leadtrail=" << std::bitset<1>(leadtrail)
	      << " id=" << std::bitset<5>(id) << std::endl;
  }
};

struct VX1290A_TDCError {
  uint32_t err_flags;
  uint32_t tdc;
  uint32_t id;
  void Print() {
    std::cout << "err_flags=" << std::bitset<15>(err_flags)
	      << " tdc=" << std::setw(2) << tdc
	      << " id=" << std::bitset<5>(id) << std::endl;
  }
};

struct VX1290A_TDCTrailer {
  uint32_t word_cnt;
  uint32_t evt_id;
  uint32_t tdc;
  uint32_t id;
  void Print() {
    std::cout << "word_cnt=" << std::setw(4) << word_cnt 
	      << " evt_id=" << std::setw(4) << evt_id
	      << " tdc=" << std::setw(1) << tdc
	      << " id=" << std::bitset<5>(id) << std::endl;
  }
};

struct VX1290A_GlobalTrigTime {
  uint32_t trig_time;
  uint32_t id;
  void Print() {
    std::cout << "trig_time=" << std::setw(9) << trig_time
	      << " id=" << std::bitset<5>(id) << std::endl;
  }
};

void VX1290A_ParseGlobalHeader(uint32_t word, VX1290A_GlobalHeader * head);
void VX1290A_ParseGlobalTrailer(uint32_t word, VX1290A_GlobalTrailer * trail);
void VX1290A_ParseTDCHeader(uint32_t word, VX1290A_TDCHeader * head);
void VX1290A_ParseTDCMeasurement(uint32_t word, VX1290A_TDCMeasurement * meas);
void VX1290A_ParseTDCError(uint32_t word, VX1290A_TDCError * err);
void VX1290A_ParseTDCTrailer(uint32_t word, VX1290A_TDCTrailer * trail);
void VX1290A_ParseGlobalTrigTime(uint32_t word, VX1290A_GlobalTrigTime * trig);

CVErrorCodes VX1290A_ReadEvent(int32_t Handle, 
			       VX1290A_GlobalHeader * gh, 
			       VX1290A_GlobalTrailer * gt, 
			       std::vector<VX1290A_TDCHeader> * th_v, 
			       std::vector<VX1290A_TDCMeasurement> * tm_v, 
			       std::vector<VX1290A_TDCError> * te_v, 
			       std::vector<VX1290A_TDCTrailer> * tt_v, 
			       VX1290A_GlobalTrigTime * gtt,
			       Settings set);

#endif
