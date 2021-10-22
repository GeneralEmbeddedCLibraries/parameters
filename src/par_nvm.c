// Copyright (c) 2021 Ziga Miklosic
// All Rights Reserved
// This software is under MIT licence (https://opensource.org/licenses/MIT)
////////////////////////////////////////////////////////////////////////////////
/**
*@file      par_nvm.c
*@brief     Parameter storage to non-volatile memory
*@author    Ziga Miklosic
*@date      22.05.2021
*@version	V1.0.1
*/
////////////////////////////////////////////////////////////////////////////////
/**
*@addtogroup PAR_NVM
* @{ <!-- BEGIN GROUP -->
*
* 	Parameter storage to non-volatile memory handling.
*
*
* @pre		NVM module shall have memory region called "Parameters".
*
* 			NVM module must be initialized before calling any of following
* 			functions.
*
* @brief	This module is responsible for parameter NVM object creation and
* 			storage manipulation. NVM parameter object consist of it's value
* 			and a CRC value for validation purposes.
*
* 			Parameter storage is reserved in "Parameters" region of NVM. Look
* 			at the nvm_cfg.h/c module for NVM region descriptions.
*
*			Parameters stored into NVM in little endianness format.
*
*			For details how parameters are handled in NVM go look at the
*			documentation.
*
* @note		RULES OF "PAR_CFG_TABLE_ID_CHECK_EN" SETTINGS:
*
* 			It is normal that parameter table will change during development
* 			and therefore code will detect table change between "RAM" and "NVM"
* 			(detection of par table change enable/disable with
* 			"PAR_CFG_TABLE_ID_CHECK_EN" setting).
*
* 			But as SW is released and potential at customer, developer must not
* 			change the pre-existing parameter table as it will loose all values
* 			stored inside NVM. Therefore it is recommended to disable table ID
* 			detection after first release of SW. Adding new parameters to pre-existing
* 			table has no harm at all nor does it have any side effects.
*
*/
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////////////////
#include "par_nvm.h"
#include "../../par_cfg.h"
#include "../../par_if.h"

