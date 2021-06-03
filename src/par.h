////////////////////////////////////////////////////////////////////////////////
/**
*@file      par.h
*@brief    	Device parameters API functions
*@author    Ziga Miklosic
*@date      22.05.2021
*@version	V1.0.0
*/
////////////////////////////////////////////////////////////////////////////////
/**
*@addtogroup PARAMETERS_API
* @{ <!-- BEGIN GROUP -->
*/
////////////////////////////////////////////////////////////////////////////////

#ifndef _PAR_H_
#define _PAR_H_

////////////////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////////////////
#include <stdint.h>
#include <stdbool.h>

#include "project_config.h"
#include "../../par_cfg.h"

////////////////////////////////////////////////////////////////////////////////
// Definitions
////////////////////////////////////////////////////////////////////////////////

/**
 *   Parameter status
 */
typedef enum
{
	ePAR_OK 				= 0x00,		/**<Normal operation */
	ePAR_WAR_LIM_TO_MAX		= 0x01,		/**<Parameter limited to max value warning */
	ePAR_WAR_LIM_TO_MIN		= 0x02,		/**<Parameter limited to min value warning */
	ePAR_ERROR				= 0x04,		/**<General parameter error */
	ePAR_ERROR_NVM			= 0x08,		/**<Parameter storage to NMV error */
}par_status_t;

/**
 * 	Parameters type enumeration
 */
typedef enum
{
	ePAR_TYPE_U8 = 0,	/**<Unsigned 8-bit value */
	ePAR_TYPE_U16,		/**<Unsigned 16-bit value */
	ePAR_TYPE_U32,		/**<Unsigned 32-bit value */
	ePAR_TYPE_I8,		/**<Signed 8-bit value */
	ePAR_TYPE_I16,		/**<Signed 16-bit value */
	ePAR_TYPE_I32,		/**<Signed 32-bit value */
	ePAR_TYPE_F32,		/**<32-bit floating value */
	ePAR_TYPE_NUM_OF
}par_type_list_t;

/**
 * 	Parameter R/W access
 */
typedef enum
{
	ePAR_ACCESS_RO = 0,			/**<Parameter read only */
	ePAR_ACCESS_RW				/**<Parameter read/write */
}par_io_acess_t;

/**
 * 	Supported data types
 */
typedef union
{
	uint8_t			u8;			/**<Unsigned 8-bit value */
	uint16_t		u16;		/**<Unsigned 16-bit value */
	uint32_t		u32;		/**<Unsigned 32-bit value */
	int8_t			i8;			/**<Signed 8-bit value */
	int16_t			i16;		/**<Signed 16-bit value */
	int32_t			i32;		/**<Signed 32-bit value */
	float32_t		f32;		/**<32-bit floating value */
} par_type_t;

/**
 * 	Parameter data settings
 *
 * @note	Single parameter object has size of 28 bytes on
 * 			arm-gcc compiler.
 */
typedef struct
{
	uint32_t			id;				/**<Variable ID */
	char*				name;			/**<Name of variable */
 	par_type_t			min;			/**<Minimum value of parameter */
	par_type_t			max;			/**<Maximum value of parameter */
	par_type_t			def;			/**<Default value of parameter */
	char*				unit;			/**<Unit of parameter */
	par_type_list_t		type;			/**<Parameter type */
	par_io_acess_t 		access;			/**<Parameter access from external device point-of-view */
 	bool				persistant;		/**<Parameter persistence flag */
} par_cfg_t;

////////////////////////////////////////////////////////////////////////////////
// Functions Prototypes
////////////////////////////////////////////////////////////////////////////////
par_status_t 	par_init				(void);
const bool		par_is_init				(void);

par_status_t 	par_set					(const par_num_t par_num, const void * p_val);
par_status_t 	par_get					(const par_num_t par_num, void * const p_val);
void 			par_set_to_default		(const par_num_t par_num);
void		 	par_set_all_to_default	(void);

par_status_t 	par_get_config			(const par_num_t par_num, par_cfg_t * const p_par_cfg);
par_type_list_t	par_get_data_type		(const par_num_t par_num);
void		 	par_get_name			(const par_num_t par_num, uint8_t * const p_name);
par_io_acess_t	par_get_access			(const par_num_t par_num);

#if ( 1 == PAR_CFG_NVM_EN )
	par_status_t	par_store_all_to_nvm	(void);
	par_status_t	par_store_to_nvm		(const par_num_t par_num);
#endif

#endif // _PAR_H_
