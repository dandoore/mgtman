# dskman
Update of Andrew Collier's SimCoupe .DSK manipulator for SAM Coupé disk images

Usage: dskman \<-h | -d | -w | -r\> \<DSK-file\> [samfile] [options]

	-h  This help
  
	-d  Directory listing of DSK-file
  
	-r  Read samfile from DSK-file
  
	-w  Write CODE samfile to DSK-file, add [options] if required

Options:

	x	Make CODE file executable at 32768

Example: dskman -w test.dsk samfile x

Filenames to write should conform to Sam conventions (Max: 10 chars, etc.)

If the DSK-File does not exist it will be created when -w is used.
