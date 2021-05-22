////////////////////////////////////////////////////////////////////////////////
/**
*@file      par.c
*@brief     Device parameters API functions
*@author    Ziga Miklosic
*@date      22.05.2021
*@version	V1.0.0
*/
////////////////////////////////////////////////////////////////////////////////
/**
*@addtogroup PARARAMETERS_API
* @{ <!-- BEGIN GROUP -->
*
* 	Parameter kernel module
*/
////////////////////////////////////////////////////////////////////////////////




/**
 * 	TODO LIST:
 *
 * 		- check for unique parameter ID (apply ASSERT if not)
 * 		- forward declaration of parameter config table
 *
 */

////////////////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////////////////
#include "par.h"
#include "../../par_cfg.h"

////////////////////////////////////////////////////////////////////////////////
// Definitions
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
// Variables
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
// Function Prototypes
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
// Functions
////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////
/**
* @} <!-- END GROUP -->
*/
////////////////////////////////////////////////////////////////////////////////




#if 0

//////////////////////////////////////////////////////////////
//
//	project:		Sleep monitor REV1.1
//	date:			19.03.2020
//
//	author:			Ziga Miklosic
//
//////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////
//	INCLUDES
//////////////////////////////////////////////////////////////
#include "par.h"
//#include "par_nvm.h"


//////////////////////////////////////////////////////////////
//	DEFINITIONS
//////////////////////////////////////////////////////////////


// Enable/disable NVM usage
#define PAR_NVM_USE_EN			( 1 )

// TODO: Add OS support
// Mutex enable
#define PAR_MUTEX_EN			( 1 )


//////////////////////////////////////////////////////////////
//	VARIABLES
//////////////////////////////////////////////////////////////

// Parameter table
static par_t par[ePAR_NUM_OF] =
{
	{ 	.id 		= 0,
		.name 		= "TEST U8",
		.type 		= ePAR_TYPE_U8,
		.access		= ePAR_ACCESS_IO,
		.save		= true,

		.min.u8 	= 0,
		.max.u8 	= 200,
		.init.u8	= 0,

		.unit 		= "u8",
	},

	{ 	.id 		= 1,
		.name 		= "TEST I8",
		.type 		= ePAR_TYPE_I8,
		.access		= ePAR_ACCESS_IO,
		.save		= true,

		.min.i8 	= -50,
		.max.i8 	= 50,
		.init.i8	= 0,

		.unit 		= "i8",
	},

	{ 	.id 		= 2,
		.name 		= "TEST U16",
		.type 		= ePAR_TYPE_U16,
		.access		= ePAR_ACCESS_IO,
		.save		= true,

		.min.u16 	= 0,
		.max.u16 	= 45000,
		.init.u16	= 100,

		.unit		= "u16"
	},

	{ 	.id 		= 3,
		.name 		= "TEST I16",
		.type 		= ePAR_TYPE_I16,
		.access		= ePAR_ACCESS_IO,
		.save 		= true,

		.min.i16 	= -200,
		.max.i16 	= 200,
		.init.i16	= -1,

		.unit 		= "i16",
	},

	{ 	.id 		= 4,
		.name 		= "TEST U32",
		.type 		= ePAR_TYPE_U32,
		.access		= ePAR_ACCESS_IO,
		.save 		= true,

		.min.u32 	= 0,
		.max.u32 	= 2343,
		.init.u32	= 111,

		.unit 		= "u32",
	},

	{ 	.id 		= 5,
		.name 		= "TEST I32",
		.type 		= ePAR_TYPE_I32,
		.access		= ePAR_ACCESS_IO,
		.save 		= true,

		.min.i32 	= -200,
		.max.i32 	= 200,
		.init.i32	= 123,

		.unit 		= "i32",
	},

	{ 	.id 		= 6,
		.name 		= "TEST F32",
		.type 		= ePAR_TYPE_F32,
		.access		= ePAR_ACCESS_IO,
		.save		= true,

		.min.f32 	= -200.0f,
		.max.f32 	= 200.0f,
		.init.f32	= -1.234f,

		.unit 		= "f32",
	},
};




//////////////////////////////////////////////////////////////
// FUNCTIONS
//////////////////////////////////////////////////////////////

