# mgtman
Update of Andrew Collier's SimCoupe .DSK manipulator for SAM Coupé disk images

Please only use released binaries, other binaries should be considered alpha and subject to change.

**Help**
        
	SAM Coupé .MGT/DSK image manipulator v2.1.0
	-------------------------------------------
	2021 Hacky command line remix by Dan Dooré
	Original MAC OS version by Andrew Collier
	Quick and dirty ANSI C port by Thomas Harte
	Command line enhancements by Frode Tennebo for Z88DK
	
	https://github.com/dandoore/mgtman/
	
	Usage: mgtman_win32 [-h] [-d <mgt-file>] [-t <mgt-file> <title-name>] [-w | -r <mgt-file> <samfile> [start-address] [execute-address]]
	
	  -h  This help
	  -d  Directory listing of mgt-file
	  -t  Title (change disk name) mgt-file with title-name where supported by the disk format
	  -r  Read samfile from mgt-file
	  -w  Write CODE samfile to mgt-file (create MGT file if not existing)
	
	Variables:
	
	   mgt-file         MGT image disk file (can be new file when using -w)
	   title-name       Disk title to write to mgt-file (Max 10 chars, 7-bit ASCII)
	   samfile          Code filename on mgt-file or file system (Max 10 chars, 7-bit ASCII)
	   start-address    When writing to mgt-file code load start address (default 32768, >=16384)
	   execute-address  When writing to mgt-file code execute address (default none, >=16384))

	Examples:

	   Directory of disk image:    mgtman_win32 -d test.mgt
	   Write auto-executing file:  mgtman_win32 -w test.mgt auto.cde 32768 32768
	   Read file from disk image:  mgtman_win32 -r test.mgt file.c
	   Change title of disk image: mgtman_win32 -t test.mgt mydisk

	Why is this called MGTman and not DSKman?
	-----------------------------------------
	DSK files now relate to EDSK format files which are a flexible disk format.
	MGT files are an 819,200Kb dump (RAW/IMG) of the disk data but the term is still used
	interchangably so most DSK files relating to the SAM Coupe are the 819,200Kb (MGT)
	format. Genuine (E)DSK files have an EDSK header and flexible file size.

**Resources for building**

Files contained in /resources/ are to allow for compiling in Z88DK, creating an MGT file and executing in [SimCoupé](https://github.com/simonowen/simcoupe) now depricated due to sterling work over at Z88DK adding [MGT Support](https://github.com/z88dk/z88dk/commit/fa1f1b45901e4412f190353647667192b4c2e61b)
* Windows batch file for compiling and executing in SimCoupé
* Copy of SAMDOS2 binary for inclusion into MGT images
* helloworld.c for Sam Coupé for testing
