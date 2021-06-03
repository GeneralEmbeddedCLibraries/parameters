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
*/
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////////////////
#include "par_nvm.h"
#include "../../par_cfg.h"
#include "../../par_if.h"

#if ( 1 == PAR_CFG_NVM_EN )

	////////////////////////////////////////////////////////////////////////////////
	// Definitions
	////////////////////////////////////////////////////////////////////////////////

	/**
	 * 		Parameter NVM object
	 *
	 * 	@note 	Each parameter is protected with 32-CRC.
	 */
	typedef struct
	{
		uint32_t 	val;	/**<4-byte storage for parameter value */
		uint32_t	crc;	/**<32-CRC of parameter value */
	} par_nvm_obj_t;

	////////////////////////////////////////////////////////////////////////////////
	// Variables
	////////////////////////////////////////////////////////////////////////////////


	////////////////////////////////////////////////////////////////////////////////
	// Function Prototypes
	////////////////////////////////////////////////////////////////////////////////
	static par_status_t		par_nvm_check_signature	(void);
	static par_status_t		par_nvm_write_signature	(void);
	static par_status_t 	par_nvm_check_table_id	(void);
	static uint32_t 		par_nvm_calc_crc32(const uint32_t val);

	////////////////////////////////////////////////////////////////////////////////
	// Functions
	////////////////////////////////////////////////////////////////////////////////

	par_status_t par_nvm_init(void)
	{
		par_status_t 	status 		= ePAR_OK;
		par_cfg_t *		p_par_table = NULL;

		// TODO: Check if NVM is initialized

		// Check NVM signature
		if ( ePAR_OK == par_nvm_check_signature())
		{
			// Rewrite everythink: signature, table ID,...
			// NOTE: Signature must be written last!!! As this will reduces problems with interrupt of power supply in between NVM write operation!
		}

		// No signature
		else
		{
			par_nvm_write_signature();
		}

		// TODO: Calculate and compare table ID


		// TODO: Read & set individual paramter from NVM if signature is OK and table ID is the same

		return status;
	}

	par_status_t par_nvm_write(const par_num_t par_num)
	{
		par_status_t status = ePAR_OK;

		// TODO: Assemble par nvm object
		// TODO: Calculate CRC

		// TODO: Write to NVM

		return status;
	}

	par_status_t par_nvm_read(const par_num_t par_num)
	{
		par_status_t status = ePAR_OK;

		// TODO: Read par nvm object
		// TODO: Calculate CRC
		// TODO: Validate CRC


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

	static uint32_t par_nvm_calc_crc32(const uint32_t val)
	{
		uint32_t crc32 = 0;


		return crc32;
	}


	////////////////////////////////////////////////////////////////////////////////
	/**
	* @} <!-- END GROUP -->
	*/
	////////////////////////////////////////////////////////////////////////////////

#endif // 1 == PAR_CFG_NVM_EN
