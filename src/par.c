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


////////////////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////////////////
#include "par.h"
#include "../../par_cfg.h"
#include "../../par_if.h"

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
static par_status_t	par_check_table_validy	(const par_cfg_t * const p_par_cfg);
static par_status_t par_set_u8				(const par_name_t par_name, const uint8_t u8_val);
static par_status_t par_set_i8				(const par_name_t par_name, const int8_t i8_val);
static par_status_t par_set_u16				(const par_name_t par_name, const uint16_t u16_val);
static par_status_t par_set_i16				(const par_name_t par_name, const int16_t i16_val);
static par_status_t par_set_u32				(const par_name_t par_name, const uint32_t u32_val);
static par_status_t par_set_i32				(const par_name_t par_name, const int32_t i32_val);
static par_status_t par_set_f32				(const par_name_t par_name, const float32_t f32_val);

////////////////////////////////////////////////////////////////////////////////
// Functions
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
/**
*		Parameters initialization
*
*	At init parameter table is being check for correct definition, allocation
*	in RAM space for parameters live values and additionaly interface to
*	platform is being done.
*
* @return		status  - Status of initialization
*/
////////////////////////////////////////////////////////////////////////////////
par_status_t par_init(void)
{
	par_status_t status = ePAR_OK;

	// Get parameter table
	gp_par_table = par_cfg_get_table();
	PAR_ASSERT( NULL != gp_par_table );

	// Check if par table is defined correctly
	status |= par_check_table_validy( gp_par_table );

	// Allocate space in RAM
	status |= par_allocate_ram_space( gpu8_par_value );

	// Initialize parameter interface
	status |= par_if_init();

	// Init succeed
	if ( ePAR_OK == status )
	{
		gb_is_init = true;
	}

	return status;
}

////////////////////////////////////////////////////////////////////////////////
/**
*		Get parameter configurations
*
* @param[in]	par_name	- Name of parameter
* @param[in]	p_par_cfg	- Pointer to parameter configurations
* @return		status 		- Status of operation
*/
////////////////////////////////////////////////////////////////////////////////
par_status_t par_get_config(const par_name_t par_name, par_cfg_t * const p_par_cfg)
{
	par_status_t status = ePAR_OK;

	// Check inputs
	PAR_ASSERT( NULL != p_par_cfg );
	PAR_ASSERT( par_name < ePAR_NUM_OF );

	*p_par_cfg = gp_par_table[ par_name ];

	return status;
}

////////////////////////////////////////////////////////////////////////////////
/**
*		Set parameter value
*
* @note 	Mandatory to cast input argument to appropriate type. E.g.:
*
* @code
* 			float32_t my_val = 1.234f;
* 			par_set( ePAR_MY_VAR, (float32_t*) &my_val );
* @endcode
*
* @param[in]	par_name	- Name of parameter
* @param[in]	p_value		- Pointer to value
* @return		status 		- Status of operation
*/
////////////////////////////////////////////////////////////////////////////////
par_status_t par_set(const par_name_t par_name, const void * p_val)
{
	par_status_t status = ePAR_OK;

	// Is init
	PAR_ASSERT( true == gb_is_init );

	// Check input
	PAR_ASSERT( par_name < ePAR_NUM_OF );
	PAR_ASSERT( ePAR_ACCESS_RW == ( gp_par_table[ par_name ].access ));

	#if ( 1 == PAR_CFG_MUTEX_EN )
		if ( ePAR_OK == par_if_aquire_mutex())
		{
	#endif
			switch ( gp_par_table[ par_name ].type )
			{
				case ePAR_TYPE_U8:
					status = par_set_u8( par_name, *(uint8_t*) p_val );
					break;

				case ePAR_TYPE_I8:
					status = par_set_i8( par_name, *(int8_t*) p_val );
					break;

				case ePAR_TYPE_U16:
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
					break;

				default:
					PAR_ASSERT( 0 );
					break;
			}

	#if ( 1 == PAR_CFG_MUTEX_EN )
			par_if_release_mutex();
		}

		// Mutex not acquire
		else
		{
			status = ePAR_ERROR;
		}
	#endif

	return status;
}

