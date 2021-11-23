// Copyright (c) 2021 Ziga Miklosic
// All Rights Reserved
// This software is under MIT licence (https://opensource.org/licenses/MIT)
////////////////////////////////////////////////////////////////////////////////
/**
*@file      par_nvm.h
*@brief    	Parameter storage to non-volatile memory
*@author    Ziga Miklosic
*@date      22.05.2021
*@version	V1.2.0
*/
////////////////////////////////////////////////////////////////////////////////
/**
*@addtogroup PAR_NVM
* @{ <!-- BEGIN GROUP -->
*/
////////////////////////////////////////////////////////////////////////////////

#ifndef _PAR_NVM_H_
#define _PAR_NVM_H_

////////////////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////////////////
#include <stdint.h>
#include "par.h"

#if ( 1 == PAR_CFG_NVM_EN )

	////////////////////////////////////////////////////////////////////////////////
	// Functions Prototypes
	////////////////////////////////////////////////////////////////////////////////
	par_status_t par_nvm_init		(void);
	par_status_t par_nvm_write		(const par_num_t par_num);
	par_status_t par_nvm_write_all	(void);

	#if ( PAR_CFG_DEBUG_EN )
		void par_nvm_print_nvm_lut(void);
	#endif

#endif // 1 == PAR_CFG_NVM_EN

////////////////////////////////////////////////////////////////////////////////
/**
* @} <!-- END GROUP -->
*/
////////////////////////////////////////////////////////////////////////////////

#endif // _PAR_NVM_H_