static par_status_t par_set_check_validy(par_id_t id);

static par_status_t par_set_u8	(par_id_t id, uint8_t u8_val);
static par_status_t par_set_i8	(par_id_t id, int8_t i8_val);
static par_status_t par_set_u16	(par_id_t id, uint16_t u16_val);
static par_status_t par_set_i16	(par_id_t id, int16_t i16_val);
static par_status_t par_set_u32	(par_id_t id, uint32_t u32_val);
static par_status_t par_set_i32	(par_id_t id, int32_t i32_val);
static par_status_t par_set_f32	(par_id_t id, float32_t f32_val);



//////////////////////////////////////////////////////////////
//
//			par_init
//
//	param: 		none
//	return:		status - status of operation
//	brief:		Initialize parameters
//
//////////////////////////////////////////////////////////////
par_status_t par_init(void)
{
	par_status_t status = ePAR_OK;
//	uint8_t i;

/*	// Init nvm
	if ( par_nvm_init() != ePAR_NVM_OK )
	{
		status = ePAR_ERROR_NVM;
		goto exit;
	}

	// Initialize all parameters from NVM
	for ( i = 0; i < ePAR_NUM_OF; i++ )
	{
#if ( PAR_NVM_USE_EN )
		if ( par[i].save )
		{
			if ( par_nvm_read(i, &( par[i].val )) != ePAR_NVM_OK )
			{
				par[i].val = par[i].init;
			}
		}
		else
		{
			par[i].val = par[i].init;
		}
#else
		par[i].val = par[i].init;
#endif
	}

exit:
*/
	return status;
}


//////////////////////////////////////////////////////////////
//
//			par_set
//
//	param: 		id - parameter id
//				p_val - pointer to constant value
//	return:		status
//	brief:		Set parameter value
//
//////////////////////////////////////////////////////////////
par_status_t par_set(par_id_t id, const void * p_val)
{
	par_status_t status;

	switch ( par[id].type )
	{
		case ePAR_TYPE_U8:
			status = par_set_u8(id, *(uint8_t*) p_val);
			break;

		case ePAR_TYPE_I8:
			status = par_set_i8(id, *(int8_t*) p_val);
			break;

		case ePAR_TYPE_U16:
			status = par_set_u16(id, *(uint16_t*) p_val);
			break;

		case ePAR_TYPE_I16:
			status = par_set_i16(id, *(int16_t*) p_val);
			break;

		case ePAR_TYPE_U32:
			status = par_set_u32(id, *(uint32_t*) p_val);
			break;

		case ePAR_TYPE_I32:
			status = par_set_i32(id, *(int32_t*) p_val);
			break;

		case ePAR_TYPE_F32:
			status = par_set_f32(id, *(float32_t*) p_val);
			break;

		default:
			status = ePAR_ERROR_TYPE;
			break;
	}


	/*
#if ( PAR_NVM_USE_EN )

	// Store to NVM
	if (( par[id].save ) && ( status < ePAR_ERROR_ID ))
	{
		if ( par_nvm_write(id, &( par[id].val )) != ePAR_NVM_OK )
		{
			status = ePAR_ERROR_NVM;
		}
	}

#endif // PAR_NVM_USE_EN

	*/

	return status;
}


//////////////////////////////////////////////////////////////
//
//			par_set
//
//	param: 		id - parameter id
//				p_val - constant pointer to value
//	return:		status
//	brief:		Set parameter value
//
//////////////////////////////////////////////////////////////
par_status_t par_get(par_id_t id, void * const p_val)
{
	par_status_t status = ePAR_OK;

	if ( id < ePAR_NUM_OF )
	{
		switch ( par[id].type )
		{
			case ePAR_TYPE_U8:
				*(uint8_t*) p_val = par[id].val.u8;
				break;

			case ePAR_TYPE_I8:
				*(int8_t*) p_val = par[id].val.i8;
				break;

			case ePAR_TYPE_U16:
				*(uint16_t*) p_val = par[id].val.u16;
				break;

			case ePAR_TYPE_I16:
				*(int16_t*) p_val = par[id].val.i16;
				break;

			case ePAR_TYPE_U32:
				*(uint32_t*) p_val = par[id].val.u32;
				break;

			case ePAR_TYPE_I32:
				*(int32_t*) p_val = par[id].val.i32;
				break;

			case ePAR_TYPE_F32:
				*(float32_t*) p_val = par[id].val.f32;
				break;

			default:
				status = ePAR_ERROR_TYPE;
				break;
		}
	}
	else
	{
		status = ePAR_ERROR_ID;
	}

	return status;
}



