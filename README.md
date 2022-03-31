[![Donate](https://img.shields.io/badge/Donate-PayPal-green.svg)](https://www.paypal.com/cgi-bin/webscr?cmd=_s-xclick&hosted_button_id=NF3FBD3KHMXDN)
[![GitHub tag (latest SemVer)](https://img.shields.io/github/tag/horsicq/xntsv.svg)](https://github.com/horsicq/xntsv/releases)
[![GitHub All Releases](https://img.shields.io/github/downloads/horsicq/xntsv/total.svg)](https://github.com/horsicq/xntsv/releases)
[![gitlocalized ](https://gitlocalize.com/repo/4736/whole_project/badge.svg)](https://github.com/horsicq/XTranslation)

XNTSV program for detailed viewing of system structures in Windows.

During process creation in Windows, special system structures appear in the system such as:

* [PEB](https://en.wikipedia.org/wiki/Process_Environment_Block)
* TEB *
* PEB_LDR_DATA
* LDR_DATA_TABLE_ENTRY
* RTL_USER_PROCESS_PARAMETERS
* EPROCESS etc.

The list of different structures is large and specific for each version of the operating system.

**You could easily add your oun structs (Edit structs/ARCH/custom.json)**

Officially it is not fully documented and change in different Windows versions.

This program shows complete information about these structures. 
It can be useful for researchers of Windows internals, as well as creators of software protection.

The program supports now:

* Windows 7
* Windows 7 SP1
* Windows 8
* Windows 8.1
* Windows Server 2016
* Windows Server 2019
* Windows 10 (all builds)
* Windows 11 (build 22000)

**The program does not support now Windows 2000,XP and Vista.** If you need structs for these OS use old versions of XNTSV.

**The program supports now kernel mode.** But you need sign driver with valid driver cert or use test cert. https://docs.microsoft.com/en-us/windows-hardware/drivers/install/how-to-test-sign-a-driver-package

* Download: https://github.com/horsicq/xntsv/releases
* How to run: https://github.com/horsicq/xntsv/blob/master/docs/RUN.md
* How to build: https://github.com/horsicq/xntsv/blob/master/docs/BUILD.md
* Changelog: https://github.com/horsicq/xntsv/blob/master/changelog.txt

![alt text](https://github.com/horsicq/xntsv/blob/master/mascots/xntsv.png "Mascot")

![alt text](https://github.com/horsicq/xntsv/blob/master/docs/1.png "1")
![alt text](https://github.com/horsicq/xntsv/blob/master/docs/2.png "2")
![alt text](https://github.com/horsicq/xntsv/blob/master/docs/3.png "3")
![alt text](https://github.com/horsicq/xntsv/blob/master/docs/4.png "4")
![alt text](https://github.com/horsicq/xntsv/blob/master/docs/5.png "5")
