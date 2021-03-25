// Sam Coupe MGT/DSK Manipulator 2.1.0
//
// 		Hacky command line remix by Dan Dooré 
//		1.0.0 MacOS by Andrew Collier
// 		Quick and dirty ANSI C port by Thomas Harte!!
// 		Command line enhancement by Frode Tennebø for z88dk
//
// https://github.com/dandoore/mgtman/

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

// Prototypes

void Savemgt(char *mgtimage);
void Openmgt(char *mgtimage);
void DetectFormat();
unsigned char *Addr(int track,int sector,int offset);
void SaveFile(char *fname, int start, int exec);
void LoadFile(char *filename);
void TitleDisk(char *diskname);
void Directorymgt(void);
void Usage(char *exename);
void Help(char *exename);

// Global vairables

int validmgt,format;				// Status and format of MGT image in memory
unsigned char	*image,*valid;		// MGT image in memory

// Main entry point with arguments

int main(int argc, char **argv)
{
int	start,exec;
int t;

/* Arguments debug

	int i=0;
	printf("ARGC: %u\n",argc);
	for (i=0; i<=argc; i++)
		{
		printf("ARG%u: %s\n",i,argv[i]);
		}
*/
	if ((image = malloc(819200)) == (unsigned char *) NULL)
	{
		printf("Not enough free memory to load .MGT file\n");
	    exit(1);
	}
	else
	{
		if (argc < 2)
		{
		     Usage(argv[0]);
		     exit(1);
		}
		if (argc >= 2 )
			{
		        if (argv[1][0] == '-')
				{
					if (argv[1][1] == 'h')
						{
						 Help(argv[0]);
						 exit(0);
						}
					if (argv[1][1] == 'd')
						{
						 Openmgt(argv[2]);
						 Directorymgt();
						 exit(0);
						}
					if (argv[1][1] == 'w')
						{
						 Openmgt(argv[2]);
						 
						 // This bit feels very hacky, must be an easier way to parse arguments of a variable number
						 
						 if (argc >= 5)
							{
							start = atoi(argv[4]);
							if (start <16384)			// Files cannot be loaded in page 0 (0-16383) as DOS is paged in here at the time
									{
									printf("Start address out of range\n\n");
									Help(argv[0]);
									exit(1);
									}
							}
							else
							{
							start = 32768;
							};
							
						 if (argc >= 6)
							{
							exec = atoi(argv[5]);
							if (exec <16384)			// Same applies for execute address as start address
									{
									printf("Execute address out of range\n\n");
									Help(argv[0]);
									exit(1);
									}
							}
							else
							{
							exec = 0;	// 0 is not valid as an exec address, these must start in section C (0x8000)
							};
						 SaveFile(argv[3],start,exec);	
						 Savemgt(argv[2]);
						 exit(0);
						}
					if (argv[1][1] == 'r')
						{
						 Openmgt(argv[2]);
						 LoadFile(argv[3]);
						 exit(0);
						}
					if (argv[1][1] == 't')
						{
						 Openmgt(argv[2]);
						 TitleDisk(argv[3]);
						 Savemgt(argv[2]);
						 exit(0);
						}
					else	// Fallback for invalid argument
						{
						Usage(argv[0]);
						exit(1);
						};
				}
			}
	};
	return 0;
}

// Usage text

void Usage(char *exename)
{
	printf("\nUsage: %s [-h] [-d <mgt-file>] [-t <mgt-file> <title-name>] [-w | -r <mgt-file> <samfile> [start-address] [execute-address]]\n\n",exename);
	printf("For help page: %s -h  \n",exename);
}

// Help text

