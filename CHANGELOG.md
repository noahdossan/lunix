# Lunix Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Added
- Improved kernel panic info
- Support for python modules

### Changed
- Kernel panic bg color from red to blue

### Fixed

### Removed
- Lulu easter egg (sad)

## [0.3.0] - 07-30-24

### Added
- `panic` command to panic the system for testing
- Added a codename to the kernel (Nova)
- Added build date
- Added file protection against non-root users
- Password login attempts (3 times)

### Changed
- Changed the prompt to look better
- Renamed kernel_panic.log and passwords.txt to .passwd and kernel.log
- Help command is now a lot shorter and readable, use man to get command details
- Ping 8.8.8.8 instead of github.com

### Fixed
- Fixed the prompt to show the correct path (~ instead of full/path/to/rootfs)
- Made the disk library catch directory exceptions

## [0.2.0] - 07-24-24

### Added
- User login management

## [0.1.0] - 07-23-24

The first release of Lunix.

### Added
- Initial release of Lunix
