# WIZ550SR
- Serial to Ethernet Module based on W5500 & Cortex-M3
 
- 22(W) x 24mm(L) x 13mm(H)
<!-- WIZ550SR pic -->
![WIZ550SR](http://wizwiki.net/wiki/lib/exe/fetch.php?media=products:wiz550sr:wiz550sr_ds:wiz550sr.png "WIZ550SR")
 
 
## WIZ550SR EVB (Separate purchases)
- WIZ550SR Developer Board.
- USB to UART chip, FT2232D.
- RJ45 with Transformer, RB1-1D5B8K1A.
- RESET Tact SW.
- BOOT0 Tact SW.
- H/W Trig Tact SW.
- LED Indicators.
- Micro USB.
 
<!-- WIZ550SR EVB pic -->
<p align="center">
  <img width="70%" src="http://wizwiki.net/wiki/lib/exe/fetch.php?media=products:wiz550sr:gettingstarted:wiz550sr_evb_1.png" />
</p>
 
For more details, please refer to [WIZ550SR Wiki page](http://wizwiki.net/wiki/doku.php?id=products:wiz550sr:start) in [WIZnet Wiki](http://wizwiki.net).
 
 
## Features
- WIZnet W5500 Hardwired TCP/IP chip
  - Hardwired TCP/IP embedded Ethernet controller
  - SPI (Serial Peripheral Interface) Microcontroller Interface
  - Hardwired TCP/IP stack supports TCP, UDP, IPv4, ICMP, ARP, IGMP, and PPPoE protocols
  - Easy to implement the other network protocols
- ST Microelectronics STM32F103RCT6
  - ARM 32-bit Cortex™-M3 microcontroller running at up to 72MHz
  - 128kB on-chip flash / 20kB on-chip SRAM / Various peripherals
- For more details ( http://wizwiki.net/wiki/doku.php?id=products:wiz550sr:start )
 
 
## Hardware material, Documents and Others
Various materials are could be found at [WIZ550SR Wiki page](http://wizwiki.net/wiki/doku.php?id=products:wiz550sr:start ) in [WIZnet Wiki](http://wizwiki.net).
- Documents
  - Overview
  - Getting Started Guide
  - User's Guide (for AT command & Config Tool)
- Technical Reference (Datasheet)
  - Hardware Specification
  - Electrical Characteristics
  - Reference Schematic & Parts
  - Dimension
- See also 
 
 
## Software
These are Firmware projects (source code) based on Eclipse IDE for C/C++ Developers (ARM GCC 4.8.3).
- Firmware source code
  - Application
  - Boot
- AT command tutorial
  - [AT command tutorial](http://wizwiki.net/wiki/doku.php?id=products:wiz550sr:wiz550sr_tutorial_en)
- Configuration tool (Java)
  - [Download Page Link](http://wizwiki.net/wiki/doku.php?id=products:wiz550sr:wiz550sr_download)
 
 
## Tool & Compiler
- Reference Link:
  - [Eclipse GCC ARM develop environment + OpenOCD for Linux](http://blog.naver.com/opusk/221008480771)
  - [Eclipse GCC ARM develop environment + OpenOCD for Windows](http://blog.naver.com/opusk/221006348299)

- Compiler: gcc-arm-none-eabi-4_8-2014q1-20140314-win32.exe (https://launchpad.net/gcc-arm-embedded/4.8/4.8-2014-q1-update)
- Java: http://www.oracle.com/technetwork/java/javase/downloads/index.html 
- Eclipse: eclipse-cpp-kepler-SR2-win32-x86_64 (http://www.eclipse.org/downloads/packages/eclipse-ide-cc-developers/keplersr2)
- GNU ARM Plugin: https://github.com/gnuarmeclipse/plug-ins/releases/tag/v2.12.1-201604190915 (Just v2.xx, Not v3.xx)
- Make file: http://gnuwin32.sourceforge.net/packages/make.htm   or  Cygwin(https://www.cygwin.com/)
	- Make file need three files: "make.exe", "rm.exe", "echo.exe"
 
 
## Revision History
v1.2.2
- Change prevbuf from pointer array to 2D array
- Change dump from pointer to array

v1.2.1
- AT Command RSP Buffer Size 20 -> 50

v1.1.9
- Add Watchdog Function

v1.1.8
- Fixed Response and debug message error in AT Mode

v1.1.7
- Added features
	- AT+MPASS : (AT command) Change the module's name(automatically saved)
	- AT+MNAME : (AT command) Change the module's password(automatically saved)
- Changed
	- AT+NMODE -> AT+MMODE

v1.1.6
- Fixed Response and debug message error in AT Mode

v1.1.5
- Fixed Problems receiving data in AT Mode

v1.1.4
- Fixed Can not find when the module working in AT Mode
- Fixed UDP Send Fail in AT Mode

v1.1.3
- Fix TCP Server Mode S2E bug.

v1.1.2
- Fixed recevied data loss(During trans to serial) problem in AT Mode

v1.1.1
- Add inactivity time function in Server/Mixed Mode

v1.1.0
- ioLibrary Update
- Remove garbage file

v1.0.0
- First release : 2016-12
