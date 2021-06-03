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
* 			Parameter NVM region is further organized into following sections:
*
*			-----------------------------------------------------------------
* 				1. Signature:
* 			-----------------------------------------------------------------
*
* 			Signature tells if NVM "Parameters" region is already initialized
* 			and has some stored parameter NVM objects.
*
*
*			-----------------------------------------------------------------
* 				2. Table ID:
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
*
*			-----------------------------------------------------------------
*				3. Parameters NVM objects
*			-----------------------------------------------------------------
*
*			NVM object consist of parameter value and its 32-CRC checksum. Every
*			time parameter is being read data integrity is being check based
*			on it's checksum.
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
	static uint32_t par_nvm_calc_crc32(const uint32_t val);

	////////////////////////////////////////////////////////////////////////////////
	// Functions
	////////////////////////////////////////////////////////////////////////////////

	par_status_t par_nvm_init(void)
	{
		par_status_t status = ePAR_OK;

		// TODO: Check if NVM is initialized

		// TODO: Check for parameter signature

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

	static uint32_t par_nvm_calc_crc32(const uint32_t val)
	{

	}

#endif // 1 == PAR_CFG_NVM_EN

////////////////////////////////////////////////////////////////////////////////
/**
* @} <!-- END GROUP -->
*/
////////////////////////////////////////////////////////////////////////////////