////////////////////////////////////////////////////////////////////////////////
/**
*		Get parameter value
*
* @note 	Mandatory to cast input argument to appropriate type. E.g.:
*
* @code
* 			float32_t my_val = 0.0f;
* 			par_get( ePAR_MY_VAR, (float32_t*) &my_val );
* @endcode
*
* @param[in]	par_name	- Name of parameter
* @param[out]	p_val		- Parameter value
* @return		status 		- Status of operation
*/
////////////////////////////////////////////////////////////////////////////////
par_status_t par_get(const par_name_t par_name, void * const p_val)
{
	par_status_t status = ePAR_OK;

	// Is init
	PAR_ASSERT( true == gb_is_init );

	// Check input
	PAR_ASSERT( par_name < ePAR_NUM_OF );

	#if ( 1 == PAR_CFG_MUTEX_EN )
		if ( ePAR_OK == par_if_aquire_mutex())
		{
	#endif
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

	#if ( 1 == PAR_CFG_MUTEX_EN )
			par_if_release_mutex();
		}

		// Mutex not acquire
		else
		{
			status = ePAR_ERROR;
		}
	#endif

	return status;
}

////////////////////////////////////////////////////////////////////////////////
/**
*		Get parameter data type
*
* @param[in]	par_name	- Name of parameter
* @return		type 		- Data type of parameter
*/
////////////////////////////////////////////////////////////////////////////////
par_type_list_t	par_get_data_type(const par_name_t par_name)
{
	par_type_list_t type = ePAR_TYPE_U8;

	// Is init
	PAR_ASSERT( true == gb_is_init );

	// Check input
	PAR_ASSERT( par_name < ePAR_NUM_OF );

	// Get type
	type = gp_par_table[ par_name ].type;

	return type;
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


static par_status_t	par_check_table_validy(const par_cfg_t * const p_par_cfg)
{
	par_status_t status = ePAR_OK;

	for ( uint32_t i = 0; i < ePAR_NUM_OF; i++ )
	{
		for ( uint32_t j = 0; j < ePAR_NUM_OF; j++ )
		{
			if ( i != j )
			{
				// Check for two identical IDs
				if ( p_par_cfg[i].id == p_par_cfg[j].id )
				{
					status = ePAR_ERROR;
					PAR_DBG_PRINT( "Parameter table error: Duplicate ID!" );
					PAR_ASSERT( 0 );
					break;
				}
			}
		}

		if ( ePAR_OK != status )
		{
			break;
		}
	}

	return status;
}


static par_status_t par_set_u8(const par_name_t par_name, const uint8_t u8_val)
{
	par_status_t status = ePAR_OK;

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


static par_status_t par_set_i8(const par_name_t par_name, const int8_t i8_val)
{
	par_status_t status = ePAR_OK;

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


static par_status_t par_set_u16(const par_name_t par_name, const uint16_t u16_val)
{
	par_status_t status = ePAR_OK;

	if ( ePAR_OK == status )
	{
		if ( u16_val > ( gp_par_table[ par_name ].max.u16 ))
		{
			*(uint16_t*)&gpu8_par_value[ gu32_par_addr_offset[par_name] ] = gp_par_table[ par_name ].max.u16;
			status = ePAR_WAR_LIM_TO_MAX;
		}
		else if ( u16_val < ( gp_par_table[ par_name ].min.u16 ))
		{
			*(uint16_t*)&gpu8_par_value[ gu32_par_addr_offset[par_name] ] = gp_par_table[ par_name ].min.u16;
			status = ePAR_WAR_LIM_TO_MIN;
		}
		else
		{
			*(uint16_t*)&gpu8_par_value[ gu32_par_addr_offset[par_name] ] = (uint16_t) ( u16_val );
			status = ePAR_OK;
		}
	}

	return status;
}


static par_status_t par_set_i16(const par_name_t par_name, const int16_t i16_val)
{
	par_status_t status = ePAR_OK;

	if ( ePAR_OK == status )
	{
		if ( i16_val > ( gp_par_table[ par_name ].max.i16 ))
		{
			*(int16_t*)&gpu8_par_value[ gu32_par_addr_offset[par_name] ] = gp_par_table[ par_name ].max.i16;
			status = ePAR_WAR_LIM_TO_MAX;
		}
		else if ( i16_val < ( gp_par_table[ par_name ].min.i16 ))
		{
			*(int16_t*)&gpu8_par_value[ gu32_par_addr_offset[par_name] ] = gp_par_table[ par_name ].min.i16;
			status = ePAR_WAR_LIM_TO_MIN;
		}
		else
		{
			*(int16_t*)&gpu8_par_value[ gu32_par_addr_offset[par_name] ] = (int16_t) ( i16_val );
			status = ePAR_OK;
		}
	}

	return status;
}


static par_status_t par_set_u32(const par_name_t par_name, const uint32_t u32_val)
{
	par_status_t status = ePAR_OK;

	if ( ePAR_OK == status )
	{
		if ( u32_val > ( gp_par_table[ par_name ].max.u32 ))
		{
			*(uint32_t*)&gpu8_par_value[ gu32_par_addr_offset[par_name] ] = gp_par_table[ par_name ].max.u32;
			status = ePAR_WAR_LIM_TO_MAX;
		}
		else if ( u32_val < ( gp_par_table[ par_name ].min.u32 ))
		{
			*(uint32_t*)&gpu8_par_value[ gu32_par_addr_offset[par_name] ] = gp_par_table[ par_name ].min.u32;
			status = ePAR_WAR_LIM_TO_MIN;
		}
		else
		{
			*(uint32_t*)&gpu8_par_value[ gu32_par_addr_offset[par_name] ] = (uint32_t) ( u32_val );
			status = ePAR_OK;
		}
	}

	return status;
}


static par_status_t par_set_i32(const par_name_t par_name, const int32_t i32_val)
{
	par_status_t status = ePAR_OK;

	if ( ePAR_OK == status )
	{
		if ( i32_val > ( gp_par_table[ par_name ].max.i32 ))
		{
			*(int32_t*)&gpu8_par_value[ gu32_par_addr_offset[par_name] ] = gp_par_table[ par_name ].max.i32;
			status = ePAR_WAR_LIM_TO_MAX;
		}
		else if ( i32_val < ( gp_par_table[ par_name ].min.i32 ))
		{
			*(int32_t*)&gpu8_par_value[ gu32_par_addr_offset[par_name] ] = gp_par_table[ par_name ].min.i32;
			status = ePAR_WAR_LIM_TO_MIN;
		}
		else
		{
			*(int32_t*)&gpu8_par_value[ gu32_par_addr_offset[par_name] ] = (int32_t) ( i32_val );
			status = ePAR_OK;
		}
	}

	return status;
}


static par_status_t par_set_f32(const par_name_t par_name, const float32_t f32_val)
{
	par_status_t status = ePAR_OK;

	if ( ePAR_OK == status )
	{
		if ( f32_val > ( gp_par_table[ par_name ].max.f32 ))
		{
			*(float32_t*)&gpu8_par_value[ gu32_par_addr_offset[par_name] ] = gp_par_table[ par_name ].max.f32;
			status = ePAR_WAR_LIM_TO_MAX;
		}
		else if ( f32_val < ( gp_par_table[ par_name ].min.f32 ))
		{
			*(float32_t*)&gpu8_par_value[ gu32_par_addr_offset[par_name] ] = gp_par_table[ par_name ].min.f32;
			status = ePAR_WAR_LIM_TO_MIN;
		}
		else
		{
			*(float32_t*)&gpu8_par_value[ gu32_par_addr_offset[par_name] ] = (float32_t) ( f32_val );
			status = ePAR_OK;
		}
	}

	return status;
}


////////////////////////////////////////////////////////////////////////////////
/**
* @} <!-- END GROUP -->
*/
////////////////////////////////////////////////////////////////////////////////
