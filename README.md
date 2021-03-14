# mgtman
Update of Andrew Collier's SimCoupe .DSK manipulator for SAM Coupé disk images

**Usage

	SAM Coupé .MGT/DSK image manipulator
	------------------------------------

	2021 Hacky Command line remix by Dan Dooré
	Original MAC OS version by Andrew Collier
	Quick and dirty ANSI C port by Thomas Harte
	Command line enhancements by Frode Tennebø for Z88DK

	Usage: mgtman <-h | -d | -w | -r> <mgt-file> [samfile] [options]

	  -h  This help
	  -d  Directory listing of mgt-file
	  -r  Read samfile from mgt-file
	  -w  Write CODE samfile to mgt-file, add [options] if required

	Options:

 	  x   Make CODE file executable at 32768

	Example: mgtman -w test.mgt samfile x

	Filenames to write should conform to Sam conventions (Max: 10 chars, etc.)
	If the mgt-file does not exist it will be created when -w is used.
	
	Why is this called MGTman and not DSKman?
	-----------------------------------------
	DSK files now relate to EDSK format files which are a flexible disk format.
	MGT files are a raw dump of the disk data but the term is still used interchangably
	so most DSK files relating to the SAM Coupé are the raw dump (MGT) format.


**Resources for building

Windows batch file for compiling C with Z88DK, creating MGT file and executing in [SimCoupe](https://github.com/simonowen/simcoupe)

Copy of SAMDOS2 binary for inclusion into MGT images.

helloworld.c for Sam Coupé
