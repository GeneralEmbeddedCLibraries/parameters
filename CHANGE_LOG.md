# Changelog
All notable changes to this project/module will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project/module adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

---
   
## V1.3.0 - xx.xx.2022

### Added
 - Adding parameter description to configuration table

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

---