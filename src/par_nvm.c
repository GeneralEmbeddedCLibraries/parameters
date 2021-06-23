////////////////////////////////////////////////////////////////////////////////
/**
*@file      par_nvm.c
*@brief     Parameter storage to non-volatile memory
*@author    Ziga Miklosic
*@date      22.05.2021
*@version	V1.0.0
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
* 			Parameter NVM region is further organized into following sections:
*
*			-----------------------------------------------------------------
* 				1. Signature:
* 			-----------------------------------------------------------------
*
* 			Signature tells if NVM "Parameters" region is already initialized.
*
*				SIGNATURE DEFINITION:
*
*			size: 			4 bytes
*			base address*:	0x00
*
* 						| 	byte 3	  | 	byte 2	  | 	byte 1	 | 		byte 0	  |
* 						---------------------------------------------------------------
*						|	0x55	  |		0xAA	  |		0x00     | 		0xFF	  |
*						---------------------------------------------------------------
*			address
*			offset*:		0x03			0x02			0x01			0x00
*
*
*			-----------------------------------------------------------------
* 				2. Header:
* 			-----------------------------------------------------------------
*
*			Header contains number of stored parameters and it's checksum. Purpose of
*			header is to tell how many parameters are currently stored inside NVM
*			region "Parameters".
*
*			Based on "number of stored parameter" value read logic is being limited.
*
*			IMPORTANT: 	Header CRC-16 is being calculated only on "number of stored
*						parameters" value.
*
* 			HEADER DEFINITION:
*
*			size: 			8 bytes
*			base address*:	0x04
*
*
* 						| 	byte 3	  | 	byte 2	  | 	byte 1	 | 		byte 0	  |
* 						---------------------------------------------------------------
*						|					number of stored parameters			      |
*						---------------------------------------------------------------
*			address
*			offset**:		0x03			0x02			0x01			0x00
*
*						| 	byte 7	  | 	byte 6	  | 	byte 5	 | 		byte 4	  |
* 						---------------------------------------------------------------
*						|			CRC-16			  |	 	       reserved		  	  |
*						---------------------------------------------------------------
*			address
*			offset**:		0x07			0x06			0x05			0x04
*
*			-----------------------------------------------------------------
* 				3. Table ID:
* 			-----------------------------------------------------------------
*
*			Table ID is based on hash number calculated based on whole
*			parameter configuration table. It's purpose is to detect table
*			change in run-time.
*
*			This feature can detect that "RAM" parameter table and "NVM"
*			parameter table are different. Knowing that copy parameter value
*			from NVM to RAM is prohibited as data will be misinterpreted.
*
*				TABLE ID:
*
*			size: 				32 bytes
*			base address*: 		0x10
*
*			*relative to "Parameters" NVM region
*
*			-----------------------------------------------------------------
*				4. Parameters NVM objects
*			-----------------------------------------------------------------
*
*			NVM object consist of parameter value, parameter ID and its CRC-16
*			checksum. Every time parameter is being read data integrity is being
*			check based	on it's checksum.
*
*			TODO: This logic shall be implemented in future:
*
*				Persistence parameters are being stored into consecutive sequence
*				starting from top of a parameter table to bottom. Therefore first
*				parameters will be stored in lower address space relative to the
*				bottom defined one.
*
*				For now parameters NVM memory storage is only defined by its
*				ID as following funtion:
*
*					NVM_address = (8 * parameter ID) + base address
*
*			TODO END:
*
*
*			IMPORTANT: 	After release is done, parameters from table shall not
*						be removed or changed but only new parameters definitions
*						is allowed to be append to existing table.
*
*
*			size: 			8 bytes
*			base address*:	0x100
*
* 						| 	byte 3	  | 	byte 2	  | 	byte 1	 | 		byte 0	  |
* 						---------------------------------------------------------------
*						|						parameter value						  |
*						---------------------------------------------------------------
*			address
*			offset**:		0x03			0x02			0x01			0x00
*
*						| 	byte 7	  | 	byte 6	  | 	byte 5	 | 		byte 4	  |
* 						---------------------------------------------------------------
*						|			CRC-16			  |	 	    parameter ID		  |
*						---------------------------------------------------------------
*			address
*			offset**:		0x07			0x06			0x05			0x04
*
*
*			*relative to "Parameters" NVM region
*			**plus: (8 * parameter ID) + base address
*
*			-----------------------------------------------------------------
*
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
*
* @brief	STARTUP SEQUENCE:
*
* 				check for signature --[invalid]--> complete rewrite of "Parameters" NVM region (signature, header, tableID & all parameters value)
* 					|
* 				  [ok]
* 					|
* 					-> check for table ID --[table diff]--> complete rewrite of "Parameters" NVM region (signature, header, tableID & all parameters value)
* 							|
* 						  [same]
* 							|
* 							-> load stored parameters values from NVM to live "RAM" space
*
*/
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////////////////
#include "par_nvm.h"
#include "../../par_cfg.h"
#include "../../par_if.h"