void Help(char *exename)
{
	printf("        ,\n");
	printf("SAM Coupe .MGT/DSK image manipulator v2.1.0\n");
	printf("-------------------------------------------\n");
	printf("                                         ,\n");
	printf("2021 Hacky command line remix by Dan Doore\n");
	printf("Original MAC OS version by Andrew Collier\n");
	printf("Quick and dirty ANSI C port by Thomas Harte\n");
	printf("Command line enhancements by Frode Tennebo for Z88DK\n\n");
	printf("https://github.com/dandoore/mgtman/\n\n");
	printf("Usage: %s [-h] [-d <mgt-file>] [-t <mgt-file> <title-name>] [-w | -r <mgt-file> <samfile> [start-address] [execute-address]]\n\n",exename);
	printf("  -h  This help\n");
	printf("  -d  Directory listing of mgt-file\n");
	printf("  -t  Title (change disk name) mgt-file with title-name where supported by the disk format\n");
	printf("  -r  Read samfile from mgt-file\n");
	printf("  -w  Write CODE samfile to mgt-file (create MGT file if not existing)\n");
	printf("\nVariables:\n\n   mgt-file         MGT image disk file (can be new file when using -w)\n");
	printf("   title-name       Disk title to write to mgt-file (Max 10 chars, 7-bit ASCII)\n");
	printf("   samfile          Code filename on mgt-file or file system (Max 10 chars, 7-bit ASCII)\n");
    printf("   start-address    When writing to mgt-file code load start address (default 32768, >=16384)\n");
	printf("   execute-address  When writing to mgt-file code execute address (default none, >=16384))\n\n");
	printf("Examples:\n\n   Directory of disk image:    %s -d test.mgt\n",exename);
	printf("   Write auto-executing file:  %s -w test.mgt auto.cde 32768 32768\n",exename);
	printf("   Read file from disk image:  %s -r test.mgt file.c\n",exename);
    printf("   Change title of disk image: %s -t test.mgt mydisk\n\n",exename);
	printf("Why is this called MGTman and not DSKman?\n");
	printf("-----------------------------------------\n");
	printf("DSK files now relate to EDSK format files which are a flexible disk format.\n");
	printf("MGT files are an 819,200Kb dump (RAW/IMG) of the disk data but the term is still used\n");
	printf("interchangably so most DSK files relating to the SAM Coupe are the 819,200Kb (MGT)\n");
	printf("format. Genuine (E)DSK files have an EDSK header and flexible file size.\n");
}

// Open MGT file and copy contents to RAM in image

void Openmgt(char *mgtimage)
{
int		i;
FILE	*mgt;

	mgt = fopen(mgtimage, "rb");

	if (mgt != NULL)
	{
		fseek (mgt, 0, 2);
		if (ftell (mgt) == 819200)
		{
			fseek (mgt, 0 , 0);
			if (fread (image, (size_t) 1, (size_t) 819200, mgt) == 819200)
			{
				validmgt = 1;
				DetectFormat();
			}
			else
			{
				printf("Read fault on .MGT file: %s\n",mgtimage);
				exit(1);
			};
		}
		else
		{
			printf("%s is not a valid .MGT file\n",mgtimage);
			exit(1);
		};
		fclose (mgt);
	}
	else
	{
		validmgt = 0;
	};
}

// Detect format of the RAM Image

void DetectFormat()
{
int	i;
unsigned char	*found;

		if (validmgt == 0)
		{
			printf("MGT file not found");
			exit(1);
		}	
		
		// Detect Format from first directory entry
		//
	    // Format 0 = SamDOS, 1 = MasterDOS, 2 = BDOS, 3=GDOS(DISCiPLE)/G+DOS(PlusD), 4=Uni-DOS

		found = Addr(0,1,0);	// First sector, first directory entry
		
		format = 0;
		if ( *(found+255) == 255) format = 0;												// SamDOS format 
		if ( *(found+252) != 0 && *(found+253) != 0 && *(found+252) != 32) format = 1;		// MasterDOS format 
		if ( *(found+255) == 32) format = 2;												// BDOS format 

		if ( (*(found) & 63) >= 1 &&  (*(found) & 63) <= 13)	// If first file entry is a ZX File type
			{
			format=3;		// GDOS(DISCiPLE)/G+DOS(PlusD)
			if (*(found+244) != 0) format = 4;	//Uni-DOS marker
			}
			
		/* Debug - print first entry values
		
		for (i=0; i<=255; i++)
			{
			if (i<15 || i>209) printf("%u: %u %c\n",i,*(found+i),*(found+i));
			}
		printf("\nFormat: %u\n",format);
			//*/
}

// Write out RAM image as MGT file

void Savemgt(char *mgtimage)
{
FILE	*mgt;

	mgt = fopen(mgtimage, "wb");
	if (mgt != NULL)
	{
		if (fwrite (image, (size_t) 1, (size_t) 819200, mgt) != 819200)
		{
			printf("Save fault on .MGT file: %s\n",mgtimage);
			exit(1);
		}
		fclose(mgt);
	}
	else
		{
			printf("Could not open MGT file %s for saving\n",mgtimage);
			exit(1);
		};
}

// Read data from RAM image based on track/sector/offset

unsigned char *Addr(int track,int sector,int offset)
{
unsigned char	*a;

	if (track < 80)
	{
		a = image + 10240*track + 512*(sector-1) + offset;
	}
	else
	{
		a = image + 5120 + 10240*(track - 128) + 512*(sector-1) + offset;
	};

	return a;
}

// Add file to RAM image

