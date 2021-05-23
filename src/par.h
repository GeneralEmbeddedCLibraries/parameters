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
 	bool			persistant;		/**<Parameter persistance flag */
} par_cfg_t;

////////////////////////////////////////////////////////////////////////////////
// Functions Prototypes
////////////////////////////////////////////////////////////////////////////////
par_status_t 	par_init			(void);
const bool		par_is_init			(void);
par_status_t 	par_set				(const par_name_t par, const void * p_val);
par_status_t 	par_get				(const par_name_t par, void * const p_val);
par_status_t 	par_set_to_default	(void);
par_status_t 	par_get_config		(const par_name_t par_name, par_cfg_t * const p_par_cfg);

#endif // _PAR_H_




//////////////////////////////////////////////////////////////
//
//	project:		Sleep monitor REV1.1
//	date:			19.03.2020
//
//	author:			Ziga Miklosic
//
//////////////////////////////////////////////////////////////

#ifndef _PAR_H_
#define _PAR_H_

//////////////////////////////////////////////////////////////
//	INCLUDES
//////////////////////////////////////////////////////////////
#include <stdint.h>
#include <stdbool.h>
#include "project_config.h"


//////////////////////////////////////////////////////////////
//	DEFINITIONS
//////////////////////////////////////////////////////////////

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 		LIST OF USER PARAMETERS
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

typedef enum
{
	ePAR_TEST__u8,
	ePAR_TEST__i8,

	ePAR_TEST__u16,
	ePAR_TEST__i16,

	ePAR_TEST__u32,
	ePAR_TEST__i32,

	ePAR_TEST__f32,
	
	ePAR_NUM_OF
}par_id_t;






// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 		PARAMETERS DEFINITIONS
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


// Parameter status
typedef enum
{
	ePAR_OK = 0,
	ePAR_WAR_LIM_TO_MAX,
	ePAR_WAR_LIM_TO_MIN,
	ePAR_ERROR_ID,
	ePAR_ERROR_ACCESS,
	ePAR_ERROR_NVM,
	ePAR_ERROR_TYPE,
}par_status_t;


// Parameter type
typedef enum
{
	ePAR_TYPE_U8 = 0,
	ePAR_TYPE_U16,
	ePAR_TYPE_U32,
	ePAR_TYPE_I8,
	ePAR_TYPE_I16,
	ePAR_TYPE_I32,
	ePAR_TYPE_F32,
	ePAR_TYPE_B,
	ePAR_TYPE_STR,
	ePAR_TYPE_NUM_OF
}par_type_list_t;


// Parameter R/W access
typedef enum
{
	ePAR_ACCESS_I = 0,			// Parameter read only
	ePAR_ACCESS_IO				// Parameter read/write
}par_io_acess_t;


// Supported data types
typedef union
{
	uint8_t			u8;
	uint16_t		u16;
	uint32_t		u32;
	int8_t			i8;
	int16_t			i16;
	int32_t			i32;
	float32_t		f32;
	bool			b;
	const char* 	str;
}par_type_t;


// Parameter data structure
// NOTE: sizeof( par_t ) = 28 bytes
typedef struct
{
	par_type_t		val;			// Current value of parameter
	par_type_t		min;			// Minimum value of parameter
	par_type_t		max;			// Maximum value of parameter
	par_type_t		init;			// Default value of parameter
	const char*		name;			// Name of variable
	const char*		unit;			// Unit of parameter
	par_id_t		id;				// Variable Id
	par_type_list_t	type;			// Parameter type
	par_io_acess_t 	access;			// Parameter access
	bool			save;			// Enable flag for saving to NVM
}par_t;


//////////////////////////////////////////////////////////////
//	VARIABLES
//////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////
// FUNCTIONS
//////////////////////////////////////////////////////////////
par_status_t par_init		(void);
par_status_t par_set		(par_id_t id, const void * p_val);
par_status_t par_get		(par_id_t id, void * const p_val);
par_status_t par_get_info	(par_id_t id, par_t * const p_par);

#endif	// _PAR_H_
