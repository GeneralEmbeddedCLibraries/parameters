# **Device Parameters**

The **Device Parameters** module manages all device parameters through a single configuration table, offering a streamlined approach to system configuration and diagnostics. This module often serves as the backbone of an embedded system, controlling the application's behavior and providing insights into device performance, making diagnostics straightforward and efficient.  

## Key Benefits  
- **Centralized Configuration**: Simplifies the management of system parameters.  
- **Diagnostics**: Enables quick and easy performance analysis of the target device.  
- **Behavior Control**: Directly influences the end behavior of the embedded application.  

## Integration with other modules in General Embedded C Libraries ecosystem
When combined with the following modules, the **Device Parameters** module significantly enhances the capabilities of embedded applications:  

1. **[CLI (Command Line Interface)](https://github.com/GeneralEmbeddedCLibraries/cli)**  
   - Enables communication between the embedded application and a PC.  
   - Allows real-time configuration and diagnostics via the CLI interface.  

2. **[NVM (Non-Volatile Memory)](https://github.com/GeneralEmbeddedCLibraries/nvm)**  
   - Ensures that configured settings are stored persistently.  

3. **Device Parameters + CLI + NVM**  
   - Together, these modules allow:  
     - Communication with a PC using CLI.  
     - Configuration and diagnostics of the application via Device Parameters.  
     - Storage of settings to NVM, ensuring data persistence.  

### Development Benefits  
By leveraging this combination of modules, embedded firmware development becomes significantly faster and more efficient, enabling developers to focus on functionality rather than repetitive low-level implementation.  

## **Dependencies**

### **1. Parameter persistance**

In case of using persistant options (*PAR_CFG_NVM_EN = 1*) it is mandatory to use [NVM module](https://github.com/GeneralEmbeddedCLibraries/nvm).


## **General Embedded C Libraries Ecosystem**
In order to be part of *General Embedded C Libraries Ecosystem* this module must be placed in following path: 
```
root/middleware/parameters/parameters/"module_space"
```

 ## **API**
| API Functions | Description | Prototype |
| --- | ----------- | ----- |
| **par_init** 					| Initialization of parameters module 				| par_status_t par_init(void) |****
| **par_deinit** 				| De-initialization of parameters module 			| par_status_t par_deinit(void) |****
| **par_is_init** 				| Get initialization flag 							| par_status par_is_init(bool * const p_is_init) |
| **par_set** 					| Set parameter 									| par_status_t par_set (const par_num_t par_num, const void *p_val) |
| **par_set_to_default** 		| Set parameter to default value 					| par_status_t par_set_to_default (const par_num_t par_num) |
| **par_set_all_to_default** 	| Set all parameters to default value 				| par_status_t par_set_all_to_default (void) |
| **par_has_changed** 			| Has parameter changed								| par_status_t par_has_changed(const par_num_t par_num, bool *const p_has_changed) |
| **par_get** 					| Get parameter value 								| par_status_t par_get (const par_num_t par_num, void *const p_val)|
| **par_get_id** 				| Get parameter ID number 							| par_status_t par_get_id (const par_num_t par_num, uint16_t *const p_id) |
| **par_get_num_by_id** 		| Get parameter number (enumeration) by its ID 		| par_status_t par_get_num_by_id (const uint16_t id, par_num_t *const p_par_num) |
| **par_get_config** 			| Get parameter configurations 						| par_status_t par_get_config (const par_num_t par_num, par_cfg_t *const p_par_cfg) |
| **par_get_type_size** 		| Get parameter data type size 						| par_status_t par_get_type_size (const par_type_list_t type, uint8_t *const p_size) |
| **par_get_type** 				| Get parameter data type 							| par_status_t par_get_type(const par_num_t par_num, par_type_list_t *const p_type) |
| **par_get_range** 			| Get parameter range 								| par_status_t par_get_range(const par_num_t par_num, par_range_t *const p_range) |


With enable NVM additional fuctions are available:

| API Functions | Description | Prototype |
| --- | ----------- | ----- |
| **par_set_n_save** 	| Set and store parameter to NVM 					| par_status_t par_set_n_save(const par_num_t par_num, const void * p_val) |
| **par_save_all** 		| Store all parameters to NVM 						| par_status_t par_save_all(void) |
| **par_save** 			| Store single parameter 							| par_status_t par_save(const par_num_t par_num) |
| **par_save_by_id** 	| Store single parameter by ID 						| par_status_t par_save_by_id(const uint16_t par_id) |
| **par_save_clean** 	| Re-Write complete NVM memory 						| par_status_t par_save_clean(void) |


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

3. Change parameter configuration table inside **par_cfg.c** file. It is recommended to use designated initializers.

```C
/**
 *	Parameters definitions
 *
 *	@brief
 *
 *	Each defined parameter has following properties:
 *
 *		i)      Parameter ID:   Unique parameter identification number. ID shall not be duplicated.
 *		ii)     Name:           Parameter name. Max. length of 32 chars.
 *		iii)    Min:            Parameter minimum value. Min value must be less than max value.
 *		iv)     Max:            Parameter maximum value. Max value must be more than min value.
 *		v)      Def:            Parameter default value. Default value must lie between interval: [min, max]
 *		vi)     Unit:           In case parameter shows physical value. Max. length of 32 chars.
 *		vii)    Data type:      Parameter data type. Supported types: uint8_t, int8_t, uint16_t, int16_t, uint32_t, int32_t and float32_t
 *		viii)   Access:         Access type visible from external device such as PC. Either ReadWrite or ReadOnly.
 *		ix)     Persistence:    Tells if parameter value is being written into NVM.
 *
 *	@note	User shall fill up wanted parameter definitions!
 */
static const par_cfg_t g_par_table[ePAR_NUM_OF] =
{

	// USER CODE BEGIN...

	// ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	//                   ID         Name                  Min              Max           Def                 Unit         Data type               PC Access                 Persistent		     Description 
	// ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

	[ePAR_TEST_U8] = {	.id = 0,  .name = "Test_u8",  .min.u8 = 0,      .max.u8 = 10,   .def.u8 = 8,       .unit = "n/a", .type = ePAR_TYPE_U8, .access = ePAR_ACCESS_RW,  .persistant = true, .desc = "Test parameter U8" },
	[ePAR_TEST_I8] = {	.id = 1,  .name = "Test_i8",  .min.i8 = -10,    .max.i8 = 100,  .def.i8 = -8,      .unit = "n/a", .type = ePAR_TYPE_I8, .access = ePAR_ACCESS_RW,  .persistant = true, .desc = "Test parameter" },

	[ePAR_TEST_U16] = {	.id = 2,  .name = "Test_u16",  .min.u16 = 0,    .max.u16 = 10,  .def.u16 = 3,      .unit = "n/a", .type = ePAR_TYPE_U16, .access = ePAR_ACCESS_RW, .persistant = true, .desc = "Test parameter U16"},
	[ePAR_TEST_I16] = {	.id = 3,  .name = "Test_i16",  .min.i16 = -10,  .max.i16 = 100, .def.i16 = -5,     .unit = "n/a", .type = ePAR_TYPE_I16, .access = ePAR_ACCESS_RW, .persistant = true, .desc = "Test parameter I16"},

	[ePAR_TEST_U32] = {	.id = 4,  .name = "Test_u32",  .min.u32 = 0,    .max.u32 = 10,  .def.u32 = 10,     .unit = "n/a", .type = ePAR_TYPE_U32, .access = ePAR_ACCESS_RW, .persistant = true, .desc = "Test parameter U32" },
	[ePAR_TEST_I32] = {	.id = 5,  .name = "Test_i32",  .min.i32 = -10,  .max.i32 = 100, .def.i32 = -10,    .unit = "n/a", .type = ePAR_TYPE_I32, .access = ePAR_ACCESS_RW, .persistant = true, .desc = "Test parameter I32" },

	[ePAR_TEST_F32] = {	.id = 6,  .name = "Test_f32",  .min.f32 = -10,  .max.f32 = 100, .def.f32 = -1.123, .unit = "n/a", .type = ePAR_TYPE_F32, .access = ePAR_ACCESS_RW, .persistant = true, .desc = "Test parameter F32" },

	// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------


	// USER CODE END...
};
```

4. Set-up all configurations options inside **par_cfg.h** file (such as using mutex, using NVM, ...)

| Configuration | Description |
| --- | --- |
| **PAR_CFG_MUTEX_EN** 			| Enable/Disable multiple access protection. |
| **PAR_CFG_NVM_EN** 			| Enable/Disable usage of NVM for persistant parameters. |
| **PAR_CFG_NVM_REGION** 		| Select NVM region for Device Parameter storage space. | 
| **PAR_CFG_DEBUG_EN** 			| Enable/Disable debugging mode. | 
| **PAR_CFG_ASSERT_EN** 		| Enable/Disable asserts. Shall be disabled in release build!  | 
| **PAR_DBG_PRINT** 			| Definition of debug print. | 
| **PAR_ASSERT** 				| Definition of assert. | 

5. Call **par_init()** function

```C
// Init parameters
if ( ePAR_OK != par_init())
{
    PROJECT_CONFIG_ASSERT( 0 );
}
```
**NOTICE: NVM module will be initialized as a part of Device Parameters initialization routine in case of usage (*PAR_CFG_NVM_EN = 1*)!**

6. Set up parameter value

For set/get of parameters value always use a casting form!

```C
// Set battery voltage & sytem current
(void) par_set( ePAR_BAT_VOLTAGE, (float32_t*) &g_pwr_data.bat.voltage_filt );
(void) par_set( ePAR_SYS_CURRENT, (float32_t*) &g_pwr_data.inp.sys_cur );

// Set and save parameter 
if ( ePAR_OK != par_set_n_save( ePAR_P1_10, (uint32_t) &p1_10_val ))
{
	// Operation error...
	// Further actions here...
}
```

7. Store to NVM

```C
// Store all paramters to NVM
if ( ePAR_OK != par_save_all())
{
	// Storing to NVM error...
	// Further actions here...
}
```