void SaveFile(char *filename, int start, int exec)
{
unsigned char 	sectmap[195],usedmap[195],*found,*exists,samfile[10];
int	filelength,maxdtrack,s,t,h,i,m,a,tt,ss;  
FILE	*file;
		
		// If no existing disk file existed, create blank SAMDOS image.
		
		if (validmgt == 0)
			{		
			for (i = 0; i<819200; i++)
				{
				*(image+i) = (unsigned char)0;
				}
			format=0;
		}
		
		if (format >= 4)
			{
			printf("Sorry, file operations only supported on SAM Coupe formats right now.\n");
			exit(1);
			}
		
		file = fopen(filename, "rb");
		if (file != NULL)
	    {
	    // Search image to check that filename is not already taken	
		
		// Make samfile from filename to 10 chars with spaces
	
		i=0;
		for (i=0; i<=9; i++)
		{
			if (i < strlen(filename))
			{
			samfile[i]=filename[i];
			}
			else
			{
			samfile[i]=' ';
			}
		}
		samfile[10]=0;
				
		exists = NULL;

		// Number of directory tracks to scan

		if ( *(image+255) == 255)	//SamDOS format
		{
			maxdtrack = 4;
		}
		else
		{
			maxdtrack = 4 + *(image+255);  // MasterDOS additional directory tracks
		}

		// Check directory for file
		
		for (t=0; t<maxdtrack; t++)
		{
			for (s=1; s<11; s++)
			{
				for (h=0; h<2; h++)
				{
					for (i=0; i<10; i++)
					{
						if (*Addr(t,s,256*h) != 0)  // If file is not deleted
						{
							if ( toupper (*Addr(t,s,256*h+1+i)) != toupper(samfile[i]))
							{
								i = 10;
							}
							else if (i==9)
							{
								exists = Addr(t,s,256*h);
								i=10;
								h=2;
								s=10;
								t=maxdtrack;
							};
						};
					};
				};
			};
		};
		
		if (exists != NULL)
		{
			printf("File %s already exists in MGT\n",filename);
			exit(1);
		}
			
		// Allocate BAM sector map for file in directory entry
		
		fseek (file, 0, 2);
		filelength = ftell(file);
		fseek (file,0,0);

		if ( *(image+255) == 255)
		{
			maxdtrack = 4;
		}
		else
		{
			maxdtrack = 4 + *(image+255);
		};

		for (i=0; i<195; i++)
		{
			sectmap[i] = usedmap[i] = 0;
		};

		for (t=0; t<maxdtrack; t++)
		{
			for (s=1; s<11; s++)
			{
				for (h=0; h<2; h++)
				{
					for (i=0; i<195; i++)		// Pre-populate sector map
					{
						sectmap[i] |= *Addr(t,s,256*h+15+i);
					};
				};
			};
		};

		if (maxdtrack>4)
		{
			sectmap[0] &= 254;
			sectmap[1] &= 3;

			if (maxdtrack>5)
			{
				a=1;
				m=4;

				for (i=0; i<10*(maxdtrack - 4); i++)
				{
					sectmap[a] &= m;

					m *=2;
					if (m==256)
					{
						a ++;
						m = 1;

					}

				};
			}
		}

		for (t=0; t<maxdtrack; t++)
		{
			for (s=1; s<11; s++)
			{
				for (h=0; h<2; h++)
				{

					if (*Addr(t,s,256*h) == 0)
					{
						found = Addr(t,s,256*h);
						h=2;
						s=10;
						t=maxdtrack;
					}

				};
			};
		};

		// Find free space on image

		i=0;
		for (a=0;a<195;a++)
		{
			for(m=1;m<256;m *=2)
			{
				i += !(sectmap[a] && m);

			};
		};

		if (filelength > (510*i - 9))
		{
			printf("Sorry, Not enough space on disk\n");
			exit(1);
		}
		else
		{
			t=4;
			s=1;
			m=1;
			a=0;

			while ((sectmap[a] & m))
			{
				m *= 2;
				if (m==256)
				{
					m =1;
					a ++;
				}

				s++;
				if (s==11)
				{
					s=1;
					t++;

					if (t==80)
						t=128;
				}
			}
			
			// Build directory and File entry
			
			i=0;
			*Addr(t,s,0) = 19;			// Status - set type as CODE in 9 byte File Header
			
			/*
			Byte 	SAMDOS type 	Plus D type
			-----------------------------------
			0 		File type 		File type
			1-2 	Modulo length 	Length of file
			3-4 	Offset start 	Start address
			5-6 	Unused 	 
			7 		Number of pages 	 
			8 		Starting page number
			
			Modulo Length & Number of Pages

			In the SAMDOS header the length of the file is calculated by multiplying the number of pages (byte 7) by 16384 and adding the modulo length (word 1-2), LSB/MSB, ie the length MOD 16Kb (%16384)

			Offset Start & Starting Page Number

			Read starting page number (byte 8). AND this with 1FH to get the page number in the range 0 to 31. To find the start, multiply the page number by 16384, add the offset and subtract 4000H (since the ROM occupies 0-3FFFH).

			When SAMDOS is paged in it resides at 4000H, and ROM0 is placed at 0-3FFFH.
			*/
			
			*(found) = 19;				// Status - set type as CODE in Diretory Entry
		
			
			/*
			SAMDOS Directory

			The first 4 tracks of the disk are allocated to the disk directory, starting at track 0, sector 1. These 4 
			tracks give us 40 sectors each split into two 256 bytes entries. Each of these entries will identify one file,
			thus allowing up to 80 entries in the directory.

			The format of each directory entry is as follows:
			Byte 	UIFA 	Desctiption
			0 		0 		Status/File type. This byte is allocated on of the file types listed previously, but is also used
							as a file status. If the byte is 0 then then file has been erased. If the file is HIDDEN the bit 7
							is set. If the file is Protected then bit 6 is set.
			1-10 	1-10 	Filename. This filename can be up to 10 characters.
			11 	 		 	MSB of the number of sectors used in the file.
			12 	  			LSB of the number of sectors used in the file.
			13 	  			Track number for start of file.
			14 	  			Sector number for start of file.
			15-209 		  	Sector address map (195 bytes) (detailed further on).
			210-219 	  	MGT Future and past (10 bytes). These were used in the PLUS D directory but are not used by the SAMDOS.
							They are allocated to MGT for future use.
			220 	15 		Flags (MGT use only).
			221-231 	  	File type information
					16-26 	If the file type is 17 or 18 then these bytes contain the file type/length and name.
					16 		If the file type is 20 then these bytes contain the screen mode.
					16-18 	If the file type is 16 then these bytes contain the program length excluding variables.
					19-21 	If the file type is 16 then these bytes contain the program length plus numeric variables.
					22-24 	If the file type is 16 then these bytes contain the program lengtrh plus numeric variables
							and the gap length before string and array variables.
			232-235 27-30 	Spare 4 bytes (reserved).
			236 	31 		Start page number, in bits 4-0, bits 7-5 are undefined.
			237-238 32-33 	Page offset (8000H-BFFFH). This is as per file header, although when the ROM passes a file
							to be saved, it starts it in section C of the addressing map.
			239 	34 		Number of pages in length (as per file header).
			240-241 35-36 	Modulo 0 to 16383 length, ie length of file MOD 16384 (as per file header).
			242-244 37-39 	Execution address. Execution address, if CODE file, or line number if an autorunning BASIC program.
			245-253 40-47 	Spare 8 bytes.
			254-255 	  	For future use by MGT only.
			
			MASTERDOS DIRECTORY ENTRY FORMAT
			This information is intended for those interested in using READ AT and WRITE at to deal directly with the disk directory when writing utility programs.

			The first tracks on side 1 of a disk are used for the directory, starting at track 0, sector 1. Each sector holds two 256-bytem directory entries. The format of each entry is as follows:

			Byte	Description
			0 	Status/File type, as returned by the FSTAT function with a parameter of 4. Zero if entry unused or ERASEd. 
			1	(10 bytes) File name. If the first byte is zero, the entry is assumed to have never been used and directory reading will not proceed further.
			11	MSB of number of sectors used in the file.
			12	LSB of number of sectors used in the file.
			13	Track number of start of file.
			14	Sector number of start of file.
			15	(195 bytes) Sector usage map for the file. Bit 0 of the first byte represents track 4, sector 1, bit 1 represents sector 2, etc. If a bit is set the sector is used by the file.
			210	(10 bytes) In all entries except the first one on the disk, these bytes hold Plus D/Disciple-form information. In the first entry, the disk name is stored here. Bit 7 of byte 210 can be set or reset without altering the disk name.
			220	Flags
			221	(10 bytes) File-type specific information. If the file type is SCREEN$ then byte 221 is the screen MODE, minus 1.
			232	(4 bytes) Reserved.
			236	Start page number in bits 4-0, bits 7-5 are undefined.
			237	(2 bytes) Start offset in the range 32768-49151.
			239	Number of pages in length.
			240	(2 bytes) Length MOD 16384. Bits 15 and 14 are undefined.
			242	Execution page for CODE files, or 255
			243	(2 bytes) Execution offset (32768-49151) for CODE files, or auto-run line for a Basic program.
			245	Day of SAVE, or 255
			246	Month of SAVE
			247	Year of SAVE 248 Hour of SAVE
			249	Minute of SAVE
			250	For a DIR file, code number tagging files in that subdirectory.
			251	Reserved.
			252-253	(2 bytes) In the first directory entry only, Random word identifying the disk.
			254	Subdirectory code number of this file.
			255	In first entry only, number of directory tracks, minus 4.
			
			GDOS/G+DOS
			==========

			Now we will see the details about a single directory entry.
			NOTE: All numbers are in decimal.

			General structure:

			OFFSET   MEANING
			  0      File type (see FILE-TYPES table); 0 = erased (free entry)
			 1-10    Filename (padded with spaces)
			11-12    Number of sectors occupated by the file (in Motorola byte order)
			 13      Track number of the first sector of the file
			 14      Sector number of the first sector of the file
			15-209   Sector allocation bitmap. Each bit corresponds to a disk sector. 
					 A bit is set if the corresponding sector belong to the file.
					 Examples: byte 15, bit 0 corresponds to track 4, sector 1; 
					 byte 16, bit 3 means track 5, sector 2...
					 IMPORTANT NOTE:
					 The s.a.b. is used only during saving operations: the s.a.b. of
					 all the 80 files are merged together (OR) so that the system
					 knows which sectors are free (not allocated to any file).
					 During loading a faster method is used: each sector contains
					 only 510 bytes of data; the last two bytes contain the
					 track number and the sector number of the next sector of the file,
					 respectively. The last sector of the chain contains (0,0) as the
					 last two bytes.

			210-255  Depend on the file type.

			BASIC (type 1)
			---------------
			211      Always 0 (this is the id used in tape header)
			212-213  Length
			214-215  Memory start address ( PROG when loading - usually 23755)
			216-217  Length without variables
			218-219  Autostart line
			NOTE: These 9 bytes are also the first 9 bytes of the file.

			NUMBER ARRAY (type 2)
			---------------------
			211      Always 1 (this is the id used in taper header)
			212-213  Length
			214-315  Memory start address.
			216-217  Array name, probably ignored.
			218-219  Not used
			NOTE: These 9 bytes are also the first 9 bytes of the file.

			STRING ARRAY (type 3)
			---------------------
			211      Always 2 (this is the id used in tape header)
			212-219  Same as for type 2
			NOTE: These 9 bytes are also the first 9 bytes of the file.

			CODE FILE (type 4)
			------------------
			211      Always 3 (this is the id used in tape header)
			212-213  Length
			214-315  Start address
			216-217  Not used
			218-219  Autorun address (0 if there is no autorun address)

			48K SNAPSHOT (type 5)
			---------------------
			211-219  Not used

			MDRV (type 6)
			-------------
			This is a microdrive cartridge image. Details omitted.
			NOTE: UNIDOS mdrv files are completely different from GDOS ones.

			SCREEN$ (type 7)
			----------------
			Same as type 4 with Start=16384 and Length=6912

			SPECIAL (type 8)
			----------------
			211-255  Any meaning assigned by the programmer.

			128K SNAPSHOT (type 9)
			----------------------
			Same as 48K Snapshot. The first byte of the file is a copy of the page
			register (port 0x7FFD), usually held in the system variable BANKM (23388).
			The 8 RAM pages are saved in ascending order from 0 to 7.

			OPENTYPE (type 10)
			------------------
			210     Number of 64K blocks in the file
			211     Always 9 (not sure)
			212-213 Length of the last block 
			214-255 Not used

			NOTE: Opentype files can be more than 64K in length and are usually created
			and handled with the stream-related BASIC statements, such as OPEN #, CLOSE #,
			PRINT #, INPUT # and so on.
			See chapter [9] for a brief description of these statements.

			EXECUTE (type 11)
			-----------------
			210-255 Same as CODE file (type 4), but Length=510 and Start=0x1BD6 implicitly
					(0x3DB6 for +D). The sector is loaded into the interface RAM
					and executed (it should contain relocatable code!).

			SUBDIRECTORY (type 12) - UNIDOS
			-------------------------------
			210-212 Same as Opentype (type 10). This file is always held on contiguous
					sectors. The last two bytes of a sector do not contain the address
					of the next sector. The structure is the same as the root directory,
					but the first entry contains the file header number of the parent
					directory. The last two bytes of the last sector contain 0xFFFF.
			213     Capacity (number of file entries allowed).

			CREATE (type 13) - UNIDOS
			-------------------------
			210-255 Same as CODE file but the start address is ignored.
						
			*/
			
			// Pad filename to 10 chars with spaces for writing to SAM directory
			i=0;
			for (i=strlen(filename); i<10; i++)
			{
				filename[i]=' ';
			};

			// Write filename to Diretory Entry
			for (i=0; i<10; i++)
			{
				*(found+1+i) = filename[i];
			};

			i = (filelength+9)/510;
			*(found+11) = i/256;	//MSB sectors used
			*(found+12) = i%256;	//LSB sectors used
			*(found+13) = t;		// Start Track
			*(found+14) = s;		// Start sector
			*(found+220) = 0;		// Flags blank
			
			// Generate Start Address for File and Dir
						
			*(found+236) = (start/16384)-1;	
			*Addr(t,s,8) = (start/16384)-1;
			*(found+237) = start%256;
			*Addr(t,s,3) = start%256;			
			*(found+238) = (start-(16384*((start/16384)-2)))/256;
			*Addr(t,s,4) = (start-(16384*((start/16384)-2)))/256;
			
			// printf("\nPage: %u\nLSB: %u\nMSB: %u\n\n",(start/16384)-1,start%256,(start-(16384*((start/16384)-2)))/256);
			
			// Generate File Length for File and Dir

			*(found+239) = filelength/16384;
			*Addr(t,s,7) = filelength/16384;
			*(found+240) = filelength%256;
			*Addr(t,s,1) = filelength%256;
			*(found+241) = (filelength%16384)/256;
			*Addr(t,s,2) = (filelength%16384)/256;

			// Generate Exec for directory only
			
			if (exec == 0)
				{
					*(found+242) = 255;
					*(found+243) = 255;
					*(found+244) = 255;
				}
				else
				{
					*(found+242) = exec/16384;	// Page
					*(found+243) = exec%256;	// LSByte
					*(found+244) = (exec-(16384*((exec/16384)-2)))/256;		// MSByte 
				}
						
			if (filelength < 502)
			{
				fread(Addr(t,s,9),1,filelength,file);
				*Addr(t,s,510) = 0;
				*Addr(t,s,511) = 0;

				usedmap[a] |= m;
				sectmap[a] |= m;
			}
			else
			{
				fread(Addr(t,s,9),1,501,file);
				filelength -= 501;
				usedmap[a] |= m;
				sectmap[a] |= m;

				while (filelength > 0)
				{
//					printf("t %d s %d, ",t,s); // Track and sector debug

					tt = t;
					ss = s;

					while ((sectmap[a] & m))
					{
						m *= 2;
						if (m==256)
						{
							m =1;
							a ++;
						}

						s++;
						if (s==11)
						{
							s=1;
							t++;

							if (t==80)
								t=128;
						}

					}

					*Addr(tt,ss,510) = t;
					*Addr(tt,ss,511) = s;

					if (filelength > 510)
					{
						fread(Addr(t,s,0),1,510,file);
						filelength -= 510;
						usedmap[a] |= m;
						sectmap[a] |= m;
					}
					else
					{
						fread(Addr(t,s,0),1,filelength,file);
						filelength = 0;
						usedmap[a] |= m;
						sectmap[a] |= m;

						*Addr(t,s,510) = 0;
						*Addr(t,s,511) = 0;
					};

				}
			};
			for (i=0;i<195;i++)
			{
				*(found+15+i) = usedmap[i];

			};
		};
}
			else
			{
				printf("File %s add failed\n", filename);
				exit(1);
			};
}

