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
* 			HEADER DEFINITION:
*
* 			size:			4 bytes
* 			base address*:	0x04
*
*						| 	byte 7	  | 	byte 6	  | 	byte 5	 | 		byte 4	  |
* 						---------------------------------------------------------------
*						|par_num[15:8]| par_num[7:0]  |	  CRC[15:8]  | 		CRC[7:0]  |
*						---------------------------------------------------------------
*			address
*			offset*:		0x07			0x06			0x05			0x04
*
*				where: 	par_num - is number of stored parameters
*						CRC		- checksum of "par_num" value
*
*			*relative to "Parameters" NVM region
*
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
*			NVM object consist of parameter value and its 32-CRC checksum. Every
*			time parameter is being read data integrity is being check based
*			on it's checksum.
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
						| 	byte 7	  | 	byte 6	  | 	byte 5	 | 		byte 4	  |
* 						---------------------------------------------------------------
*						|						32 - CRC							  |
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


#if ( 1 == PAR_CFG_NVM_EN )

	#include "middleware/nvm/nvm/src/nvm.h"

	////////////////////////////////////////////////////////////////////////////////
	// Definitions
	////////////////////////////////////////////////////////////////////////////////


	/**
	 * 	Parameter object NVM address offset
	 */
	#define PAR_NVM_PAR_OBJ_ADDR_OFFSET				( 0x100 )

	/**
	 * 		Parameter NVM object
	 *
	 * 	@note 	Each parameter is protected with 32-CRC.
	 */
	typedef union
	{
		struct
		{
			uint32_t 	val;	/**<4-byte storage for parameter value */
			uint32_t	crc;	/**<32-CRC of parameter value */
		} field;
		uint64_t u;
	}par_nvm_obj_t;

	////////////////////////////////////////////////////////////////////////////////
	// Variables
	////////////////////////////////////////////////////////////////////////////////


	////////////////////////////////////////////////////////////////////////////////
	// Function Prototypes
	////////////////////////////////////////////////////////////////////////////////
	static par_status_t		par_nvm_check_signature	(void);
	static par_status_t		par_nvm_write_signature	(void);
	static par_status_t		par_nvm_clean_signature	(void);
	static par_status_t 	par_nvm_check_header	(void);
	static par_status_t 	par_nvm_write_header	(void);
	static uint32_t 		par_nvm_calc_crc32		(const uint32_t val);
	static par_status_t		par_nvm_load_all		(void);

	#if ( 1 == PAR_CFG_TABLE_ID_CHECK_EN )
		static par_status_t 	par_nvm_check_table_id	(void);
		static par_status_t 	par_nvm_write_table_id	(void);
	#endif



	////////////////////////////////////////////////////////////////////////////////
	// Functions
	////////////////////////////////////////////////////////////////////////////////




	par_status_t par_nvm_init(void)
	{
		par_status_t status = ePAR_OK;

		// Check prerequeirements
		PAR_ASSERT( true == nvm_is_init());

		// Check NVM signature
		if ( ePAR_OK == par_nvm_check_signature())
		{
			#if ( 1 == PAR_CFG_TABLE_ID_CHECK_EN )

				// Same parameters table
				if ( ePAR_OK == par_nvm_check_table_id())
				{
					// Load all parameters from NVM
					status |= par_nvm_load_all();
				}

				// Parameters table change
				else
				{
					// Clean signature
					status |= par_nvm_clean_signature();

					// Write new table
					status |= par_nvm_write_table_id();

					// Write new header
					status |= par_nvm_write_header();

					// Write signature
					status |= par_nvm_write_signature();

					// Set all parameters to default values
					par_set_all_to_default();

					// Write default values to NVM
					// TODO:
				}

			#else

				// Load all parameters from NVM
				status |= par_nvm_load_all();

			#endif
		}

		// No signature
		else
		{
			#if ( 1 == PAR_CFG_TABLE_ID_CHECK_EN )

				// Write table ID
				status |= par_nvm_write_table_id();

			#endif

			// Write header
			status |= par_nvm_write_header();

			// Lastly write signature
			// NOTE: 	Safety aspect to write signature last. Signature presents some validation factor!
			//			Possible power lost during table ID or header write will not have any side effects!
			status |= par_nvm_write_signature();

			// Set all parameters to default values
			par_set_all_to_default();

			// Write default values to NVM
			// TODO:
		}

		PAR_ASSERT( ePAR_OK == status );

		return status;
	}


	par_status_t par_nvm_write(const par_num_t par_num)
	{
		par_status_t 	status 		= ePAR_OK;
		par_nvm_obj_t	par_obj		= { .u = 0ULL };
		uint32_t		par_addr	= 0UL;

		// Legal call
		PAR_ASSERT( true == par_is_init());
		PAR_ASSERT( par_num < ePAR_NUM_OF )
		PAR_ASSERT( true == par_get_persistance( par_num ))

		// Get current par value
		par_get( par_num, (uint32_t*) &par_obj.field.val );

		// Calculate CRC
		par_obj.field.crc = par_nvm_calc_crc32( par_obj.field.val );

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

		// Legal call
		PAR_ASSERT( true == par_is_init());
		PAR_ASSERT( par_num < ePAR_NUM_OF )
		PAR_ASSERT( true == par_get_persistance( par_num ))

		// Calculate parameter NVM address
		par_addr = (uint32_t)( PAR_NVM_PAR_OBJ_ADDR_OFFSET + ( 4UL * par_num ));

		// Read from NVM
		if ( eNVM_OK != nvm_read( PAR_CFG_NVM_REGION, par_addr, sizeof( par_nvm_obj_t ), (const uint8_t*) &par_obj.u ))
		{
			status = ePAR_ERROR_NVM;
		}

		// Calculate CRC
		calc_crc = par_nvm_calc_crc32( par_obj.field.val );

		// Validate CRC
		if ( calc_crc == par_obj.field.crc )
		{
			par_set( par_num, (uint32_t*) &par_obj.field.val );
		}

		// CRC corrupt
		else
		{
			status = ePAR_ERROR_NVM;
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
		par_status_t status = ePAR_OK;


		return status;
	}


	static par_status_t	par_nvm_write_signature(void)
	{
		par_status_t status = ePAR_OK;


		return status;
	}


	static par_status_t	par_nvm_clean_signature(void)
	{
		par_status_t status = ePAR_OK;


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


	static par_status_t par_nvm_check_header(void)
	{
		par_status_t status = ePAR_OK;


		return status;
	}


	static par_status_t	par_nvm_write_header(void)
	{
		par_status_t status = ePAR_OK;


		return status;
	}




	static uint32_t par_nvm_calc_crc32(const uint32_t val)
	{
		uint32_t crc32 = 0;


		return crc32;
	}

	static par_status_t par_nvm_load_all(void)
	{
		par_status_t 	status 	= ePAR_OK;
		uint32_t 		par_num = 0UL;

		for ( par_num = 0; par_num < ePAR_NUM_OF; par_num++ )
		{
			par_nvm_read( par_num );
		}


		return status;
	}


	////////////////////////////////////////////////////////////////////////////////
	/**
	* @} <!-- END GROUP -->
	*/
	////////////////////////////////////////////////////////////////////////////////

#endif // 1 == PAR_CFG_NVM_EN
