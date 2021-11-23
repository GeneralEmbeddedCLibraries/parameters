# **Device parameters**

Parameters module is build for easier configuration and diagnostics of device via communication port of choise. All of the properties of parameters are configurable via single table. 

Parameters module also takes caro of management to NVM space.

## **Dependencies**
--- 

Definition of float32_t must be provided by user. In current implementation it is defined in "project_config.h". Just add following statement to your code where it suits the best.

```C
// Define float
typedef float float32_t;
```

Additionaly module uses "static_assert()" function defined in <assert.h>.

In case of using persistant options for parameters it is mandatory to use [NVM module](https://github.com/GeneralEmbeddedCLibraries/nvm).

 ## **API**
---
| API Functions | Description | Prototype |
| --- | ----------- | ----- |
| **par_init** | Initialization of parameters module | par_status_t par_init(void) |****
| **par_is_init** | Get initialization flag | bool 	par_is_init (void) |
| **par_set** | Set parameter | par_status_t 	par_set (const par_num_t par_num, const void *p_val) |
| **par_set_to_default** | Set parameter to default value | par_status_t 	par_set_to_default (const par_num_t par_num) |
| **par_set_all_to_default** | Set all parameters to default value | par_status_t 	par_set_all_to_default (void) |
| **par_get** | Get parameter value | par_status_t 	par_get (const par_num_t par_num, void *const p_val)|
| **par_get_id** | Get parameter ID number | par_status_t 	par_get_id (const par_num_t par_num, uint16_t *const p_id) |
| **par_get_num_by_id** | Get parameter number (enumeration) by its ID | par_status_t 	par_get_num_by_id (const uint16_t id, par_num_t *const p_par_num) |
| **par_get_config** | Get parameter configurations | par_status_t 	par_get_config (const par_num_t par_num, par_cfg_t *const p_par_cfg) |
| **par_get_type_size** | Get parameter data type size | par_status_t 	par_get_type_size (const par_type_list_t type, uint8_t *const p_size) |


With enable NVM additional fuctions are available:

| API Functions | Description | Prototype |
| --- | ----------- | ----- |
| **par_save_all** | Store all parameters to NVM | par_status_t 	par_save_all (void) |
| **par_save** | Store single parameter | par_status_t 	par_save (const par_num_t par_num) |
| **par_save_by_id** | Store single parameter by ID | par_status_t 	par_save_by_id (const uint16_t par_id) |


## Usage

**Put all user code between sections: USER CODE BEGIN & USER CODE END!**

1. Copy template files to root directory of module.
2. List names of all wanted parameters inside **par_cfg.h** file

```C
/**
 * 	List of device parameters
 *
 * @note 	User shall provide parameter name here as it would be using
 * 			later inside code.
 *
 * @note 	User shall change code only inside section of "USER_CODE_BEGIN"
 * 			ans "USER_CODE_END".
 */
typedef enum
{
	// USER CODE START...

	ePAR_TEST_U8 = 0,
	ePAR_TEST_I8,

	ePAR_TEST_U16,
	ePAR_TEST_I16,

	ePAR_TEST_U32,
	ePAR_TEST_I32,

	ePAR_TEST_F32,

	// USER CODE END...

	ePAR_NUM_OF
} par_num_t;
```

3. Change parameter configuration table inside **par_cfg.c** file

```C
/**
 *	Parameters definitions
 *
 *	@brief
 *
 *	Each defined parameter has following properties:
 *
 *		i) 		Parameter ID: 	Unique parameter identification number. ID shall not be duplicated.
 *		ii) 	Name:			Parameter name. Max. length of 32 chars.
 *		iii)	Min:			Parameter minimum value. Min value must be less than max value.
 *		iv)		Max:			Parameter maximum value. Max value must be more than min value.
 *		v)		Def:			Parameter default value. Default value must lie between interval: [min, max]
 *		vi)		Unit:			In case parameter shows physical value. Max. length of 32 chars.
 *		vii)	Data type:		Parameter data type. Supported types: uint8_t, int8_t, uint16_t, int16_t, uint32_t, int32_t and float32_t
 *		viii)	Access:			Access type visible from external device such as PC. Either ReadWrite or ReadOnly.
 *		ix)		Persistence:	Tells if parameter value is being written into NVM.
 *
 *
 *	@note	User shall fill up wanted parameter definitions!
 */
static const par_cfg_t g_par_table[ePAR_NUM_OF] =
{

	// USER CODE BEGIN...

	// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	//	ID			Name						Min 				Max 				Def 					Unit				Data type				PC Access					Persistent
	// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

	{	.id = 0, 	.name = "Test_u8",	 		.min.u8 = 0,		.max.u8 = 10,		.def.u8 = 8,			.unit = "n/a",		.type = ePAR_TYPE_U8,	.access = ePAR_ACCESS_RW, 	.persistant = true 		},
	{	.id = 1, 	.name = "Test_i8", 			.min.i8 = -10,		.max.i8 = 100,		.def.i8 = -8,			.unit = "n/a",		.type = ePAR_TYPE_I8,	.access = ePAR_ACCESS_RW, 	.persistant = true 		},

	{	.id = 2, 	.name = "Test_u16",	 		.min.u16 = 0,		.max.u16 = 10,		.def.u16 = 3,			.unit = "n/a",		.type = ePAR_TYPE_U16,	.access = ePAR_ACCESS_RW, 	.persistant = true 		},
	{	.id = 3, 	.name = "Test_i16", 		.min.i16 = -10,		.max.i16 = 100,		.def.i16 = -5,			.unit = "n/a",		.type = ePAR_TYPE_I16,	.access = ePAR_ACCESS_RW, 	.persistant = true 		},

	{	.id = 4, 	.name = "Test_u32", 		.min.u32 = 0,		.max.u32 = 10,		.def.u32 = 10,			.unit = "n/a",		.type = ePAR_TYPE_U32,	.access = ePAR_ACCESS_RW, 	.persistant = true 		},
	{	.id = 5, 	.name = "Test_i32", 		.min.i32 = -10,		.max.i32 = 100,		.def.i32 = -10,			.unit = "n/a",		.type = ePAR_TYPE_I32,	.access = ePAR_ACCESS_RW, 	.persistant = true 		},

	{	.id = 6, 	.name = "Test_f32", 		.min.f32 = -10,		.max.f32 = 100,		.def.f32 = -1.123,		.unit = "n/a",		.type = ePAR_TYPE_F32,	.access = ePAR_ACCESS_RW, 	.persistant = true 		},

	// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------


	// USER CODE END...
};
```

4. Set-up all configurations options inside **par_cfg.h** file (such as using mutex, using NVM, ...)

```C
/**
 * 	Enable/Disable multiple access protection
 */
#define PAR_CFG_MUTEX_EN						( 1 )

/**
 * 	Enable/Disable storing persistent parameters to NVM
 */
#define PAR_CFG_NVM_EN							( 1 )

/**
 * 	NVM parameter region option
 *
 * 	@note 	User shall select region based on nvm_cfg.h region
 * 			definitions "nvm_region_name_t"
 *
 * 			Don't care if "PAR_CFG_NVM_EN" set to 0
 */
#define PAR_CFG_NVM_REGION						( eNVM_REGION_EEPROM_RUN_PAR )

/**
 * 	Enable/Disable parameter table unique ID checking
 *
 * @note	Base on hash unique ID is being calculated with
 * 			purpose to detect device and stored parameter table
 * 			difference.
 *
 * 			Must be disabled once the device is release in order
 * 			to prevent loss of calibrated data stored in NVM.
 *
 * 	@pre	"PAR_CFG_NVM_EN" must be enabled otherwise does
 * 			not make sense to calculating ID at all.
 */
#define PAR_CFG_TABLE_ID_CHECK_EN				( 1 )

/**
 * 	Enable/Disable debug mode
 */
#define PAR_CFG_DEBUG_EN						( 1 )

/**
 * 	Enable/Disable assertions
 */
#define PAR_CFG_ASSERT_EN						( 1 )
```

5. Call **par_init()** function

```C
// Init parameters
if ( ePAR_OK != par_init())
{
    PROJECT_CONFIG_ASSERT( 0 );
}
```

In case of using storing parameters to NVM, NVM module must be initialized **before** parameters!

6. Set up parameter value

For set/get of parameters value always use a casting form!

```C
// Set battery voltage & sytem current
par_set( ePAR_BAT_VOLTAGE, (float32_t*) &g_pwr_data.bat.voltage_filt );
par_set( ePAR_SYS_CURRENT, (float32_t*) &g_pwr_data.inp.sys_cur );
```