// Title Disk (MasterDOS/BDOS/Uni-DOS)

void TitleDisk(char *diskname)
{
int	i;
unsigned char *found;

		if (validmgt == 0)
		{
			printf("MGT file not found");
			exit(1);
		}	
		
		if (format ==0 || format == 3 )
			{
			printf("Sorry, disk rename only supported on MasterDOS/BDOS/Uni-DOS.\n");
			exit(1);
			}
			
		// Pad name to 10 chars	
			i=0;
			for (i=strlen(diskname); i<10; i++)
			{
				diskname[i]=' ';
			};
		    diskname[10]=0;	
			
		if (format ==1 || format ==2)	// MasterDOS and BDOS
		{
		found = Addr(0,1,0);	// First sector, first directory entry
			
		for (i=0; i<10; i++)
			{
				*(found+210+i) = diskname[i]; // 210 in first entry is Disk Name under Masterdos
			};	
		}
		
		if (format == 4)	// Uni-DOS
		{
		found = Addr(0,1,0);	// First sector, first directory entry
			
		for (i=0; i<10; i++)
			{
				*(found+246+i) = diskname[i]; // 246 in first entry is Disk Name under UNI-DOS
			};	
		}
}

// Read file from RAM image

void LoadFile(char *filename)
{
int	i,t,s,h,length,maxdtrack,ss,tt;
unsigned char	*found;
unsigned char	samfile[10];
FILE	*file;

		if (validmgt == 0)
		{
			printf("MGT file not found");
			exit(1);
		}	
		
		if (format >= 4)
			{
			printf("Sorry, file operations only supported on SAM Coupe formats right now.\n");
			exit(1);
			}
		
		// Make samefilename from filename to 10 chars with spaces
	
		i=0;
		for (i=0; i<=9; i++)
		{
			if (i < strlen(filename))
			{
			samfile[i]=filename[i];
			}
			else
			{
			samfile[i]=' ';
			}
		}
		samfile[10]=0;
		
		// Search image for filename to extract

		found = NULL;

		if ( *(image+255) == 255) 		// SamDOS format 
		{
			maxdtrack = 4;
		}
		else
		{
			maxdtrack = 4 + *(image+255); // MasterDOS additional directory tracks
		}

		for (t=0; t<maxdtrack; t++)
		{
			for (s=1; s<11; s++)
			{
				for (h=0; h<2; h++)
				{
					for (i=0; i<10; i++)
					{
						if (toupper (*Addr(t,s,256*h+1+i)) != toupper(samfile[i]))
						{
							i = 10;
						}
						else if (i==9)
						{
							found = Addr(t,s,256*h);
							i=10;
							h=2;
							s=10;
							t=maxdtrack;
						};
					};
				};
			};
		};

		if (found == NULL)
		{
			printf("\nFile %s not found in .MGT\n",filename);
			exit(1);
		}
		else
		{

// Open output file

			file = fopen(filename, "wb");
			
			if (file != NULL)
			{
				length = *(found +240);
				length += 256* *(found +241);
				length += 16384* *(found +239);

				i = length;

				t = *(found + 13);
				s = *(found + 14);

				if (i>=501)
				{
					fwrite(Addr(t,s,9),1,501,file);
					i -= 501;
				}
				else
				{
					fwrite(Addr(t,s,9),1,i,file);
					i=0;
				};

				while (i>0)
				{
					tt = *Addr(t,s,510);
					ss = *Addr(t,s,511);

					t = tt;
					s = ss;

//					printf("t %d  s %d\t",t,s);  // Debug output

					if (i>=510)
					{
						fwrite(Addr(t,s,0),1,510,file);
						i -= 510;
					}
					else
					{
						fwrite(Addr(t,s,0),1,i,file);
						i=0;
					};

				};

				fclose(file);

				printf("Extracted filename %s \nLength %d ",filename,length);

				length = *(found +237);
				length += 256* *(found +238);
				length += 16384* ((*(found +236) & 31) -1);

				printf("Start %d ", length);

				length = *(found +243);
				length += 256* *(found +244);
				length += 16384* (*(found +242) & 31);
			    length -= 32768; // take away offset

				if (length != 540671) printf("Execute %d ", length);

				printf("\n");
			} 
		}
}

