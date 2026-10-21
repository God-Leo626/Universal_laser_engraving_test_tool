/**********************************************************************************
*Copyright (c) 2019, Sonoff.ltd
*All rights reserved.
*
* File name			:	
* Summary Content		:
* Document			:	None
* Notes				:

* Version				:	V1.0
*
* Modification date 	Ver		       	Name		 		Modify 
*  ----------------------------------------------------------------------------
* 20190703          	V1.0 				Alex		       	Establish
************************************************************************************/

#ifndef __BL_EFLASH_BOOTLOAD_40M_H__
#define __BL_EFLASH_BOOTLOAD_40M_H__

#include "Header.h"


//烧写 Segment DATA 数据包个数
#define BL_BURN_SEGMENT_DATA_PKT_NUM				15

extern const UCHAR8 bl_bootload_header_table[180];

extern const UCHAR8 bl_segment_header_table[20];

extern const UCHAR8 bl_segment_data_table[BL_BURN_SEGMENT_DATA_PKT_NUM][4084];





#endif