#include <assert.h>

#if ( 1 == PAR_CFG_NVM_EN )

	#include "middleware/nvm/nvm/src/nvm.h"

	/**
	 * 	Check NVM module compatibility
	 */
	static_assert( 1 == NVM_VER_MAJOR );
	static_assert( 0 == NVM_VER_MINOR );
	static_assert( 0 == NVM_VER_DEVELOP );

	////////////////////////////////////////////////////////////////////////////////
	// Definitions
	////////////////////////////////////////////////////////////////////////////////

	/**
	 * 	Parameter signature NVM address offset
	 */
	#define PAR_NVM_SIGNATURE_ADDR_OFFSET			( 0x00 )

	/**
	 * 	Parameter signature
	 */
	#define PAR_NVM_SIGNATURE						( 0xFF00AA55 )

	/**
	 * 	Parameter header NVM address offset
	 */
	#define PAR_NVM_HEADER_ADDR_OFFSET				( 0x04 )

	/**
	 * 	Parameter table ID NVM address offset
	 */
	#define PAR_NVM_TABLE_ID_ADDR_OFFSET			( 0x10 )

	/**
	 * 	Parameter object NVM address offset
	 */
	#define PAR_NVM_PAR_OBJ_ADDR_OFFSET				( 0x100 )

	/**
	 * 		Parameter NVM object
	 *
	 * 	@note 	Each parameter is protected with CRC-16.
	 *
	 * 			Parameter header has same structure but only
	 * 			ID field is not being used!
	 */
	typedef union
	{
		struct
		{
			uint32_t 	val;	/**<4-byte storage for parameter value */
			uint16_t	id;		/**<Parameter ID */
			uint16_t	crc;	/**<CRC-16 of parameter value */
		} field;
		uint64_t u;
	}par_nvm_obj_t;

	////////////////////////////////////////////////////////////////////////////////
	// Variables
	////////////////////////////////////////////////////////////////////////////////


	////////////////////////////////////////////////////////////////////////////////
	// Function Prototypes
	////////////////////////////////////////////////////////////////////////////////
	static par_status_t		par_nvm_check_signature				(void);
	static par_status_t		par_nvm_write_signature				(void);
	static par_status_t		par_nvm_erase_signature				(void);
	static par_status_t 	par_nvm_read_header					(uint32_t * const p_num_of_par);
	static par_status_t 	par_nvm_write_header				(const uint32_t num_of_par);
	static uint16_t 		par_nvm_calc_crc					(const uint8_t * const p_data, const uint8_t size);
	static par_status_t		par_nvm_load_all					(void);
	static uint32_t			par_nvm_calc_num_of_per_par			(void);

	#if ( 1 == PAR_CFG_TABLE_ID_CHECK_EN )
		static par_status_t 	par_nvm_check_table_id	(void);
		static par_status_t 	par_nvm_write_table_id	(void);
	#endif



	////////////////////////////////////////////////////////////////////////////////
	// Functions
	////////////////////////////////////////////////////////////////////////////////




	par_status_t par_nvm_init(void)
	{
		par_status_t 	status 			= ePAR_OK;
		uint32_t		num_of_per_par 	= 0UL;

		// Pre-condition
		PAR_ASSERT( true == nvm_is_init());
		PAR_ASSERT( true == par_is_init());

		// Check NVM signature
		if ( ePAR_OK == par_nvm_check_signature())
		{
			PAR_DBG_PRINT( "PAR_NVM: Signature OK" );

			#if ( 1 == PAR_CFG_TABLE_ID_CHECK_EN )

				// Same parameters table
				if ( ePAR_OK == par_nvm_check_table_id())
				{
					PAR_DBG_PRINT( "PAR_NVM: Unique table ID OK" )

					// Load all parameters from NVM
					status |= par_nvm_load_all();
				}

				// Parameters table change
				else
				{
					PAR_DBG_PRINT( "PAR_NVM: Unique table ID invalid. Rewrite complete NVM memory..." )

					// Clean signature
					status |= par_nvm_erase_signature();

					// Write new table
					status |= par_nvm_write_table_id();

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
					status |= par_nvm_write_table_id();
				#endif

				// Write header
				status |= par_nvm_write_header( num_of_per_par );

				// Write default values to NVM
				status |= par_store_all_to_nvm();

				// Lastly write signature
				// NOTE: 	Safety aspect to write signature last. Signature presents some validation factor!
				//			Possible power lost during table ID or header write will not have any side effects!
				status |= par_nvm_write_signature();

				PAR_DBG_PRINT( "PAR_NVM: Storing %u parameters to NVM", num_of_per_par );
			}

			// None of the persistent parameter
			else
			{
				// No actions..
			}
		}

		PAR_ASSERT( ePAR_OK == status );

		return status;
	}


	par_status_t par_nvm_write(const par_num_t par_num)
	{
		par_status_t 	status 		= ePAR_OK;
		par_nvm_obj_t	par_obj		= { .u = 0ULL };
		uint32_t		par_addr	= 0UL;

		// Pre-condition
		PAR_ASSERT( true == nvm_is_init());

		// Legal call
		PAR_ASSERT( true == par_is_init());
		PAR_ASSERT( par_num < ePAR_NUM_OF )
		PAR_ASSERT( true == par_get_persistance( par_num ))

		// Get current par value
		par_get( par_num, (uint32_t*) &par_obj.field.val );

		// Calculate CRC
		par_obj.field.crc = par_nvm_calc_crc((uint8_t*) &par_obj.field.val, 6U );

		// Calculate parameter NVM address
		par_addr = (uint32_t)( PAR_NVM_PAR_OBJ_ADDR_OFFSET + ( 4UL * par_num ));

		// Write to NVM
		if ( eNVM_OK != nvm_write( PAR_CFG_NVM_REGION, par_addr, sizeof( par_nvm_obj_t ), (const uint8_t*) &par_obj.u ))
		{
			status = ePAR_ERROR_NVM;
		}

		return status;
	}


	par_status_t par_nvm_read(const par_num_t par_num)
	{
		par_status_t 	status 		= ePAR_OK;
		par_nvm_obj_t	par_obj		= { .u = 0ULL };
		uint32_t		par_addr	= 0UL;
		uint32_t		calc_crc	= 0UL;

		// Pre-condition
		PAR_ASSERT( true == nvm_is_init());

		// Legal call
		PAR_ASSERT( true == par_is_init());
		PAR_ASSERT( par_num < ePAR_NUM_OF )
		PAR_ASSERT( true == par_get_persistance( par_num ))

		// Calculate parameter NVM address
		par_addr = (uint32_t)( PAR_NVM_PAR_OBJ_ADDR_OFFSET + ( 4UL * par_num ));

		// Read from NVM
		if ( eNVM_OK != nvm_read( PAR_CFG_NVM_REGION, par_addr, sizeof( par_nvm_obj_t ), (uint8_t*) &par_obj.u ))
		{
			status = ePAR_ERROR_NVM;
		}
		else
		{
/*			// Calculate CRC
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
				status = ePAR_ERROR_NVM;
			}*/

			// TODO: Remove only testing
			par_set( par_num, (uint32_t*) &par_obj.field.val );
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
	*@addtogroup KERNEL_PAR_NVM_FUNCTIONS
	* @{ <!-- BEGIN GROUP -->
	*
	* 	Kernel functions of device parameters NVM handling
	*/
	////////////////////////////////////////////////////////////////////////////////

	static par_status_t	par_nvm_check_signature(void)
	{
		par_status_t 	status 	= ePAR_OK;
		uint32_t 		sign	= 0UL;

		if ( eNVM_OK != nvm_read( PAR_CFG_NVM_REGION, PAR_NVM_SIGNATURE_ADDR_OFFSET, 4U, (uint8_t*) &sign ))
		{
			status = ePAR_ERROR_NVM;
		}
		else
		{
			if ( PAR_NVM_SIGNATURE != sign )
			{
				status = ePAR_ERROR;
			}
		}

		return status;
	}


	static par_status_t	par_nvm_write_signature(void)
	{
		par_status_t 	status 	= ePAR_OK;
		uint32_t 		sign	= PAR_NVM_SIGNATURE;

		status = nvm_write( PAR_CFG_NVM_REGION, PAR_NVM_SIGNATURE_ADDR_OFFSET, 4U, (uint8_t*) &sign );

		return status;
	}


	static par_status_t	par_nvm_erase_signature(void)
	{
		par_status_t status = ePAR_OK;

		status = nvm_erase( PAR_CFG_NVM_REGION, PAR_NVM_SIGNATURE_ADDR_OFFSET, 4U );

		return status;
	}

	#if ( 1 == PAR_CFG_TABLE_ID_CHECK_EN )

		static par_status_t par_nvm_check_table_id(void)
		{
			par_status_t status = ePAR_OK;


			return status;
		}


		static par_status_t par_nvm_write_table_id(void)
		{
			par_status_t status = ePAR_OK;



			return status;
		}

	#endif // 1 == PAR_CFG_TABLE_ID_CHECK_EN


	static par_status_t par_nvm_read_header(uint32_t * const p_num_of_par)
	{
		par_status_t 	status 		= ePAR_OK;
		par_nvm_obj_t	par_obj		= { .u = 0ULL };
		uint32_t		calc_crc	= 0UL;

		// Pre-condition
		PAR_ASSERT( true == nvm_is_init());

		// Check inputs
		PAR_ASSERT( NULL != p_num_of_par );

		if ( eNVM_OK != nvm_read( PAR_CFG_NVM_REGION, PAR_NVM_HEADER_ADDR_OFFSET, sizeof( par_nvm_obj_t ), (uint8_t*) &par_obj.u ))
		{
			status = ePAR_ERROR_NVM;
		}
		else
		{
			// Calculate CRC
			calc_crc = par_nvm_calc_crc((uint8_t*) &par_obj.field.val, 4U );

			// Validate CRC
			if ( calc_crc == par_obj.field.crc )
			{
				*p_num_of_par = par_obj.field.val;
			}

			// CRC corrupt
			else
			{
				status = ePAR_ERROR_NVM;
			}
		}

		return status;
	}


	static par_status_t	par_nvm_write_header(const uint32_t num_of_par)
	{
		par_status_t 	status 	= ePAR_OK;
		par_nvm_obj_t	par_obj	= { .field.val = num_of_par, .field.id = 0U, .field.crc = 0U };

		// Pre-condition
		PAR_ASSERT( true == nvm_is_init());

		// Calculate CRC
		par_obj.field.crc = par_nvm_calc_crc((uint8_t*) &par_obj.field.id, 4U );

		// Write to NVM
		if ( eNVM_OK != nvm_write( PAR_CFG_NVM_REGION, PAR_NVM_HEADER_ADDR_OFFSET, sizeof( par_nvm_obj_t ), (const uint8_t*) &par_obj.u ))
		{
			status = ePAR_ERROR_NVM;
		}

		return status;
	}


	static uint16_t par_nvm_calc_crc(const uint8_t * const p_data, const uint8_t size)
	{
		uint16_t crc16 = 0;


		return crc16;
	}

	static par_status_t par_nvm_load_all(void)
	{
		par_status_t 	status 	= ePAR_OK;
		uint32_t 		par_num = 0UL;

		for ( par_num = 0; par_num < ePAR_NUM_OF; par_num++ )
		{
			if ( true == par_get_persistance( par_num ))
			{
				if ( ePAR_OK != par_nvm_read( par_num ))
				{
					status = ePAR_ERROR_NVM;
					break;
				}
			}
		}

		PAR_DBG_PRINT( "PAR: Loading all parameters from NVM. Status: %u", status );

		return status;
	}

	static uint32_t	par_nvm_calc_num_of_per_par(void)
	{
		uint32_t num_of_per_par = 0UL;
		uint32_t par_num 		= 0UL;

		PAR_ASSERT( true == par_is_init());

		for ( par_num = 0; par_num < ePAR_NUM_OF; par_num++ )
		{
			if ( true == par_get_persistance( par_num ))
			{
				num_of_per_par++;
			}
		}

		return num_of_per_par;
	}

	////////////////////////////////////////////////////////////////////////////////
	/**
	* @} <!-- END GROUP -->
	*/
	////////////////////////////////////////////////////////////////////////////////

#endif // 1 == PAR_CFG_NVM_EN
