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

#include <stdlib.h>

////////////////////////////////////////////////////////////////////////////////
// Definitions
////////////////////////////////////////////////////////////////////////////////





////////////////////////////////////////////////////////////////////////////////
// Variables
////////////////////////////////////////////////////////////////////////////////

/**
 * 	Pointer to parameter table
 */
static const par_cfg_t * gp_par_table = NULL;

/**
 * 	Initialization guard
 */
static bool gb_is_init = false;

/**
 * 	Parameter active value that is stored in RAM and its
 * 	address offsets
 */
static uint8_t * 	gpu8_par_value 						= NULL;
static uint32_t 	gu32_par_addr_offset[ ePAR_NUM_OF ] = { 0 };


////////////////////////////////////////////////////////////////////////////////
// Function Prototypes
////////////////////////////////////////////////////////////////////////////////
static par_status_t par_allocate_ram_space	(uint8_t * p_ram_space);
static uint32_t 	par_calc_ram_usage		(void);
static uint8_t 		par_get_data_type_size	(const par_type_list_t par_type);


static par_status_t par_set_check_validy(const par_name_t par_name);

static par_status_t par_set_u8	(const par_name_t par_name, uint8_t u8_val);
static par_status_t par_set_i8	(const par_name_t par_name, int8_t i8_val);


/*static par_status_t par_set_u16	(const par_name_t par_name, uint16_t u16_val);
static par_status_t par_set_i16	(const par_name_t par_name, int16_t i16_val);
static par_status_t par_set_u32	(const par_name_t par_name, uint32_t u32_val);
static par_status_t par_set_i32	(const par_name_t par_name, int32_t i32_val);
static par_status_t par_set_f32	(const par_name_t par_name, float32_t f32_val);*/



////////////////////////////////////////////////////////////////////////////////
// Functions
////////////////////////////////////////////////////////////////////////////////


par_status_t par_init(void)
{
	par_status_t status = ePAR_OK;

	// Get parameter table
	gp_par_table = par_cfg_get_table();

	PAR_ASSERT( NULL != gp_par_table );

	gb_is_init = true;

	// Check if defined correctly
	// TODO: unique par ID ?
	// TODO: min lower than max ?
	// TODO: def within min & max value ?


	// Allocate space in RAM
	par_allocate_ram_space( gpu8_par_value );





	return status;
}



par_status_t par_get_config(const par_name_t par_name, par_cfg_t * const p_par_cfg)
{
	par_status_t status = ePAR_OK;

	// Is init
	PAR_ASSERT( true == gb_is_init );

	// Check inputs
	PAR_ASSERT( NULL != p_par_cfg );
	PAR_ASSERT( par_name < ePAR_NUM_OF );

	*p_par_cfg = gp_par_table[ par_name ];

	return status;
}


par_status_t par_set(const par_name_t par_name, const void * p_val)
{
	par_status_t status;

	switch ( gp_par_table[ par_name ].type )
	{
		case ePAR_TYPE_U8:
			status = par_set_u8( par_name, *(uint8_t*) p_val );
			break;

		case ePAR_TYPE_I8:
			status = par_set_i8( par_name, *(int8_t*) p_val );
			break;

/*		case ePAR_TYPE_U16:
			status = par_set_u16( par_name, *(uint16_t*) p_val );
			break;

		case ePAR_TYPE_I16:
			status = par_set_i16( par_name, *(int16_t*) p_val );
			break;

		case ePAR_TYPE_U32:
			status = par_set_u32( par_name, *(uint32_t*) p_val );
			break;

		case ePAR_TYPE_I32:
			status = par_set_i32( par_name, *(int32_t*) p_val );
			break;

		case ePAR_TYPE_F32:
			status = par_set_f32( par_name, *(float32_t*) p_val );
			break;*/

		default:
			PAR_ASSERT( 0 );
			break;
	}

	return status;
}

par_status_t par_get(const par_name_t par_name, void * const p_val)
{
	par_status_t status = ePAR_OK;

	if ( par_name < ePAR_NUM_OF )
	{
		switch ( gp_par_table[par_name].type )
		{
			case ePAR_TYPE_U8:
				*(uint8_t*) p_val = *(uint8_t*)&gpu8_par_value[ gu32_par_addr_offset[par_name] ];
				break;

			case ePAR_TYPE_I8:
				*(int8_t*) p_val = *(int8_t*)&gpu8_par_value[ gu32_par_addr_offset[par_name] ];
				break;

			case ePAR_TYPE_U16:
				*(uint16_t*) p_val = *(uint16_t*)&gpu8_par_value[ gu32_par_addr_offset[par_name] ];
				break;

			case ePAR_TYPE_I16:
				*(int16_t*) p_val = *(int16_t*)&gpu8_par_value[ gu32_par_addr_offset[par_name] ];
				break;

			case ePAR_TYPE_U32:
				*(uint32_t*) p_val = *(uint32_t*)&gpu8_par_value[ gu32_par_addr_offset[par_name] ];
				break;

			case ePAR_TYPE_I32:
				*(int32_t*) p_val = *(int32_t*)&gpu8_par_value[ gu32_par_addr_offset[par_name] ];
				break;

			case ePAR_TYPE_F32:
				*(float32_t*) p_val = *(float32_t*)&gpu8_par_value[ gu32_par_addr_offset[par_name] ];
				break;

			default:
				PAR_ASSERT( 0 );
				break;
		}
	}
	else
	{
		status = ePAR_ERROR;
	}

	return status;
}



////////////////////////////////////////////////////////////////////////////////
/**
* @} <!-- END GROUP -->
*/
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
/**
*@addtogroup KERNEL_FUNCTIONS
* @{ <!-- BEGIN GROUP -->
*
* 	Kernel functions of device parameters
*/
////////////////////////////////////////////////////////////////////////////////