//////////////////////////////////////////////////////////////
//
//			par_set_u8
//
//	param: 		id - parameter id
//				u8_val - 8bit unsigned value
//	return:		status
//	brief:		Set 8bit unsigned parameter value
//
//////////////////////////////////////////////////////////////
static par_status_t par_set_u8(par_id_t id, uint8_t u8_val)
{
	par_status_t status = par_set_check_validy(id);

	if ( ePAR_OK == status )
	{
		if ( u8_val > ( par[id].max.u8 ))
		{
			par[id].val.u8 = par[id].max.u8;
			status = ePAR_WAR_LIM_TO_MAX;
		}
		else if ( u8_val < ( par[id].min.u8 ))
		{
			par[id].val.u8 = par[id].min.u8;
			status = ePAR_WAR_LIM_TO_MIN;
		}
		else
		{
			par[id].val.u8 = (uint8_t) ( u8_val );
			status = ePAR_OK;
		}
	}

	return status;
}


//////////////////////////////////////////////////////////////
//
//			par_set_i8
//
//	param: 		id - parameter id
//				i8_val - 8bit signed value
//	return:		status
//	brief:		Set 8bit signed parameter value
//
//////////////////////////////////////////////////////////////
static par_status_t par_set_i8(par_id_t id, int8_t i8_val)
{
	par_status_t status = par_set_check_validy(id);

	if ( ePAR_OK == status )
	{
		if ( i8_val > ( par[id].max.i8 ))
		{
			par[id].val.i8 = par[id].max.i8;
			status = ePAR_WAR_LIM_TO_MAX;
		}
		else if ( i8_val < ( par[id].min.i8 ))
		{
			par[id].val.i8 = par[id].min.i8;
			status = ePAR_WAR_LIM_TO_MIN;
		}
		else
		{
			par[id].val.i8 = (int8_t) ( i8_val );
			status = ePAR_OK;
		}
	}

	return status;
}


//////////////////////////////////////////////////////////////
//
//			par_set_u16
//
//	param: 		id - parameter id
//				u16_val - 16bit unsigned value
//	return:		status
//	brief:		Set 16bit unsigned parameter value
//
//////////////////////////////////////////////////////////////
static par_status_t par_set_u16(par_id_t id, uint16_t u16_val)
{
	par_status_t status = par_set_check_validy(id);

	if ( ePAR_OK == status )
	{
		if ( u16_val > ( par[id].max.u16 ))
		{
			par[id].val.u16 = par[id].max.u16;
			status = ePAR_WAR_LIM_TO_MAX;
		}
		else if ( u16_val < ( par[id].min.u16 ))
		{
			par[id].val.u16 = par[id].min.u16;
			status = ePAR_WAR_LIM_TO_MIN;
		}
		else
		{
			par[id].val.u16 = (uint16_t) ( u16_val );
			status = ePAR_OK;
		}
	}

	return status;
}


//////////////////////////////////////////////////////////////
//
//			par_set_i16
//
//	param: 		id - parameter id
//				i16_val - 16bit signed value
//	return:		status
//	brief:		Set 16bit signed parameter value
//
//////////////////////////////////////////////////////////////
static par_status_t par_set_i16(par_id_t id, int16_t i16_val)
{
	par_status_t status = par_set_check_validy(id);

	if ( ePAR_OK == status )
	{
		if ( i16_val > ( par[id].max.i16 ))
		{
			par[id].val.i16 = par[id].max.i16;
			status = ePAR_WAR_LIM_TO_MAX;
		}
		else if ( i16_val < ( par[id].min.i16 ))
		{
			par[id].val.i16 = par[id].min.i16;
			status = ePAR_WAR_LIM_TO_MIN;
		}
		else
		{
			par[id].val.i16 = (int16_t) ( i16_val );
			status = ePAR_OK;
		}
	}

	return status;
}