// Output directory to screen

void Directorymgt(void)
{
int maxdtrack,nfiles,nfsect,flen,type,exec,startpage,startoffset,start,i,stat,track,sect,half;
char	diskname[11], filename[11];

		if (validmgt == 0)
		{
			printf("MGT file not found");
			exit(1);
		}	
	
		maxdtrack = 4;	// Default directory track size
		
		// Format 0 = SamDOS, 1 = MasterDOS, 2 = BDOS, 3=GDOS(DISCiPLE)/G+DOS(PlusD), 4=Uni-DOS
		
		if (format == 0) 
			{
			printf("\n             *** SAMDOS directory ***            \n\n");
			}
			
		if (format == 1) 
			{
			maxdtrack = 4 + *(image+255); // MasterDOS additional directory tracks
			
			for (i=0; i<10; i++)
			{
				diskname[i] = *(image+210+i); // 210 in first entry is Disk Name under Masterdos

			};
			diskname[10]=0;
			printf("\n        *** MasterDOS directory: %s ***        \n\n",diskname);
			}
			
		if (format == 2) 
			{
			for (i=0; i<10; i++)
			{
				diskname[i] = *(image+210+i); // 210 in first entry is Disk Name under BDOS

			};
			diskname[10]=0;
			printf("\n        *** BDOS directory: %s ***        \n\n",diskname);
			}
			
		if (format == 3) 
			{
			printf("\n        *** G(+)DOS directory ***            \n\n");
			}	

		if (format == 4) 
			{
			for (i=0; i<10; i++)
			{
				diskname[i] = *(image+246+i); // 246 in first entry is Disk Name under Uni-DOS

			};
			diskname[10]=0;
			printf("\n        *** UNI-DOS directory: %s ***        \n\n",diskname);
			}
			
		printf("Filename   HP Type         Size   Address Execute\n");
		printf("-------------------------------------------------\n");
		
		nfiles = 0;
		nfsect = 10 * (160 - maxdtrack) + (maxdtrack > 4);

		for (track = 0; track < maxdtrack; track++)
		{
			for (sect = 1; sect <= 10; sect ++)
			{
				for (half = 0; half < 2; half ++)
				{
					if ((track != 4) || (sect != 1))
					{
						if ((stat = *Addr(track,sect,256*half)) != 0)
						{

							for (i=0; i<10; i++)
							{
								filename[i] = *Addr(track,sect,256*half+1+i);
								if ((filename[i]<32) || (filename[i]>126)) filename[i]='?';

							};
							filename[10]=0;

							printf("%s ",filename);

							if (stat & 128) printf("*"); else printf("."); // Hidden 
							if (stat & 64) printf("*"); else printf("."); //Protected
							
							// This is hacky - use switch statement?
							
							if ((stat & 63)==0) printf(" %s","ERASED     ");
							if ((stat & 63)==1) printf(" %s","ZX BASIC   ");
							if ((stat & 63)==2) printf(" %s","ZX D.ARRAY ");
							if ((stat & 63)==3) printf(" %s","ZX $.ARRAY ");
							if ((stat & 63)==4) printf(" %s","ZX CODE    ");
							if ((stat & 63)==5) printf(" %s","ZX SNP 48k ");
							if ((stat & 63)==6) printf(" %s","MD.FILE    ");
							if ((stat & 63)==7) printf(" %s","ZX SCREEN$ ");
							if ((stat & 63)==8) printf(" %s","SPECIAL    ");
							if ((stat & 63)==9) printf(" %s","ZX SNP 128k");
							if ((stat & 63)==10) printf(" %s","OPENTYPE   ");
							if ((stat & 63)==11) printf(" %s","EXECUTE    "); // Only valid on DISCiPLE/+D as writes bytes to onboard 8K RAM
							if ((stat & 63)==12) printf(" %s","DIR        "); // In MDOS and Uni-Dos, SAMDOS uses 'WHAT?' for this
							if ((stat & 63)==13) printf(" %s","CREATE     "); // Uni-Dos only
						    if ((stat & 63)==16) printf(" %s","BASIC      ");
							if ((stat & 63)==17) printf(" %s","D.ARRAY    ");
							if ((stat & 63)==18) printf(" %s","$.ARRAY    ");
							if ((stat & 63)==19) printf(" %s","CODE       ");
							if ((stat & 63)==20) printf(" %s","SCREEN$    ");
							if ((stat & 63)==22) printf(" %s","DRIVER-APP "); // DRiVER App
							if ((stat & 63)==23) printf(" %s","DRIVER-BOOT");
							if ((stat & 63)==24) printf(" %s","EDOS NOMEN "); // Entropy EDOS (abandoned)
							if ((stat & 63)==25) printf(" %s","EDOS SYSTEM");
							if ((stat & 63)==26) printf(" %s","EDOS OVRLAY");
							if ((stat & 63)==28) printf(" %s","HDOS DOS   "); // SD Software HDOS
							if ((stat & 63)==29) printf(" %s","HDOS DIR   ");	
							if ((stat & 63)==30) printf(" %s","HDOS DISK  ");	
							if ((stat & 63)==31) printf(" %s","HDOS FRETMP");								
							
							// File size
							
							flen = *Addr(track,sect,256*half +240);
							flen += 256* *Addr(track, sect,256*half +241);
							flen += 16384* *Addr(track,sect,256*half +239);
												
							printf("%7.0f ",(float)flen);
														
							// Code Start if code file
							if ((stat & 63)==19)
								{
								start = 16384 *((*Addr(track, sect, 256*half +236) & 31)-1);
								start += *Addr(track,sect,256*half +237);
								start += 256* *Addr(track, sect, 256*half +238);					
								printf("%7.0f ",(float)start);
								}
								else
								{
									printf("       ");
								};
								
							// RUN/GOTO line if BASIC file	
							if ((stat & 63)==16) 
							{
								exec = *Addr(track,sect,256*half +243);
								exec += 256* *Addr(track, sect, 256*half +244);
						    												
							if (exec < 65535)	// Valid start addresss
								{
								printf("%7.0f\n",(float)exec);
								}
								else
								{
								printf("\n");
								};
							}
							
							// Execute address if CODE file
							if ((stat & 63)==19) 
							{
								exec = 16384* ((*Addr(track, sect, 256*half +242) & 31));
								exec += *Addr(track,sect,256*half +243);
								exec += 256* *Addr(track, sect, 256*half +244);
								exec -= 32768; // take away offset
																											
								if (exec != 540671)	// No exec addresss
									{
									printf("%7.0f\n",(float)exec);
									}
									else
									{
									printf("\n");
									};
							}	

							// If not code or BASIC (i.e. no addresses to show then carridge return)
							if ((((stat & 63)!=19) && (stat & 63)!=16)) printf("\n");
							
							nfsect -= *Addr(track,sect,256*half +12);
							nfsect -= 256* *Addr(track,sect,256*half +11);
							++nfiles;
						}
						else if ((*Addr(track,sect,256*half +1)) == 0)
						{
							half = 1;
							sect = 10;
							track = maxdtrack;
						};
					};
				};
			};
		};
		printf("\n%d files, %d Kb free\n",nfiles,nfsect/2);
}