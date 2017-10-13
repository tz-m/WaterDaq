/*********************************************************************

  Name:         mesqdc32drv.h
  Created by:   K.Olchanski

  Contents:     mesqdc32 8-channel 200 MHz 12-bit QDC

  $Id: mesqdc32drv.h 5177 2011-08-12 02:26:08Z amaudruz $
                
*********************************************************************/
#ifndef  MESQDC32DRV_INCLUDE_H
#define  MESQDC32DRV_INCLUDE_H

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "mvmestd.h"

#include "mesqdc32.h"
int      mesqdc32_ReadData(MVME_INTERFACE *mvme, uint32_t a32base, uint32_t *pdata, int *len);
void     mesqdc32_RegisterWrite(MVME_INTERFACE *mvme, uint32_t base, int offset, uint32_t value);
uint32_t mesqdc32_RegisterRead(MVME_INTERFACE *mvme, uint32_t a32base, int offset);
uint32_t mesqdc32_BufferFreeRead(MVME_INTERFACE *mvme, uint32_t a32base);
uint32_t mesqdc32_BufferOccupancy(MVME_INTERFACE *mvme, uint32_t a32base, uint32_t channel);
uint32_t mesqdc32_BufferFree(MVME_INTERFACE *mvme, uint32_t base, int nbuffer);

void     mesqdc32_AcqCtl(MVME_INTERFACE *mvme, uint32_t a32base, uint32_t operation);
void     mesqdc32_ChannelCtl(MVME_INTERFACE *mvme, uint32_t a32base, uint32_t reg, uint32_t mask);
void     mesqdc32_TrgCtl(MVME_INTERFACE *mvme, uint32_t a32base, uint32_t reg, uint32_t mask);

void     mesqdc32_RegisterWrite(MVME_INTERFACE *mvme, uint32_t a32base, int offset, uint32_t value);
void     mesqdc32_Reset(MVME_INTERFACE *mvme, uint32_t a32base);

void     mesqdc32_Status(MVME_INTERFACE *mvme, uint32_t a32base);
int      mesqdc32_Setup(MVME_INTERFACE *mvme, uint32_t a32base, int mode);
void     mesqdc32_info(MVME_INTERFACE *mvme, uint32_t a32base, int *nch, uint32_t *n32w);

uint32_t mesqdc32_MSCF16_Set(MVME_INTERFACE *mvme, uint32_t base, uint32_t modAdd, int subAdd, int data);
uint32_t mesqdc32_MSCF16_Get(MVME_INTERFACE *mvme, uint32_t base, uint32_t modAdd, int subAdd, uint32_t *data);
uint32_t mesqdc32_MSCF16_IDC(MVME_INTERFACE *mvme, uint32_t base, uint32_t modAdd);
uint32_t mesqdc32_MSCF16_RConoff(MVME_INTERFACE *mvme, uint32_t base, uint32_t modAdd, int onoff);

void mesqdc32_ThresholdSet(MVME_INTERFACE *mvme, uint32_t base, uint32_t channel, uint32_t thres);
void mesqdc32_ThresholdGet(MVME_INTERFACE *mvme, uint32_t base, uint32_t channel, uint32_t *thres);

#endif // MESQDC32DRV_INCLUDE_H