//////////////////////////////////////////////////////////////
//
//			par_set_u32
//
//	param: 		id - parameter id
//				u32_val - 32bit unsigned value
//	return:		status
//	brief:		Set 32bit unsigned parameter value
//
//////////////////////////////////////////////////////////////
static par_status_t par_set_u32(par_id_t id, uint32_t u32_val)
{
	par_status_t status = par_set_check_validy(id);

	if ( ePAR_OK == status )
	{
		if ( u32_val > ( par[id].max.u32 ))
		{
			par[id].val.u32 = par[id].max.u32;
			status = ePAR_WAR_LIM_TO_MAX;
		}
		else if ( u32_val < ( par[id].min.u32 ))
		{
			par[id].val.u32 = par[id].min.u32;
			status = ePAR_WAR_LIM_TO_MIN;
		}
		else
		{
			par[id].val.u32 = (uint32_t) ( u32_val );
			status = ePAR_OK;
		}
	}

	return status;
}


//////////////////////////////////////////////////////////////
//
//			par_set_i32
//
//	param: 		id - parameter id
//				i32_val - 32bit signed value
//	return:		status
//	brief:		Set 32bit signed parameter value
//
//////////////////////////////////////////////////////////////
static par_status_t par_set_i32(par_id_t id, int32_t i32_val)
{
	par_status_t status = par_set_check_validy(id);

	if ( ePAR_OK == status )
	{
		if ( i32_val > ( par[id].max.i32 ))
		{
			par[id].val.i32 = par[id].max.i32;
			status = ePAR_WAR_LIM_TO_MAX;
		}
		else if ( i32_val < ( par[id].min.i32 ))
		{
			par[id].val.i32 = par[id].min.i32;
			status = ePAR_WAR_LIM_TO_MIN;
		}
		else
		{
			par[id].val.i32 = (int32_t) ( i32_val );
			status = ePAR_OK;
		}
	}

	return status;
}


//////////////////////////////////////////////////////////////
//
//			par_set_f32
//
//	param: 		id - parameter id
//				f32_val - float value
//	return:		status
//	brief:		Set float parameter value
//
//////////////////////////////////////////////////////////////
static par_status_t par_set_f32(par_id_t id, float32_t f32_val)
{
	par_status_t status = par_set_check_validy(id);

	if ( ePAR_OK == status )
	{
		if ( f32_val > ( par[id].max.f32 ))
		{
			par[id].val.f32 = par[id].max.f32;
			status = ePAR_WAR_LIM_TO_MAX;
		}
		else if ( f32_val < ( par[id].min.f32 ))
		{
			par[id].val.f32 = par[id].min.f32;
			status = ePAR_WAR_LIM_TO_MIN;
		}
		else
		{
			par[id].val.f32 = (float32_t) ( f32_val );
			status = ePAR_OK;
		}
	}

	return status;
}


//////////////////////////////////////////////////////////////
//
//			par_set_check_validy
//
//	param: 		id - parameter id
//	return:		status
//	brief:		Check if setting parameter is legal
//
//////////////////////////////////////////////////////////////
static par_status_t par_set_check_validy(par_id_t id)
{
	par_status_t status;

	// Check if par listed
	if ( id < ePAR_NUM_OF )
	{
		// Check access
		if ( ePAR_ACCESS_IO == ( par[id].access ))
		{
			status = ePAR_OK;
		}
		else
		{
			status = ePAR_ERROR_ACCESS;
		}
	}
	else
	{
		status = ePAR_ERROR_ID;
	}

	return status;
}


//////////////////////////////////////////////////////////////
//
//			par_get_info
//
//	param: 		id - parameter id
//				p_par - pointer to parameter
//	return:		status
//	brief:		Get parameter informations.
//
//////////////////////////////////////////////////////////////
par_status_t par_get_info(par_id_t id, par_t * const p_par)
{
	par_status_t status = ePAR_OK;

	if ( id < ePAR_NUM_OF )
	{
		*p_par = par[id];
	}
	else
	{
		status = ePAR_ERROR_ID;
	}

	return status;
}


//////////////////////////////////////////////////////////////
// END OF FILE
//////////////////////////////////////////////////////////////

#endif

