# Changelog
All notable changes to this project/module will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project/module adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

---
## V2.0.0 - 13.02.2023
### Added
 - Added NVM initialization (Issue #19: Add NVM init)
 - Added de-initialization function

### Changed
 - Changed API calls: Get initialization function prototype change

### Fixes
 - Alignment of API calls (Issue #20: Allign all API calls)
 - Fix parameter NVM storage. For flash end memory device there was bug due to not completely re-writing header

---
## V1.3.0 - 28.09.2022
### Added
 - Adding parameter description to configuration table
 - Updated readme

### Fixes
 - Minor comments corrections

---
## V1.2.0 - 23.11.2021
### Added
 - Change storage policy to build consecutive par nvm objects in order to save space and to be back-compatible with older FW
 - Change API functions so that major of functions returns statuses

---
## V1.0.1 - 25.06.2021
### Added
- Added copyright notice

---
## V1.0.0 - 24.06.2021
### Added
- Parameters definitions via config table
- Live values inside RAM 
- Multiple configuration options
- Set/Get parameters functions
- NVM support
- Unique par table ID based on hash functions
- All platform/config dependent stuff are defined by user via interface files
- Parameter storing into fixed predefined NVM memory based only on its ID. 