#if ( 1 == PAR_CFG_NVM_EN )

	#include <assert.h>
	#include <string.h>

	#include "middleware/nvm/nvm/src/nvm.h"

	/**
	 * 	Check NVM module compatibility
	 */
	static_assert( 1 == NVM_VER_MAJOR );
	static_assert( 0 == NVM_VER_MINOR );
	//static_assert( 0 == NVM_VER_DEVELOP );

	////////////////////////////////////////////////////////////////////////////////
	// Definitions
	////////////////////////////////////////////////////////////////////////////////


	/**
	 * 	Parameter signature and size in bytes
	 */
	#define PAR_NVM_SIGN							( 0xFF00AA55 )
	#define PAR_NVM_SIGN_SIZE						( 4UL )

	/**
	 * 	Parameter header number of object size
	 *
	 * 	Unit: byte
	 */
	#define PAR_NVM_NB_OF_OBJ_SIZE					( 2UL )

	/**
	 * 	Parameter CRC size
	 *
	 * 	Unit: byte
	 */
	#define PAR_NVM_CRC_SIZE						( 2UL )

	/**
	 * 	Parameter configuration hash size
	 *
	 * 	Unit: byte
	 */
	#define PAR_NVM_HASH_SIZE						( 32UL )

	/**
	 * 	Parameter NVM header content address start
	 *
	 * 	@note 	This is offset to reserved NVM region. For absolute address
	 * 			add that value to NVM start region.
	 */
	#define PAR_NVM_HEAD_ADDR						( 0x00 )
	#define PAR_NVM_HEAD_SIGN_ADDR					( PAR_NVM_HEAD_ADDR )
	#define PAR_NVM_HEAD_NB_OF_OBJ_ADDR				( PAR_NVM_HEAD_SIGN_ADDR 		+ PAR_NVM_HEAD_SIGN_ADDR 	)
	#define PAR_NVM_HEAD_CRC_ADDR					( PAR_NVM_HEAD_NB_OF_OBJ_ADDR 	+ PAR_NVM_NB_OF_OBJ_SIZE 	)
	#define PAR_NVM_HEAD_HASH_ADDR					( PAR_NVM_HEAD_CRC_ADDR 		+ PAR_NVM_CRC_SIZE 			)

	/**
	 * 	Parameters first data object start address
	 *
	 * 	Unit: byte
	 */
	#define PAR_NVM_FIRST_DATA_OBJ_ADDR				( PAR_NVM_HEAD_HASH_ADDR + PAR_NVM_HEAD_HASH_ADDR )

	/**
	 * 	Parameter NVM header object
	 */
	typedef struct
	{
		uint32_t sign;		/**<Signature */
		uint16_t obj_nb;	/**<Stored data object number */
		uint16_t crc;		/**<Header CRC */
	} par_nvm_head_obj_t;

	/**
	 * 	Parameter NVM data object
	 */
	typedef struct
	{
		uint16_t	id;		/**<Parameter ID */
		uint8_t		size;	/**<Size of parameter data block */
		uint8_t		crc;	/**<CRC of parameter value */
		par_type_t 	val;	/**<4-byte storage for parameter value */
	} par_nvm_data_obj_t;

	typedef struct
	{
		uint32_t addr;
		uint16_t id;
	} par_nvm_lut_t;

	////////////////////////////////////////////////////////////////////////////////
	// Variables
	////////////////////////////////////////////////////////////////////////////////

	/**
	 * 	Parameter NVM lut
	 */
	static par_nvm_lut_t g_par_nvm_data_obj_addr[ePAR_NUM_OF] = {0};

	////////////////////////////////////////////////////////////////////////////////
	// Function Prototypes
	////////////////////////////////////////////////////////////////////////////////
	static par_status_t		par_nvm_load_all					(const uint16_t num_of_par);
	static par_status_t 	par_nvm_read						(const par_num_t par_num);
	static par_status_t		par_nvm_reset_all					(void);

	static par_status_t		par_nvm_read_signature				(void);
	static par_status_t		par_nvm_write_signature				(void);
	static par_status_t		par_nvm_corrupt_signature			(void);

	static par_status_t 	par_nvm_read_header					(uint16_t * const p_num_of_par);
	static par_status_t 	par_nvm_write_header				(const uint16_t num_of_par);
	static par_status_t 	par_nvm_validate_header				(uint16_t * const p_num_of_par);

	static uint16_t 		par_nvm_calc_crc					(const uint8_t * const p_data, const uint8_t size);
	static uint16_t			par_nvm_get_per_par					(void);

	static void 			par_nvm_build_nvm_lut				(void);

	#if ( 1 == PAR_CFG_TABLE_ID_CHECK_EN )
		static par_status_t	par_nvm_erase_signature	(void);
		static par_status_t par_nvm_check_table_id	(const uint8_t * const p_table_id);
		static par_status_t par_nvm_write_table_id	(const uint8_t * const p_table_id);
	#endif

	////////////////////////////////////////////////////////////////////////////////
	// Functions
	////////////////////////////////////////////////////////////////////////////////

	////////////////////////////////////////////////////////////////////////////////
	/**
	*		Initialize parameter NVM handling
	*
	* @brief 	Based on settings in "par_cfg.h" initialisation phase is done.
	* 			Settings such as "PAR_CFG_TABLE_ID_CHECK_EN" will affect checking
	* 			for table ID.
	*
	* @return	status - Status of initialisation
	*/
	////////////////////////////////////////////////////////////////////////////////
	par_status_t par_nvm_init(void)
	{
		par_status_t 	status 		= ePAR_OK;
		uint16_t 		obj_nb		= 0;
		uint16_t		per_par_nb	= 0;

		PAR_ASSERT( true == nvm_is_init());

		// Get number of persistent parameters
		per_par_nb = par_nvm_get_per_par();

		// At least one persistent parameter
		if ( per_par_nb > 0 )
		{
			// Read signature
			status = par_nvm_read_signature();

			// Signature OK
			if ( ePAR_OK == status )
			{
				// Check hash
				#if ( PAR_CFG_TABLE_ID_CHECK_EN )
					// TODO: ...
				#endif

				// Validate header
				status = par_nvm_validate_header( &obj_nb );

				// Header OK
				if ( ePAR_OK == status )
				{
					// Load all parameters
					status = par_nvm_load_all( obj_nb );
				}
			}

			// Signature NOT OK
			else if ( ePAR_ERROR == status )
			{
				status = par_nvm_reset_all();
			}

			// NVM Error
			else
			{
				// No actions...
			}
		}

		// No persistent parameters
		else
		{
			PAR_DBG_PRINT( "PAR_NVM: No persistent parameters... Nothing to do..." );
		}










/*
		uint32_t		num_of_per_par 	= 0UL;

		#if ( 1 == PAR_CFG_TABLE_ID_CHECK_EN )

			static uint8_t par_table_id[32] = { 0 };

			// Calculate parameter table unique ID
			par_if_calc_hash( par_cfg_get_table(), par_cfg_get_table_size(), (uint8_t*) &par_table_id );

		#endif

		// Pre-condition
		PAR_ASSERT( true == par_nvm_precondition_check() );

		// Check NVM signature
		if ( ePAR_OK == par_nvm_check_signature())
		{
			PAR_DBG_PRINT( "PAR_NVM: Signature OK" );

			#if ( 1 == PAR_CFG_TABLE_ID_CHECK_EN )

				// Same parameters table
				if ( ePAR_OK == par_nvm_check_table_id((uint8_t*) &par_table_id ))
				{
					PAR_DBG_PRINT( "PAR_NVM: Unique table ID OK" );

					// Load all parameters from NVM
					status |= par_nvm_load_all();
				}

				// Parameters table change
				else
				{
					PAR_DBG_PRINT( "PAR_NVM: Unique table ID invalid. Rewrite complete NVM memory..." );

					// Clean signature
					status |= par_nvm_erase_signature();

					// Write new table
					status |= par_nvm_write_table_id((uint8_t*) &par_table_id );

					// Write new header
					num_of_per_par = par_nvm_calc_num_of_per_par();
					status |= par_nvm_write_header( num_of_per_par );

					// Write signature
					status |= par_nvm_write_signature();

					// Set all parameters to default values
					par_set_all_to_default();

					// Write default values to NVM
					status |= par_store_all_to_nvm();
				}

			#else

				// Load all parameters from NVM
				status |= par_nvm_load_all();

			#endif
		}

		// No signature
		else
		{
			PAR_DBG_PRINT( "PAR_NVM: Signature missing" );

			// Set all parameters to default values
			par_set_all_to_default();

			// Get how many persistant parameters there are
			num_of_per_par = par_nvm_calc_num_of_per_par();

			if ( num_of_per_par > 0 )
			{
				#if ( 1 == PAR_CFG_TABLE_ID_CHECK_EN )
					status |= par_nvm_write_table_id((uint8_t*) &par_table_id );
				#endif

				// Write header
				status |= par_nvm_write_header( num_of_per_par );

				// Write default values to NVM
				status |= par_save_all();

				// Lastly write signature
				// NOTE: 	Safety aspect to write signature last. Signature presents some validation factor!
				//			Possible power lost during table ID or header write will not have any side effects!
				status |= par_nvm_write_signature();
			}

			// None of the persistent parameter
			else
			{
				// No actions..
			}
		}

		//PAR_ASSERT( ePAR_OK == status );
*/

		return status;
	}

	////////////////////////////////////////////////////////////////////////////////
	/**
	*		Store parameter value to NVM
	*
	* @param[in]	par_num	- Parameter enumeration number
	* @return		status 	- Status of operation
	*/
	////////////////////////////////////////////////////////////////////////////////
	par_status_t par_nvm_write(const par_num_t par_num)
	{
		par_status_t 	status 		= ePAR_OK;

		PAR_ASSERT( true == nvm_is_init());
		PAR_ASSERT( par_num < ePAR_NUM_OF )








/*
		par_nvm_obj_t	par_obj		= { .u = 0ULL };
		uint32_t		par_addr	= 0UL;
		par_cfg_t		par_cfg		= {0};

		PAR_ASSERT( true == par_nvm_precondition_check());
		PAR_ASSERT( par_num < ePAR_NUM_OF )

		if ( par_num < ePAR_NUM_OF )
		{
			// Get configuration
			par_get_config( par_num, &par_cfg );

			// Is that parameter persistent
			if ( true == par_cfg.persistant )
			{
				// Get current par value
				par_get( par_num, (uint32_t*) &par_obj.field.val );

				// Get parameter ID
				par_get_id( par_num, &par_obj.field.id );

				// Calculate CRC
				par_obj.field.crc = par_nvm_calc_crc((uint8_t*) &par_obj.field.val, 6U );

				// Calculate parameter NVM address
				par_addr = (uint32_t)( PAR_NVM_PAR_OBJ_ADDR_OFFSET + ( 8UL * par_obj.field.id ));

				// Write to NVM
				if ( eNVM_OK != nvm_write( PAR_CFG_NVM_REGION, par_addr, sizeof( par_nvm_obj_t ), (const uint8_t*) &par_obj.u ))
				{
					status = ePAR_ERROR_NVM;
				}
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
		*/

		return status;
	}

	////////////////////////////////////////////////////////////////////////////////
	/**
	*		Store all parameter value to NVM
	*
	* @return		status 	- Status of operation
	*/
	////////////////////////////////////////////////////////////////////////////////
	par_status_t par_nvm_write_all(void)
	{
		par_status_t 	status 		= ePAR_OK;
		uint16_t		par_num		= 0;
		uint16_t		per_par_nb	= 0;
		par_cfg_t		par_cfg		= { 0 };

		for ( par_num = 0UL; par_num < ePAR_NUM_OF; par_num++ )
		{
			par_get_config( par_num, &par_cfg );

			if ( true == par_cfg.persistant )
			{
				status |= par_nvm_write( par_num );
				per_par_nb++;
			}
		}

		// Write header
		status |= par_nvm_write_header( per_par_nb );

		PAR_DBG_PRINT( "PAR_NVM: Storing all to NVM status: %s", par_get_status_str(status) );

		return status;
	}

	////////////////////////////////////////////////////////////////////////////////
	/**
	* @} <!-- END GROUP -->
	*/
	////////////////////////////////////////////////////////////////////////////////

	////////////////////////////////////////////////////////////////////////////////
	/**
	*@addtogroup KERNEL_PAR_NVM_FUNCTIONS
	* @{ <!-- BEGIN GROUP -->
	*
	* 	Kernel functions of device parameters NVM handling
	*/
	////////////////////////////////////////////////////////////////////////////////

	////////////////////////////////////////////////////////////////////////////////
	/**
	*		Read parameter value from NVM
	*
	* @note		This function is being part of a API as it is only needed at init
	* 			phase.
	*
	* @param[in]	par_num	- Parameter enumeration number
	* @return		status 	- Status of operation
	*/
	////////////////////////////////////////////////////////////////////////////////
	static par_status_t par_nvm_read(const par_num_t par_num)
	{
		par_status_t 	status 		= ePAR_OK;

		PAR_ASSERT( true == nvm_is_init());
		PAR_ASSERT( par_num < ePAR_NUM_OF )


/*		par_status_t 	status 		= ePAR_OK;
		par_nvm_obj_t	par_obj		= { .u = 0ULL };
		uint32_t		par_addr	= 0UL;
		uint32_t		calc_crc	= 0UL;
		uint16_t		par_id		= 0UL;

		PAR_ASSERT( true == par_nvm_precondition_check() );
		PAR_ASSERT( par_num < ePAR_NUM_OF );

		// Get parameter ID
		par_get_id( par_num, &par_obj.field.id );

		// Calculate parameter NVM address
		par_addr = (uint32_t)( PAR_NVM_PAR_OBJ_ADDR_OFFSET + ( 8UL * par_id ));

		// Read from NVM
		if ( eNVM_OK != nvm_read( PAR_CFG_NVM_REGION, par_addr, sizeof( par_nvm_obj_t ), (uint8_t*) &par_obj.u ))
		{
			status = ePAR_ERROR_NVM;
		}
		else
		{
			// Calculate CRC
			calc_crc = par_nvm_calc_crc((uint8_t*) &par_obj.field.val, 6U );

			// Validate CRC
			if ( calc_crc == par_obj.field.crc )
			{
				par_set( par_num, (uint32_t*) &par_obj.field.val );
			}

			// CRC corrupt
			else
			{
				par_set_to_default( par_num );
				status = ePAR_ERROR_CRC;
			}
		}

		*/

		return status;
	}

	////////////////////////////////////////////////////////////////////////////////
	/**
	*		Read parameter NVM signature
	*
	* @brief	Return ePAR_OK if signature OK. In case of NVM error it returns
	* 			ePAR_ERROR_NVM.
	*
	* @return		status 	- Status of operation
	*/
	////////////////////////////////////////////////////////////////////////////////
	static par_status_t	par_nvm_read_signature(void)
	{
		par_status_t 	status 	= ePAR_OK;
		uint32_t 		sign	= 0UL;

		if ( eNVM_OK != nvm_read( PAR_CFG_NVM_REGION, PAR_NVM_HEAD_SIGN_ADDR, PAR_NVM_SIGN_SIZE, (uint8_t*) &sign ))
		{
			status = ePAR_ERROR_NVM;
			PAR_DBG_PRINT( "PAR_NVM: NVM error during signature read!" );
		}
		else
		{
			if ( PAR_NVM_SIGN == sign )
			{
				status = ePAR_OK;
				PAR_DBG_PRINT( "PAR_NVM: Signature OK!" );
			}
			else
			{
				status = ePAR_ERROR;
				PAR_DBG_PRINT( "PAR_NVM: Signature corrupted!" );
			}
		}

		return status;
	}

	////////////////////////////////////////////////////////////////////////////////
	/**
	*		Write parameter signature to NVM
	*
	*	@brief	Return ePAR_OK if signature written OK. In case of NVM error it returns
	* 			ePAR_ERROR_NVM.
	*
	* @return		status 	- Status of operation
	*/
	////////////////////////////////////////////////////////////////////////////////
	static par_status_t	par_nvm_write_signature(void)
	{
		par_status_t 	status 	= ePAR_OK;
		uint32_t 		sign	= PAR_NVM_SIGN;

		if ( eNVM_OK != nvm_write( PAR_CFG_NVM_REGION, PAR_NVM_HEAD_SIGN_ADDR, PAR_NVM_SIGN_SIZE, (uint8_t*) &sign ))
		{
			status = ePAR_ERROR_NVM;
			PAR_DBG_PRINT( "PAR_NVM: NVM error during signature write!" );
		}

		return status;
	}

	////////////////////////////////////////////////////////////////////////////////
	/**
	*		Corrupt parameter signature to NVM
	*
	* @brief	Return ePAR_OK if signature corrupted OK. In case of NVM error it returns
	* 			ePAR_ERROR_NVM.
	*
	* @return		status 	- Status of operation
	*/
	////////////////////////////////////////////////////////////////////////////////
	static par_status_t	par_nvm_corrupt_signature(void)
	{
		par_status_t status = ePAR_OK;

		if ( eNVM_OK != nvm_erase( PAR_CFG_NVM_REGION, PAR_NVM_HEAD_SIGN_ADDR, PAR_NVM_SIGN_SIZE ))
		{
			status = ePAR_ERROR_NVM;
			PAR_DBG_PRINT( "PAR_NVM: NVM error during signature corruption!" );
		}

		return status;
	}

	#if ( 1 == PAR_CFG_TABLE_ID_CHECK_EN )

		////////////////////////////////////////////////////////////////////////////////
		/**
		*		Erase parameter signature from NVM
		*
		* @return		status 	- Status of operation
		*/
		////////////////////////////////////////////////////////////////////////////////
		static par_status_t	par_nvm_erase_signature(void)
		{
			par_status_t status = ePAR_OK;

			status = nvm_erase( PAR_CFG_NVM_REGION, PAR_NVM_SIGNATURE_ADDR_OFFSET, 4U );

			return status;
		}

		////////////////////////////////////////////////////////////////////////////////
		/**
		*		Check unique parameter table ID
		*
		* @brief	This function check for parameter configuration table change while
		* 			some parameters are already stored in NVM. First it read stored
		* 			table ID from NVM and then compare it with current table ID (live
		* 			from RAM). In case of mismatched it return error.
		*
		* 			Table ID is being calculated based on hash algorithm (SHA-256).
		*
		* @param[in]	p_table_id	- Pointer to reference table ID (in "RAM")
		* @return		status 		- Status of operation
		*/
		////////////////////////////////////////////////////////////////////////////////
		static par_status_t par_nvm_check_table_id(const uint8_t * const p_table_id)
		{
			par_status_t 	status 				= ePAR_OK;
			uint8_t 		nvm_table_id[32] 	= { 0 };

			if ( eNVM_OK != nvm_read( eNVM_REGION_EEPROM_RUN_PAR, PAR_NVM_TABLE_ID_ADDR_OFFSET, 32U, (uint8_t*) &nvm_table_id ))
			{
				status = ePAR_ERROR_NVM;
			}
			else
			{
				// Table ID is the same in "RAM" and in NVM
				if ( 0 == memcmp( &nvm_table_id, p_table_id, 32U ))
				{
					status = ePAR_OK;
				}

				// Different table ID found
				else
				{
					status = ePAR_ERROR;
				}
			}

			return status;
		}

		////////////////////////////////////////////////////////////////////////////////
		/**
		*		Write unique parameter table ID to NVM
		*
		* @param[in]	p_table_id	- Pointer to reference table ID (in "RAM")
		* @return		status 		- Status of operation
		*/
		////////////////////////////////////////////////////////////////////////////////
		static par_status_t par_nvm_write_table_id(const uint8_t * const p_table_id)
		{
			par_status_t status = ePAR_OK;

			if ( eNVM_OK != nvm_write( eNVM_REGION_EEPROM_RUN_PAR, PAR_NVM_TABLE_ID_ADDR_OFFSET, 32U, p_table_id ))
			{
				status = ePAR_ERROR_NVM;
			}

			return status;
		}

	#endif // 1 == PAR_CFG_TABLE_ID_CHECK_EN

	////////////////////////////////////////////////////////////////////////////////
	/**
	*		Read parameter NVM header
	*
	* @param[in]	p_num_of_par	- Pointer to number of stored parameters in NVM
	* @return		status 			- Status of operation
	*/
	////////////////////////////////////////////////////////////////////////////////
	static par_status_t par_nvm_read_header(uint16_t * const p_num_of_par)
	{
		par_status_t 		status 		= ePAR_OK;
		par_nvm_head_obj_t	head_obj	= {0};
		uint32_t			calc_crc	= 0UL;

		PAR_ASSERT( true == nvm_is_init());
		PAR_ASSERT( NULL != p_num_of_par );

		if ( eNVM_OK != nvm_read( PAR_CFG_NVM_REGION, PAR_NVM_HEAD_ADDR, sizeof( par_nvm_head_obj_t ), (uint8_t*) &head_obj ))
		{
			status = ePAR_ERROR_NVM;
			PAR_DBG_PRINT( "PAR_NVM: NVM error during header read!" );
		}
		else
		{
			// Calculate CRC
			calc_crc = par_nvm_calc_crc((uint8_t*) &head_obj.obj_nb, PAR_NVM_NB_OF_OBJ_SIZE );

			// Validate CRC
			if ( calc_crc == head_obj.crc )
			{
				*p_num_of_par = head_obj.obj_nb;
			}

			// CRC corrupt
			else
			{
				status = ePAR_ERROR_CRC;
				PAR_DBG_PRINT( "PAR_NVM: Header CRC corrupted!" );
			}
		}

		return status;
	}

	////////////////////////////////////////////////////////////////////////////////
	/**
	*		Write parameter NVM header
	*
	* @param[in]	num_of_par	- Number of persistent parameters that are stored in NVM
	* @return		status 		- Status of operation
	*/
	////////////////////////////////////////////////////////////////////////////////
	static par_status_t	par_nvm_write_header(const uint16_t num_of_par)
	{
		par_status_t 		status 		= ePAR_OK;
		par_nvm_head_obj_t	head_obj	= {0};

		// Pre-condition
		PAR_ASSERT( true == nvm_is_init());

		// Add number of objects
		head_obj.obj_nb = num_of_par;

		// Calculate CRC
		head_obj.crc = par_nvm_calc_crc((uint8_t*) &head_obj.obj_nb, PAR_NVM_NB_OF_OBJ_SIZE );

		// Write num of object and CRC
		if ( eNVM_OK != nvm_write( PAR_CFG_NVM_REGION, PAR_NVM_HEAD_ADDR, sizeof( par_nvm_head_obj_t ), (const uint8_t*) &head_obj ))
		{
			status = ePAR_ERROR_NVM;
			PAR_DBG_PRINT( "PAR_NVM: NVM error during header write!" );
		}

		// Write signature
		status |= par_nvm_write_signature();

		return status;
	}

	static par_status_t par_nvm_validate_header(uint16_t * const p_num_of_par)
	{
		par_status_t 	status = ePAR_OK;
		uint16_t 		obj_nb = 0;

		// Read header
		status = par_nvm_read_header( &obj_nb );

		// NVM error
		if ( ePAR_ERROR_NVM == status )
		{
			// No actions...
		}
		else
		{
			// Header OK and persistent parameters found
			if 	(	( ePAR_OK == status )
				&&	( obj_nb > 0 ))
			{
				*p_num_of_par = obj_nb;
				status = ePAR_OK;
			}
			else
			{
				status = par_nvm_reset_all();
			}
		}

		return status;
	}

	////////////////////////////////////////////////////////////////////////////////
	/**
	*		Calculate CRC-16
	*
	* @param[in]	p_data	- Pointer to data
	* @param[in]	size	- Size of data to calc crc
	* @return		crc16	- Calculated CRC
	*/
	////////////////////////////////////////////////////////////////////////////////
	static uint16_t par_nvm_calc_crc(const uint8_t * const p_data, const uint8_t size)
	{
		const 	uint16_t poly 	= 0x1021U;	// CRC-16-CCITT
		const 	uint16_t seed 	= 0x1234U;	// Custom seed
				uint16_t crc16 	= seed;

		// Check input
		PAR_ASSERT( NULL != p_data );
		PAR_ASSERT( size > 0 );

	    for (uint8_t i = 0; i < size; i++)
	    {
	    	crc16 = ( crc16 ^ ( p_data[i] << 8U ));

	        for (uint8_t j = 0U; j < 8U; j++)
	        {
	        	if (crc16 & 0x8000)
	        	{
	        		crc16 = (( crc16 << 1U ) ^ poly );
	            }
	        	else
	            {
	        		crc16 = ( crc16 << 1U );
	            }
	        }
	    }

		return crc16;
	}

	////////////////////////////////////////////////////////////////////////////////
	/**
	*		Load all parameters value from NVM
	*
	*
	*	TODO: Shall be defined what is the action of corrupted CRC detection on top level!
	*
	* @return		status 		- Status of operation
	*/
	////////////////////////////////////////////////////////////////////////////////
	static par_status_t par_nvm_load_all(const uint16_t num_of_par)
	{
		par_status_t 	status 			= ePAR_OK;




/*		uint32_t 		par_num 		= 0UL;
		uint32_t		stored_par_num	= 0UL;
		uint32_t		loaded_par_num	= 0UL;

		// Get number of stored parameters
		if ( ePAR_OK == par_nvm_read_header( &stored_par_num ))			// TODO: FIx this !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
		{
			// Loop thru par table
			for ( par_num = 0; par_num < ePAR_NUM_OF; par_num++ )
			{
				status |= par_nvm_read( par_num );
				loaded_par_num++;
			}

			PAR_DBG_PRINT( "PAR_NVM: Loading %u of %u stored parameters from NVM. Status: %u", loaded_par_num, stored_par_num, status );
		}
		else
		{
			status = ePAR_ERROR_NVM;

			PAR_DBG_PRINT( "PAR_NVM: Reading header error!" );
		}*/

		return status;
	}

	////////////////////////////////////////////////////////////////////////////////
	/**
	*		Get total number of persistent parameters
	*
	* @return	num_of_per_par - Number of persistent parameters
	*/
	////////////////////////////////////////////////////////////////////////////////
	static uint16_t	par_nvm_get_per_par(void)
	{
		uint16_t 	num_of_per_par 	= 0UL;
		uint16_t 	par_num 		= 0UL;
		par_cfg_t	par_cfg			= {0};

		for ( par_num = 0; par_num < ePAR_NUM_OF; par_num++ )
		{
			par_get_config( par_num, &par_cfg );

			if ( true == par_cfg.persistant )
			{
				num_of_per_par++;
			}
		}

		return num_of_per_par;
	}

	////////////////////////////////////////////////////////////////////////////////
	/**
	*		Reset total section of parameters NVM
	*
	* @brief	This function completely re-write whole NVM section.
	*
	* 			It first corrupt signature in order to raise "working in progress"
	* 			flag.
	*
	* @return	num_of_per_par - Number of persistent parameters
	*/
	////////////////////////////////////////////////////////////////////////////////
	static par_status_t par_nvm_reset_all(void)
	{
		par_status_t status = ePAR_OK;

		// Build NVM lut
		par_nvm_build_nvm_lut();

		// Corrupt header
		status |= par_nvm_corrupt_signature();

		// Write all data object
		status |= par_nvm_write_all();

		return status;
	}

	static void par_nvm_build_nvm_lut(void)
	{
		uint16_t 		per_par_nb 			= 0;
		par_num_t		par_num				= 0;
		par_cfg_t		par_cfg				= {0};
		par_type_list_t	par_type_prev		= 0;
		uint8_t			par_type_prev_size	= 0;

		for ( par_num = 0; par_num < ePAR_NUM_OF; par_num++ )
		{
			par_get_config( par_num, &par_cfg );

			if ( true == par_cfg.persistant )
			{
				// First parameter
				if ( 0 == per_par_nb )
				{
					g_par_nvm_data_obj_addr[per_par_nb].addr = PAR_NVM_FIRST_DATA_OBJ_ADDR;
				}

				// Build consecutive address space
				// NOTE: For know each NVM data object is fixed in size (4 bytes)
				else
				{
					// Get previous data type size
					par_get_type_size( par_type_prev, &par_type_prev_size );

					// Calculate and add address of next parameter
					g_par_nvm_data_obj_addr[per_par_nb].addr = ( g_par_nvm_data_obj_addr[per_par_nb-1].addr + par_type_prev_size );
				}

				// Store parameter ID
				g_par_nvm_data_obj_addr[per_par_nb].id = par_cfg.id;

				// Next persistent parameter
				per_par_nb++;

				// Store previous data type
				par_type_prev = par_cfg.type;
			}
		}
	}

	////////////////////////////////////////////////////////////////////////////////
	/**
	* @} <!-- END GROUP -->
	*/
	////////////////////////////////////////////////////////////////////////////////

#endif // 1 == PAR_CFG_NVM_EN
