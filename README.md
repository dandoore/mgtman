# mgtman
Update of Andrew Collier's SimCoupe .DSK manipulator for SAM Coupé disk images

**Features**

* File directory of images (SamDOS, MasterDOS, BDOS, GDOS, G+DOS & UNI-DOS)
* Reading and writing of CODE files (SamDOS, MasterDOS, BDOS)
* Writing of SCREEEN$ files (SamDOS, MasterDOS, BDOS)
* Write disk title (MasterDOS/BDOS/UNI-DOS)
* Wrting of BASIC files (in progress)

**Resources for using with Z88DK**

Files contained in /resources/ are to allow for compiling in Z88DK, creating an MGT file and executing in [SimCoupé](https://github.com/simonowen/simcoupe) now mostly depricated due to sterling work over at Z88DK adding [MGT Support](https://github.com/z88dk/z88dk/commit/fa1f1b45901e4412f190353647667192b4c2e61b)
* Windows batch file for compiling and executing in SimCoupé
* Copy of SAMDOS2 binary for inclusion into MGT images
* helloworld.c for Sam Coupé for testing
