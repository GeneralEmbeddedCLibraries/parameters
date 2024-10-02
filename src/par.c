// Copyright (c) 2023 Ziga Miklosic
// All Rights Reserved
// This software is under MIT licence (https://opensource.org/licenses/MIT)
////////////////////////////////////////////////////////////////////////////////
/**
*@file      par.c
*@brief     Device parameters API functions
*@author    Ziga Miklosic
*@date      15.02.2023
*@version	V2.1.0
*/
////////////////////////////////////////////////////////////////////////////////
/**
*@addtogroup PARAMETERS_API
* @{ <!-- BEGIN GROUP -->
*
* 	Parameter kernel module
*/
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////////////////
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "par.h"
#include "par_nvm.h"
#include "../../par_cfg.h"
#include "../../par_if.h"

////////////////////////////////////////////////////////////////////////////////
// Definitions
////////////////////////////////////////////////////////////////////////////////

/**
 * 		Maximum size of string in bytes
 *
 * 	@note	Used for memcpy of name and unit get API functions.
 */
#define PAR_MAX_STRING_SIZE							( 32 )

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

#if ( PAR_CFG_DEBUG_EN )

	/**
	 * 	Status strings
	 */
	static const char * gs_status[] =
	{
		"OK",

		"ERROR",
		"ERROR INIT",
		"ERROR NVM",
		"ERROR CRC",

		"WARN SET TO DEF",
		"WARN NVM REWRITTEN",
		"NO PERSISTENT",
	};
#endif

////////////////////////////////////////////////////////////////////////////////
// Function Prototypes
////////////////////////////////////////////////////////////////////////////////
static par_status_t par_allocate_ram_space	(uint8_t ** pp_ram_space);
static uint32_t 	par_calc_ram_usage		(void);
static par_status_t	par_check_table_validy	(const par_cfg_t * const p_par_cfg);
static par_status_t par_set_u8				(const par_num_t par_num, const uint8_t u8_val);
static par_status_t par_set_i8				(const par_num_t par_num, const int8_t i8_val);
static par_status_t par_set_u16				(const par_num_t par_num, const uint16_t u16_val);
static par_status_t par_set_i16				(const par_num_t par_num, const int16_t i16_val);
static par_status_t par_set_u32				(const par_num_t par_num, const uint32_t u32_val);
static par_status_t par_set_i32				(const par_num_t par_num, const int32_t i32_val);
static par_status_t par_set_f32				(const par_num_t par_num, const float32_t f32_val);
static void         par_load_default        (void);

////////////////////////////////////////////////////////////////////////////////
// Functions
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
/**
*		Device Parameters initialization
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

    if ( false == gb_is_init )
    {
    	// Get parameter table
    	gp_par_table = par_cfg_get_table();
    	PAR_ASSERT( NULL != gp_par_table );

    	// Check if par table is defined correctly
    	status |= par_check_table_validy( gp_par_table );

    	// Allocate space in RAM
    	status |= par_allocate_ram_space( &gpu8_par_value );
    	PAR_ASSERT( NULL != gpu8_par_value );

    	// Initialize parameter interface
    	status |= par_if_init();

    	// Init succeed
    	if ( ePAR_OK == status )
    	{
    		gb_is_init = true;
    	}

    	// Set all parameters to default
    	par_load_default();

    	#if ( 1 == PAR_CFG_NVM_EN )

    		// Init and load parameters from NVM
    		status |= par_nvm_init();

    	#endif

    	PAR_DBG_PRINT( "PAR: Parameters initialized with status: %s", par_get_status_str( status ));
    }
    else
    {
        status = ePAR_ERROR_INIT;
    }

	return status;
}

////////////////////////////////////////////////////////////////////////////////
/**
*		De-initialize Device Parameters
*
* @return		status 		- Status of operation
*/
////////////////////////////////////////////////////////////////////////////////
par_status_t par_deinit(void)
{
    par_status_t status = ePAR_OK;

    if ( true == gb_is_init )
    {
    	#if ( 1 == PAR_CFG_NVM_EN )

    		// Init and load parameters from NVM
    		status |= par_nvm_deinit();

    	#endif
        
        // Module de-initialized
        gb_is_init = false;
    }
    else
    {
        status = ePAR_ERROR_INIT;
    }

    return status;
}