static par_status_t par_allocate_ram_space(uint8_t * p_ram_space)
{
	par_status_t 	status 		= ePAR_OK;
	uint32_t		ram_size	= 0UL;

	// Calculate total size of RAM
	ram_size = par_calc_ram_usage();

	// Allocate space in RAM
	p_ram_space = malloc( ram_size );
	PAR_ASSERT( NULL != p_ram_space );

	return status;
}


static uint32_t par_calc_ram_usage(void)
{
	par_cfg_t 		par_cfg 	= { 0 };
	uint32_t 		i 			= 0UL;
	uint32_t		total_size	= 0UL;

	// For every parameter
	for ( i = 0; i < ePAR_NUM_OF; i++ )
	{
		// Get parameter configs
		par_get_config( i, &par_cfg );

        // Align addresses
        if	(	( par_cfg.type == ePAR_TYPE_U16 )
        	|| 	( par_cfg.type == ePAR_TYPE_I16 ))
        {
            while(( total_size % 2 ) != 0 )
            {
            	total_size++;
            }
        }
        else if (	( par_cfg.type == ePAR_TYPE_U32 )
        		|| 	( par_cfg.type == ePAR_TYPE_I32 )
				|| 	( par_cfg.type == ePAR_TYPE_F32 ))
        {
            while(( total_size % 4 ) != 0 )
            {
            	total_size++;
            }
        }
        else
        {
        	// No actions...
        }

        // Store par RAM address offset
        gu32_par_addr_offset[i] = total_size;

        // Accumulate total RAM space
        total_size += par_get_data_type_size( par_cfg.type );
	}

	return total_size;
}


static uint8_t par_get_data_type_size(const par_type_list_t par_type)
{
	uint8_t type_size = 0U;

	switch ( par_type )
	{
		case ePAR_TYPE_U8:
			type_size = sizeof( uint8_t );
			break;

		case ePAR_TYPE_I8:
			type_size = sizeof( int8_t );
			break;

		case ePAR_TYPE_U16:
			type_size = sizeof( uint16_t );
			break;

		case ePAR_TYPE_I16:
			type_size = sizeof( int16_t );
			break;

		case ePAR_TYPE_U32:
			type_size = sizeof( uint32_t );
			break;

		case ePAR_TYPE_I32:
			type_size = sizeof( int32_t );
			break;

		case ePAR_TYPE_F32:
			type_size = sizeof( float32_t );
			break;

		default:
			// No actions...
			break;
	}

	return type_size;
}



static par_status_t par_set_u8(const par_name_t par_name, uint8_t u8_val)
{
	par_status_t status = par_set_check_validy( par_name );

	if ( ePAR_OK == status )
	{
		if ( u8_val > ( gp_par_table[ par_name ].max.u8 ))
		{
			*(uint8_t*)&gpu8_par_value[ gu32_par_addr_offset[par_name] ] = gp_par_table[ par_name ].max.u8;
			status = ePAR_WAR_LIM_TO_MAX;
		}
		else if ( u8_val < ( gp_par_table[ par_name ].min.u8 ))
		{
			*(uint8_t*)&gpu8_par_value[ gu32_par_addr_offset[par_name] ] = gp_par_table[ par_name ].min.u8;
			status = ePAR_WAR_LIM_TO_MIN;
		}
		else
		{
			*(uint8_t*)&gpu8_par_value[ gu32_par_addr_offset[par_name] ] = (uint8_t) ( u8_val );
			status = ePAR_OK;
		}
	}

	return status;
}


static par_status_t par_set_i8(const par_name_t par_name, int8_t i8_val)
{
	par_status_t status = par_set_check_validy( par_name );

	if ( ePAR_OK == status )
	{
		if ( i8_val > ( gp_par_table[ par_name ].max.i8 ))
		{
			*(int8_t*)&gpu8_par_value[ gu32_par_addr_offset[par_name] ] = gp_par_table[ par_name ].max.i8;
			status = ePAR_WAR_LIM_TO_MAX;
		}
		else if ( i8_val < ( gp_par_table[ par_name ].min.i8 ))
		{
			*(int8_t*)&gpu8_par_value[ gu32_par_addr_offset[par_name] ] = gp_par_table[ par_name ].min.i8;
			status = ePAR_WAR_LIM_TO_MIN;
		}
		else
		{
			*(int8_t*)&gpu8_par_value[ gu32_par_addr_offset[par_name] ] = (int8_t) ( i8_val );
			status = ePAR_OK;
		}
	}

	return status;
}

/*
static par_status_t par_set_u16(const par_name_t par_name, uint16_t u16_val)
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


static par_status_t par_set_i16(const par_name_t par_name, int16_t i16_val)
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


static par_status_t par_set_u32(const par_name_t par_name, uint32_t u32_val)
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


static par_status_t par_set_i32(const par_name_t par_name, int32_t i32_val)
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


static par_status_t par_set_f32(const par_name_t par_name, float32_t f32_val)
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
}*/


//////////////////////////////////////////////////////////////
//
//			par_set_check_validy
//
//	param: 		id - parameter id
//	return:		status
//	brief:		Check if setting parameter is legal
//
//////////////////////////////////////////////////////////////
static par_status_t par_set_check_validy(const par_name_t par_name)
{
	par_status_t status = ePAR_OK;

	// Check if par listed
	if ( par_name < ePAR_NUM_OF )
	{
		// Check access
		if ( ePAR_ACCESS_RW == ( gp_par_table[ par_name ].access ))
		{
			status = ePAR_OK;
		}
		else
		{
			status = ePAR_ERROR;
		}
	}
	else
	{
		status = ePAR_ERROR;
	}

	return status;
}






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