////////////////////////////////////////////////////////////////////////////////
/**
*		Get initialisation flag
*
* @param[out]   p_is_init   - Pointer to init flag
* @return		status 		- Status of operation
*/
////////////////////////////////////////////////////////////////////////////////
par_status_t par_is_init(bool * const p_is_init)
{
	par_status_t status = ePAR_OK;
    
    if ( NULL != p_is_init )
    {
        *p_is_init = gb_is_init;
    }
    else
    {
        status = ePAR_ERROR;
    }

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
* @note		Input is parameter number (enumeration) defined in par_cfg.h and not
* 			parameter ID number!
*
* @param[in]	par_num	- Parameter number (enumeration)
* @param[in]	p_val	- Pointer to value
* @return		status 	- Status of operation
*/
////////////////////////////////////////////////////////////////////////////////
par_status_t par_set(const par_num_t par_num, const void * p_val)
{
	par_status_t status = ePAR_OK;

	// Is init
	PAR_ASSERT( true == gb_is_init );

	// Check input
	PAR_ASSERT( par_num < ePAR_NUM_OF );

	if ( true == gb_is_init )
	{
		if ( par_num < ePAR_NUM_OF )
		{
			#if ( 1 == PAR_CFG_MUTEX_EN )
				if ( ePAR_OK == par_if_aquire_mutex())
            #endif
				{
					switch ( gp_par_table[ par_num ].type )
					{
						case ePAR_TYPE_U8:
							status = par_set_u8( par_num, *(uint8_t*) p_val );
							break;

						case ePAR_TYPE_I8:
							status = par_set_i8( par_num, *(int8_t*) p_val );
							break;

						case ePAR_TYPE_U16:
							status = par_set_u16( par_num, *(uint16_t*) p_val );
							break;

						case ePAR_TYPE_I16:
							status = par_set_i16( par_num, *(int16_t*) p_val );
							break;

						case ePAR_TYPE_U32:
							status = par_set_u32( par_num, *(uint32_t*) p_val );
							break;

						case ePAR_TYPE_I32:
							status = par_set_i32( par_num, *(int32_t*) p_val );
							break;

						case ePAR_TYPE_F32:
							status = par_set_f32( par_num, *(float32_t*) p_val );
							break;

						case ePAR_TYPE_NUM_OF:
						default:
							PAR_ASSERT( 0 );
							break;
					}

			#if ( 1 == PAR_CFG_MUTEX_EN )
					par_if_release_mutex();
            #endif
            #if ( 1 == PAR_CFG_AUTO_SAVE)
                    status |= par_save(par_num);
            #endif
				}
            #if ( 1 == PAR_CFG_MUTEX_EN )
				// Mutex not acquire
				else
				{
					status = ePAR_ERROR;
				}
			#endif
		}
		else
		{
			status = ePAR_ERROR;
		}
	}
	else
	{
		status = ePAR_ERROR_INIT;
	}

	return status;
}

////////////////////////////////////////////////////////////////////////////////
/**
*		Set parameter to default value
*
* @pre	Parameters must be initialised before usage!
*
* @param[in]	par_num	- Parameter number (enumeration)
* @return		status 	- Status of operation
*/
////////////////////////////////////////////////////////////////////////////////
par_status_t par_set_to_default(const par_num_t par_num)
{
	par_status_t status = ePAR_OK;

	PAR_ASSERT( true == gb_is_init );
	PAR_ASSERT( par_num < ePAR_NUM_OF );

	if ( true == gb_is_init )
	{
		if ( par_num < ePAR_NUM_OF )
		{
            status |= par_set(par_num, &gp_par_table[par_num].def);
		}
		else
		{
			status = ePAR_ERROR;
		}
	}
	else
	{
		status = ePAR_ERROR_INIT;
	}

	return status;
}

////////////////////////////////////////////////////////////////////////////////
/**
*		Set all parameters to default value
*
* @pre	Parameters must be initialised before usage!
*
* @return	status - Status of operation
*/
////////////////////////////////////////////////////////////////////////////////
par_status_t par_set_all_to_default(void)
{
	par_status_t	status 	= ePAR_OK;
	uint32_t 		par_num = 0UL;

	PAR_ASSERT( true == gb_is_init );

	if ( true == gb_is_init )
	{
		for ( par_num = 0; par_num < ePAR_NUM_OF; par_num++ )
		{
			par_set_to_default( par_num );
		}

		PAR_DBG_PRINT( "PAR: Setting all parameters to default" );
	}
	else
	{
		status = ePAR_ERROR_INIT;
	}

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
* @note		Input is parameter number (enumeration) defined in par_cfg.h and not
* 			parameter ID number!
*
* @param[in]	par_num	- Parameter number (enumeration)
* @param[out]	p_val	- Parameter value
* @return		status 	- Status of operation
*/
////////////////////////////////////////////////////////////////////////////////
par_status_t par_get(const par_num_t par_num, void * const p_val)
{
	par_status_t status = ePAR_OK;

	// Is init
	PAR_ASSERT( true == gb_is_init );

	// Check input
	PAR_ASSERT( par_num < ePAR_NUM_OF );

	#if ( 1 == PAR_CFG_MUTEX_EN )
		if ( ePAR_OK == par_if_aquire_mutex())
		{
	#endif
			switch ( gp_par_table[par_num].type )
			{
				case ePAR_TYPE_U8:
					*(uint8_t*) p_val = *(uint8_t*)&gpu8_par_value[ gu32_par_addr_offset[par_num] ];
					break;

				case ePAR_TYPE_I8:
					*(int8_t*) p_val = *(int8_t*)&gpu8_par_value[ gu32_par_addr_offset[par_num] ];
					break;

				case ePAR_TYPE_U16:
					*(uint16_t*) p_val = *(uint16_t*)&gpu8_par_value[ gu32_par_addr_offset[par_num] ];
					break;

				case ePAR_TYPE_I16:
					*(int16_t*) p_val = *(int16_t*)&gpu8_par_value[ gu32_par_addr_offset[par_num] ];
					break;

				case ePAR_TYPE_U32:
					*(uint32_t*) p_val = *(uint32_t*)&gpu8_par_value[ gu32_par_addr_offset[par_num] ];
					break;

				case ePAR_TYPE_I32:
					*(int32_t*) p_val = *(int32_t*)&gpu8_par_value[ gu32_par_addr_offset[par_num] ];
					break;

				case ePAR_TYPE_F32:
					*(float32_t*) p_val = *(float32_t*)&gpu8_par_value[ gu32_par_addr_offset[par_num] ];
					break;

				case ePAR_TYPE_NUM_OF:
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
*		Get parameter ID
*
* @param[in]	par_num	- Parameter number (enumeration)
* @param[in]	p_id 	- Pointer to parameter ID
* @return		status	- Status of operation
*/
////////////////////////////////////////////////////////////////////////////////
par_status_t par_get_id(const par_num_t par_num, uint16_t * const p_id)
{
	par_status_t status = ePAR_OK;

	PAR_ASSERT( true == gb_is_init );
	PAR_ASSERT( par_num < ePAR_NUM_OF );
	PAR_ASSERT( NULL != p_id );

	if ( true == gb_is_init )
	{
		if ( 	( par_num < ePAR_NUM_OF )
			&&	( NULL != p_id  ))
		{
			*p_id = gp_par_table[ par_num ].id;
		}
		else
		{
			status = ePAR_ERROR;
		}
	}
	else
	{
		status = ePAR_ERROR_INIT;
	}

	return status;
}

////////////////////////////////////////////////////////////////////////////////
/**
*		Get parameter number (enumeration) by ID
*
* @param[in]	id 			- Parameter ID
* @param[in]	p_par_num	- Pointer to parameter enumeration number
* @return		status		- Status of operation
*/
////////////////////////////////////////////////////////////////////////////////
par_status_t par_get_num_by_id(const uint16_t id, par_num_t * const p_par_num)
{
	par_status_t 	status 	= ePAR_OK;
	uint16_t 		par_num = 0UL;
	bool			found	= false;

	PAR_ASSERT( true == gb_is_init );
	PAR_ASSERT( NULL != p_par_num );

	if ( true == gb_is_init )
	{
		if ( NULL != p_par_num )
		{
			for ( par_num = 0; par_num < ePAR_NUM_OF; par_num++ )
			{
				if ( gp_par_table[par_num].id == id )
				{
					*p_par_num = par_num;
					found = true;
					break;
				}
			}

			// Does parameter with requested ID even exist
			if ( false == found )
			{
				status = ePAR_ERROR;
			}
		}
		else
		{
			status = ePAR_ERROR;
		}
	}
	else
	{
		status = ePAR_ERROR_INIT;
	}

	return status;
}

////////////////////////////////////////////////////////////////////////////////
/**
*		Get parameter configurations
*
* @param[in]	par_num		- Parameter number (enumeration)
* @param[in]	p_par_cfg	- Pointer to parameter configurations
* @return		status 		- Status of operation
*/
////////////////////////////////////////////////////////////////////////////////
par_status_t par_get_config(const par_num_t par_num, par_cfg_t * const p_par_cfg)
{
			par_status_t 	status 		= ePAR_OK;
	const 	par_cfg_t * 	p_cfg_table	= par_cfg_get_table();

	PAR_ASSERT( NULL != p_cfg_table );
	PAR_ASSERT( NULL != p_par_cfg );
	PAR_ASSERT( par_num < ePAR_NUM_OF );

	if ( 	( NULL != p_par_cfg )
		&& 	( NULL != p_cfg_table )
		&&	( par_num < ePAR_NUM_OF ))
	{
		*p_par_cfg = p_cfg_table[ par_num ];
	}
	else
	{
		status = ePAR_ERROR;
	}

	return status;
}

////////////////////////////////////////////////////////////////////////////////
/**
*		Get parameter type size
*
* @param[in]	type		- Data type of parameter
* @param[out]	p_size		- Pointer to parameter data type size
* @return		status 		- Status of operation
*/
////////////////////////////////////////////////////////////////////////////////
par_status_t par_get_type_size(const par_type_list_t type, uint8_t * const p_size)
{
	par_status_t status = ePAR_OK;

	PAR_ASSERT( type < ePAR_TYPE_NUM_OF );
	PAR_ASSERT( NULL != p_size );

	if ( 	( type < ePAR_TYPE_NUM_OF )
		&&	( NULL != p_size ))
	{
		switch ( type )
		{
			case ePAR_TYPE_U8:
				*p_size = sizeof( uint8_t );
				break;

			case ePAR_TYPE_I8:
				*p_size = sizeof( int8_t );
				break;

			case ePAR_TYPE_U16:
				*p_size = sizeof( uint16_t );
				break;

			case ePAR_TYPE_I16:
				*p_size = sizeof( int16_t );
				break;

			case ePAR_TYPE_U32:
				*p_size = sizeof( uint32_t );
				break;

			case ePAR_TYPE_I32:
				*p_size = sizeof( int32_t );
				break;

			case ePAR_TYPE_F32:
				*p_size = sizeof( float32_t );
				break;

			case ePAR_TYPE_NUM_OF:
			default:
				status = ePAR_ERROR;
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
*		Get parameter type
*
* @param[in]	par_num	- Parameter number (enumeration)
* @param[in]	p_type 	- Pointer to parameter type
* @return		status	- Status of operation
*/
////////////////////////////////////////////////////////////////////////////////
par_status_t par_get_type(const par_num_t par_num, par_type_list_t *const p_type)
{
	par_status_t status = ePAR_OK;

	PAR_ASSERT( true == gb_is_init );
	PAR_ASSERT( NULL != p_type );
    PAR_ASSERT( ePAR_NUM_OF > par_num );

	if ( 	( true == gb_is_init )
		&&	( NULL != p_type )
        &&  ( ePAR_NUM_OF > par_num ))
	{
        *p_type = gp_par_table[par_num].type;
	}
	else
	{
		status = ePAR_ERROR;
	}

	return status;
}

////////////////////////////////////////////////////////////////////////////////
/**
*		Get parameter value range
*
* @param[in]	par_num	- Parameter number (enumeration)
* @param[in]	p_type 	- Pointer to value range
* @return		status	- Status of operation
*/
////////////////////////////////////////////////////////////////////////////////
par_status_t par_get_range(const par_num_t par_num, par_range_t *const p_range)
{
	par_status_t status = ePAR_OK;

	PAR_ASSERT( true == gb_is_init );
	PAR_ASSERT( NULL != p_range );
    PAR_ASSERT( ePAR_NUM_OF > par_num );

	if ( 	( true == gb_is_init )
		&&	( NULL != p_range )
        &&  ( ePAR_NUM_OF > par_num ))
	{
        p_range->min = gp_par_table[par_num].min;
        p_range->max = gp_par_table[par_num].max;
	}
	else
	{
		status = ePAR_ERROR;
	}

	return status;
}

#if ( 1 == PAR_CFG_NVM_EN )

	////////////////////////////////////////////////////////////////////////////////
	/**
	*		Store all parameters value to NVM
	*
	* @pre		NVM storage must be initialized first and "PAR_CFG_NVM_EN"
	* 			settings must be enabled.
	*
	* @return		status 	- Status of operation
	*/
	////////////////////////////////////////////////////////////////////////////////
	par_status_t par_save_all(void)
	{
		par_status_t status = ePAR_OK;

		PAR_ASSERT( true == gb_is_init );

		if ( true == gb_is_init )
		{
			status = par_nvm_write_all();
		}
		else
		{
			status = ePAR_ERROR_INIT;
		}

		return status;
	}

	////////////////////////////////////////////////////////////////////////////////
	/**
	*		Store single parameter value to NVM
	*
	* @pre		NVM storage must be initialized first and "PAR_CFG_NVM_EN"
	* 			settings must be enabled.
	*
	* @param[in]	par_num	- Parameter number (enumeration)
	* @return		status 	- Status of operation
	*/
	////////////////////////////////////////////////////////////////////////////////
	par_status_t par_save(const par_num_t par_num)
	{
		par_status_t status = ePAR_OK;

		PAR_ASSERT( true == gb_is_init );

		if ( true == gb_is_init )
		{
			status = par_nvm_write( par_num, true );
		}
		else
		{
			status = ePAR_ERROR_INIT;
		}

		return status;
	}

	////////////////////////////////////////////////////////////////////////////////
	/**
	*		Store single parameter value to NVM by its ID value
	*
	* @pre		NVM storage must be initialized first and "PAR_CFG_NVM_EN"
	* 			settings must be enabled.
	*
	* @code
	* 			// Use case
	* 			// Store par from ID 10 to 32
	* 			uint8_t par_id;
	*
	* 			for ( par_id = 10; par_id < 32; par_id++ )
	* 			{
	* 			 	status |= par_save_by_id( par_id )
	* 			}
	*
	* @endcode
	*
	* @param[in]	par_id	- Parameter ID number
	* @return		status 	- Status of operation
	*/
	////////////////////////////////////////////////////////////////////////////////
	par_status_t par_save_by_id(const uint16_t par_id)
	{
		par_status_t 	status 	= ePAR_OK;
		par_num_t		par_num	= 0;

		PAR_ASSERT( true == gb_is_init );

		if ( true == gb_is_init )
		{
			status = par_get_num_by_id( par_id, &par_num );

			if ( ePAR_OK == status )
			{
				status = par_save( par_num );
			}
		}
		else
		{
			status = ePAR_ERROR_INIT;
		}

		return status;
	}

	////////////////////////////////////////////////////////////////////////////////
	/**
	*		Clean all stored parameters inside NVM
	*
	* @note		This function shall be locked as it will erase complete parameter
	* 			region of NVM space. Shall be used only during
	*
	* @pre		NVM storage must be initialized first and "PAR_CFG_NVM_EN"
	* 			settings must be enabled.
	*
	* @return		status 	- Status of operation
	*/
	////////////////////////////////////////////////////////////////////////////////
	par_status_t par_save_clean(void)
	{
		par_status_t status = ePAR_OK;

		PAR_ASSERT( true == gb_is_init );

		if ( true == gb_is_init )
		{
			status = par_nvm_reset_all();
		}
		else
		{
			status = ePAR_ERROR_INIT;
		}

		return status;
	}

#endif

#if ( PAR_CFG_DEBUG_EN )

	////////////////////////////////////////////////////////////////////////////////
	/**
	*		Get status string description
	*
	* @param[in]	status	- Parameter status
	* @return		str		- Parameter status description
	*/
	////////////////////////////////////////////////////////////////////////////////
	const char * par_get_status_str(const par_status_t status)
	{
		uint8_t i = 0;
		const char * str = "N/A";

		if ( ePAR_OK == status  )
		{
			str = (const char*) gs_status[0];
		}
		else
		{
			for ( i = 0; i < 16; i++ )
			{
				if ( status & ( 1<<i ))
				{
					str =  (const char*) gs_status[i+1];
					break;
				}
			}
		}

		return str;
	}
#endif

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

////////////////////////////////////////////////////////////////////////////////
/**
*		Allocate space for live parameter values
*
* @param[in]	pp_ram_space	- Pointer to pointer allocated space
* @return		status			- Status of operation
*/
////////////////////////////////////////////////////////////////////////////////
static par_status_t par_allocate_ram_space(uint8_t ** pp_ram_space)
{
	par_status_t 	status 		= ePAR_OK;
	uint32_t		ram_size	= 0UL;

	// Calculate total size of RAM
	ram_size = par_calc_ram_usage();

	// Allocate space in RAM
	*pp_ram_space = malloc( ram_size );
	PAR_ASSERT( NULL != *pp_ram_space );

	return status;
}

////////////////////////////////////////////////////////////////////////////////
/**
*		Calculate total size for parameter live values
*
* @note 	This function may not be compatible with other microcontroller
* 			architectures as it is based on STM32 with its data alignment policy!
*
* @return		total_size - Size of all parameters in bytes
*/
////////////////////////////////////////////////////////////////////////////////
static uint32_t par_calc_ram_usage(void)
{
	par_cfg_t 	par_cfg 		= { 0 };
	uint32_t 	par_num			= 0UL;
	uint32_t	total_size		= 0UL;
	uint8_t		par_type_size	= 0;

	// For every parameter
	for ( par_num = 0; par_num < ePAR_NUM_OF; par_num++ )
	{
		// Get parameter configs
		par_get_config( par_num, &par_cfg );

        // Align addresses
        if	(	( par_cfg.type == ePAR_TYPE_U16 )
        	|| 	( par_cfg.type == ePAR_TYPE_I16 ))
        {
        	// 2 bytes alignment
            while(( total_size % 2 ) != 0 )
            {
            	total_size++;
            }
        }

        else if (	( par_cfg.type == ePAR_TYPE_U32 )
        		|| 	( par_cfg.type == ePAR_TYPE_I32 )
				|| 	( par_cfg.type == ePAR_TYPE_F32 ))
        {
        	// 4 bytes alignment
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
        gu32_par_addr_offset[par_num] = total_size;

		// Get size of data type
		par_get_type_size( par_cfg.type, &par_type_size );

        // Accumulate total RAM space
        total_size += par_type_size;
	}

	return total_size;
}

////////////////////////////////////////////////////////////////////////////////
/**
*		Check that parameter table is correctly defined
*
* @param[in]	p_par_cfg	- Pointer to parameters table
* @return		status		- Status of operation
*/
////////////////////////////////////////////////////////////////////////////////
static par_status_t	par_check_table_validy(const par_cfg_t * const p_par_cfg)
{
	par_status_t status = ePAR_OK;

	// For each parameter
	for ( uint32_t i = 0; i < ePAR_NUM_OF; i++ )
	{
		// Compare parameters IDs
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

		/**
		 * 	Check for correct MIN, MAX and DEF value definitions
		 *
		 *	1. Check that MAX is larger than MIN
		 *	2. Check that DEF is equal or less than MAX
		 *	3. Check that DEF is equal or more than MIN
		 */
		PAR_ASSERT(( ePAR_TYPE_U8 == p_par_cfg[i].type ) 	? ((( p_par_cfg[i].min.u8 < p_par_cfg[i].max.u8 ) && ( p_par_cfg[i].def.u8 <= p_par_cfg[i].max.u8 )) && (  p_par_cfg[i].min.u8 <= p_par_cfg[i].def.u8 )) : ( 1 ));
		PAR_ASSERT(( ePAR_TYPE_I8 == p_par_cfg[i].type ) 	? ((( p_par_cfg[i].min.i8 < p_par_cfg[i].max.i8 ) && ( p_par_cfg[i].def.i8 <= p_par_cfg[i].max.i8 )) && (  p_par_cfg[i].min.i8 <= p_par_cfg[i].def.i8 )) : ( 1 ));
		PAR_ASSERT(( ePAR_TYPE_U16 == p_par_cfg[i].type ) 	? ((( p_par_cfg[i].min.u16 < p_par_cfg[i].max.u16 ) && ( p_par_cfg[i].def.u16 <= p_par_cfg[i].max.u16 )) && (  p_par_cfg[i].min.u16 <= p_par_cfg[i].def.u16 )) : ( 1 ));
		PAR_ASSERT(( ePAR_TYPE_I16 == p_par_cfg[i].type ) 	? ((( p_par_cfg[i].min.i16 < p_par_cfg[i].max.i16 ) && ( p_par_cfg[i].def.i16 <= p_par_cfg[i].max.i16 )) && (  p_par_cfg[i].min.i16 <= p_par_cfg[i].def.i16 )) : ( 1 ));
		PAR_ASSERT(( ePAR_TYPE_U32 == p_par_cfg[i].type ) 	? ((( p_par_cfg[i].min.u32 < p_par_cfg[i].max.u32 ) && ( p_par_cfg[i].def.u32 <= p_par_cfg[i].max.u32 )) && (  p_par_cfg[i].min.u32 <= p_par_cfg[i].def.u32 )) : ( 1 ));
		PAR_ASSERT(( ePAR_TYPE_I32 == p_par_cfg[i].type ) 	? ((( p_par_cfg[i].min.i32 < p_par_cfg[i].max.i32 ) && ( p_par_cfg[i].def.i32 <= p_par_cfg[i].max.i32 )) && (  p_par_cfg[i].min.i32 <= p_par_cfg[i].def.i32 )) : ( 1 ));
		PAR_ASSERT(( ePAR_TYPE_F32 == p_par_cfg[i].type ) 	? ((( p_par_cfg[i].min.f32 < p_par_cfg[i].max.f32 ) && ( p_par_cfg[i].def.f32 <= p_par_cfg[i].max.f32 )) && (  p_par_cfg[i].min.f32 <= p_par_cfg[i].def.f32 )) : ( 1 ));
	}

	return status;
}

////////////////////////////////////////////////////////////////////////////////
/**
*		Set unsigned 8-bit parameter
*
* @param[in]	par_num	- Parameter number (enumeration)
* @param[in]	u8_val	- Value of parameter
* @return		status	- Status of operation
*/
////////////////////////////////////////////////////////////////////////////////
static par_status_t par_set_u8(const par_num_t par_num, const uint8_t u8_val)
{
	par_status_t status = ePAR_OK;

	if ( ePAR_OK == status )
	{
		if ( u8_val > ( gp_par_table[ par_num ].max.u8 ))
		{
			*(uint8_t*)&gpu8_par_value[ gu32_par_addr_offset[par_num] ] = gp_par_table[ par_num ].max.u8;
		}
		else if ( u8_val < ( gp_par_table[ par_num ].min.u8 ))
		{
			*(uint8_t*)&gpu8_par_value[ gu32_par_addr_offset[par_num] ] = gp_par_table[ par_num ].min.u8;
		}
		else
		{
			*(uint8_t*)&gpu8_par_value[ gu32_par_addr_offset[par_num] ] = (uint8_t) ( u8_val );
		}
	}

	return status;
}

////////////////////////////////////////////////////////////////////////////////
/**
*		Set signed 8-bit parameter
*
* @param[in]	par_num	- Parameter number (enumeration)
* @param[in]	i8_val	- Value of parameter
* @return		status	- Status of operation
*/
////////////////////////////////////////////////////////////////////////////////
static par_status_t par_set_i8(const par_num_t par_num, const int8_t i8_val)
{
	par_status_t status = ePAR_OK;

	if ( ePAR_OK == status )
	{
		if ( i8_val > ( gp_par_table[ par_num ].max.i8 ))
		{
			*(int8_t*)&gpu8_par_value[ gu32_par_addr_offset[par_num] ] = gp_par_table[ par_num ].max.i8;
		}
		else if ( i8_val < ( gp_par_table[ par_num ].min.i8 ))
		{
			*(int8_t*)&gpu8_par_value[ gu32_par_addr_offset[par_num] ] = gp_par_table[ par_num ].min.i8;
		}
		else
		{
			*(int8_t*)&gpu8_par_value[ gu32_par_addr_offset[par_num] ] = (int8_t) ( i8_val );
		}
	}

	return status;
}

////////////////////////////////////////////////////////////////////////////////
/**
*		Set unsigned 16-bit parameter
*
* @param[in]	par_num	- Parameter number (enumeration)
* @param[in]	u16_val	- Value of parameter
* @return		status	- Status of operation
*/
////////////////////////////////////////////////////////////////////////////////
static par_status_t par_set_u16(const par_num_t par_num, const uint16_t u16_val)
{
	par_status_t status = ePAR_OK;

	if ( ePAR_OK == status )
	{
		if ( u16_val > ( gp_par_table[ par_num ].max.u16 ))
		{
			*(uint16_t*)&gpu8_par_value[ gu32_par_addr_offset[par_num] ] = gp_par_table[ par_num ].max.u16;
		}
		else if ( u16_val < ( gp_par_table[ par_num ].min.u16 ))
		{
			*(uint16_t*)&gpu8_par_value[ gu32_par_addr_offset[par_num] ] = gp_par_table[ par_num ].min.u16;
		}
		else
		{
			*(uint16_t*)&gpu8_par_value[ gu32_par_addr_offset[par_num] ] = (uint16_t) ( u16_val );
		}
	}

	return status;
}

////////////////////////////////////////////////////////////////////////////////
/**
*		Set signed 16-bit parameter
*
* @param[in]	par_num	- Parameter number (enumeration)
* @param[in]	i16_val	- Value of parameter
* @return		status	- Status of operation
*/
////////////////////////////////////////////////////////////////////////////////
static par_status_t par_set_i16(const par_num_t par_num, const int16_t i16_val)
{
	par_status_t status = ePAR_OK;

	if ( ePAR_OK == status )
	{
		if ( i16_val > ( gp_par_table[ par_num ].max.i16 ))
		{
			*(int16_t*)&gpu8_par_value[ gu32_par_addr_offset[par_num] ] = gp_par_table[ par_num ].max.i16;
		}
		else if ( i16_val < ( gp_par_table[ par_num ].min.i16 ))
		{
			*(int16_t*)&gpu8_par_value[ gu32_par_addr_offset[par_num] ] = gp_par_table[ par_num ].min.i16;
		}
		else
		{
			*(int16_t*)&gpu8_par_value[ gu32_par_addr_offset[par_num] ] = (int16_t) ( i16_val );
		}
	}

	return status;
}

////////////////////////////////////////////////////////////////////////////////
/**
*		Set unsigned 32-bit parameter
*
* @param[in]	par_num	- Parameter number (enumeration)
* @param[in]	u32_val	- Value of parameter
* @return		status	- Status of operation
*/
////////////////////////////////////////////////////////////////////////////////
static par_status_t par_set_u32(const par_num_t par_num, const uint32_t u32_val)
{
	par_status_t status = ePAR_OK;

	if ( ePAR_OK == status )
	{
		if ( u32_val > ( gp_par_table[ par_num ].max.u32 ))
		{
			*(uint32_t*)&gpu8_par_value[ gu32_par_addr_offset[par_num] ] = gp_par_table[ par_num ].max.u32;
		}
		else if ( u32_val < ( gp_par_table[ par_num ].min.u32 ))
		{
			*(uint32_t*)&gpu8_par_value[ gu32_par_addr_offset[par_num] ] = gp_par_table[ par_num ].min.u32;
		}
		else
		{
			*(uint32_t*)&gpu8_par_value[ gu32_par_addr_offset[par_num] ] = (uint32_t) ( u32_val );
		}
	}

	return status;
}

////////////////////////////////////////////////////////////////////////////////
/**
*		Set signed 32-bit parameter
*
* @param[in]	par_num	- Parameter number (enumeration)
* @param[in]	i32_val	- Value of parameter
* @return		status	- Status of operation
*/
////////////////////////////////////////////////////////////////////////////////
static par_status_t par_set_i32(const par_num_t par_num, const int32_t i32_val)
{
	par_status_t status = ePAR_OK;

	if ( ePAR_OK == status )
	{
		if ( i32_val > ( gp_par_table[ par_num ].max.i32 ))
		{
			*(int32_t*)&gpu8_par_value[ gu32_par_addr_offset[par_num] ] = gp_par_table[ par_num ].max.i32;
		}
		else if ( i32_val < ( gp_par_table[ par_num ].min.i32 ))
		{
			*(int32_t*)&gpu8_par_value[ gu32_par_addr_offset[par_num] ] = gp_par_table[ par_num ].min.i32;
		}
		else
		{
			*(int32_t*)&gpu8_par_value[ gu32_par_addr_offset[par_num] ] = (int32_t) ( i32_val );
		}
	}

	return status;
}

////////////////////////////////////////////////////////////////////////////////
/**
*		Set floating value parameter
*
* @param[in]	par_num	- Parameter number (enumeration)
* @param[in]	f32_val	- Value of parameter
* @return		status	- Status of operation
*/
////////////////////////////////////////////////////////////////////////////////
static par_status_t par_set_f32(const par_num_t par_num, const float32_t f32_val)
{
	par_status_t status = ePAR_OK;

	if ( ePAR_OK == status )
	{
		if ( f32_val > ( gp_par_table[ par_num ].max.f32 ))
		{
			*(float32_t*)&gpu8_par_value[ gu32_par_addr_offset[par_num] ] = gp_par_table[ par_num ].max.f32;
		}
		else if ( f32_val < ( gp_par_table[ par_num ].min.f32 ))
		{
			*(float32_t*)&gpu8_par_value[ gu32_par_addr_offset[par_num] ] = gp_par_table[ par_num ].min.f32;
		}
		else
		{
			*(float32_t*)&gpu8_par_value[ gu32_par_addr_offset[par_num] ] = (float32_t) ( f32_val );
		}
	}

	return status;
}

////////////////////////////////////////////////////////////////////////////////
/**
*		Load parameter default values to live space
*
* @return		void
*/
////////////////////////////////////////////////////////////////////////////////
static void par_load_default(void)
{
	par_cfg_t par_cfg = {0};
	uint8_t par_type_size = 0;

    for ( par_num_t par_num = 0; par_num < ePAR_NUM_OF; par_num++ )
    {
        // Get par type
        par_get_config( par_num, &par_cfg );

        // Get size of data type
        par_get_type_size( par_cfg.type, &par_type_size );

        // Copy default value to live space
        memcpy( &gpu8_par_value[gu32_par_addr_offset[par_num]], &gp_par_table[par_num].def.u8, par_type_size );
    }

    PAR_DBG_PRINT( "PAR: Loading default parameters" );
}

////////////////////////////////////////////////////////////////////////////////
/**
* @} <!-- END GROUP -->
*/
////////////////////////////////////////////////////////////////////////////////
