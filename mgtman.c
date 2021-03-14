// Sam Coupe MGT/DSK Manipulator 2.0.0
//
// 		Hacky command line remix by Dan Dooré 
//		1.0.0 MacOS by Andrew Collier
// 		Quick and dirty ANSI C port by Thomas Harte!!
// 		Command line enhancement by Frode Tennebø for z88dk

// 2.0.0 - Initial Build
//			Made command line only
//			Added access to read and directory functions
//			Refactor directory listing to deal with all file types, start addresses, execute addresses
//			Fixed bug in calculation of execute address in read function with offset
//			Added check for duplicate filenames before writing to MGT
//			Added abilty to make CODE files auto starting

// Bugs
//			Disc detection of samdos/masterdos always picks MasterDOS?

// Possible Enhancements
//			Have less hacky C code - C is not my thing, in fact programming these days isn't :) 
//			Windows Z88DK build script in PowerShell
//			Linux Z88DK Build script - in BASH? SH?
//			Make writing of start and execution address based on user input instead of arbiatary 32768/32768
//			Support PlusD/Disciple/UNI-DOS images

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

// Prototypes

void	Savemgt(char *mgtimage);
void	Openmgt(char *mgtimage);
void	SaveFile(char *fname, char exec);
void	LoadFile(char *filename);
void	Directorymgt(void);
void 	Usage(void);

// Global vairables

int 	validmgt;					// Status of MGT image in memory
unsigned char	*image,*valid;		// MGT image in memory

// Main entry point with arguments

int main(int argc, char **argv)
{
char	command;
int t;

	if ((image = malloc(819200)) == (unsigned char *) NULL)
	{
		printf("Not enough free memory to load .MGT file\n");
	    exit(1);
	}
	else
	{
		if (argc < 3)
		{
		     Usage();
		     exit(1);
		}
	    
		if (argc >= 2 )
			{
		        if (argv[1][0] == '-')
				{
					if (argv[1][1] == 'h')
						{
						 Usage();
						 exit(1);
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
						 if (argv[4] != NULL)
							{
							SaveFile(argv[3],argv[4][0]);
							}
						else
							{
							SaveFile(argv[3],'n');	
							}
						 Savemgt(argv[2]);
						 exit(0);
						}
					if (argv[1][1] == 'r')
						{
						 Openmgt(argv[2]);
						 LoadFile(argv[3]);
						 exit(0);
						}
				}
			}
	};
	return 0;
}

// Usage text

void Usage(void)
{
	printf ("        ,\n");
	printf ("SAM Coupe .MGT/DSK image manipulator\n");
	printf ("------------------------------------\n");
	printf ("                                         ,\n");
	printf ("2021 Hacky command line remix by Dan Doore\n");
	printf ("Original MAC OS version by Andrew Collier\n");
	printf ("Quick and dirty ANSI C port by Thomas Harte\n");
	printf ("Command line enhancements by Frode Tennebo for Z88DK\n");

	printf ("\nUsage: mgtman <-h | -d | -w | -r> <mgt-file> [samfile] [options]\n\n");
	printf ("  -h  This help\n");
	printf ("  -d  Directory listing of mgt-file\n");
	printf ("  -r  Read samfile from mgt-file\n");
	printf ("  -w  Write CODE samfile to mgt-file, add [options] if required\n");
	printf ("\nOptions:\n\n   x  Make CODE file executable at 32768\n\n");
	printf ("Example: mgtman -w test.mgt samfile x\n\n");
	printf ("Filenames to write should conform to Sam conventions (Max: 10 chars, etc.)\n");
	printf ("If the mgt-File does not exist it will be created when -w is used.\n\n");
	printf ("Why is this called MGTman and not DSKman?\n");
	printf ("-----------------------------------------\n");
	printf ("DSK files now relate to EDSK format files which are a flexible disk format.\n");
	printf ("MGT files are a raw dump of the disk data but the term is still used interchangably\n");
	printf ("so most DSK files relating to the SAM Coupe are the raw dump (MGT) format.\n");
}

// Open MGT file and copy contents to RAM in image

void Openmgt(char *mgtimage)
{
int i;
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
			}
			else
			{
				printf ("Read fault on .MGT file: %s\n",mgtimage);
				exit(1);
				
			};
		}
		else
		{
			printf ("%s is not a valid .MGT file\n",mgtimage);
			exit(1);
		};
		fclose (mgt);
	}
	else
	{
		validmgt = 0;
	};

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
			printf ("Save fault on .MGT file: %s\n",mgtimage);
			exit(1);
		}
		fclose(mgt);
	}
	else
		{
			printf ("Could not open MGT file %s for saving\n",mgtimage);
			exit(1);
		};
}

// Read data from RAM image based on track/sector/offset

unsigned char *Addr(int track,int sector,int offset)
{
	unsigned char *a;

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

void SaveFile(char *filename, char exec)
{
unsigned char sectmap[195],usedmap[195],*found,*exists,samfile[10];
int filelength,maxdtrack,s,t,h,i,m,a,tt,ss;
FILE	*file;
		
		// If  a new disk, create.
		
		if (validmgt == 0)
		{		
		for (i = 0; i<819200; i++)
			{
			*(image+i) = (unsigned char)0;
			}
		}
		
		file = fopen(filename, "rb");
		if (file != NULL)
	    {
	    // Search image to check that filename is not already taken	
		
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
			
		// Check directory
		
		exists = NULL;

		if ( *(image+255) == 255)
		{
			maxdtrack = 4;
		}
		else
		{
			maxdtrack = 4 + *(image+255);
		}

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
			printf ("File %s already exists in MGT\n",filename);
			exit(1);
		}
			
		// Allocate sector map for file
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
			*(found) = 19;				// Status - set type as CODE in Diretory Entry
			*Addr(t,s,0) = 19;			// Status - set type as CODE in 9 byte File Header
			
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

			*(found+236) = 1;
			*Addr(t,s,8) = 1;
			*(found+237) = 0;
			*Addr(t,s,3) = 0;			
			*(found+238) = 128;
			*Addr(t,s,4) = 128;

			*(found+239) = filelength/16384;
			*Addr(t,s,7) = filelength/16384;
			*(found+240) = filelength%256;
			*Addr(t,s,1) = filelength%256;
			*(found+241) = (filelength%16384)/256;
			*Addr(t,s,2) = (filelength%16384)/256;

			// If exec option is set to 'x' make it so with 32768
	
			if (exec == 'x')
				{
					*(found+242) = 2;		//page
					*(found+243) = 0;		//lsb
					*(found+244) = 128;		// msb
				}
				else
				{
					*(found+242) = 255;
					*(found+243) = 255;
					*(found+244) = 255;
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
//					printf ("t %d s %d, ",t,s); // Track and sector debug

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

// Read file from RAM image

void LoadFile(char *filename)
{
int i,t,s,h,length,maxdtrack,ss,tt;
unsigned char *found;
unsigned char samfile[10];
FILE	*file;

		if (validmgt == 0)
		{
			printf("MGT file not found");
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

		if ( *(image+255) == 255)
		{
			maxdtrack = 4;
		}
		else
		{
			maxdtrack = 4 + *(image+255);
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
			printf ("\nFile %s not found in .MGT\n",filename);
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

				printf ("Start %d ", length);

				length = *(found +243);
				length += 256* *(found +244);
				length += 16384* (*(found +242) & 31);
			    length -= 32768; // take away offset

				if (length != 540671) printf ("Execute %d ", length);

				printf ("\n");
			} 
		}
}

// Output directory to screen

void Directorymgt(void)
{
int maxdtrack,nfiles,nfsect,flen,type,exec,startpage,startoffset,start,i,stat,track,sect,half;
char filename[11];

		if (validmgt == 0)
		{
			printf("MGT file not found");
			exit(1);
		}	
		
		if ( *(image+255) == 255)
		{
			printf ("             *** SAMDOS directory ***            \n\n");
			maxdtrack = 4;
		}
		else
		{
			maxdtrack = 4 + *(image+255);

			for (i=0; i<10; i++)
			{
				filename[i] = *(image+210+i);

			};
			filename[10]=0;
			printf ("        *** MasterDos directory: %s ***        \n\n",filename);
			
		};

		printf ("Filename   HP Type         Size   Address Execute\n");
		printf ("-------------------------------------------------\n");
		
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

							if (stat & 128) printf ("*"); else printf("."); // Hidden
							if (stat & 64) printf ("*"); else printf("."); //Protected
							
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
							if ((stat & 63)==11) printf(" %s","N/A EXECUTE");
							if ((stat & 63)==12) printf(" %s","WHAT?      ");
						    if ((stat & 63)==16) printf(" %s","BASIC      ");
							if ((stat & 63)==17) printf(" %s","D.ARRAY    ");
							if ((stat & 63)==18) printf(" %s","$.ARRAY    ");
							if ((stat & 63)==19) printf(" %s","CODE       ");
							if ((stat & 63)==20) printf(" %s","SCREEN$    ");
							
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
								
							// Starting line if BASIC file	
							if ((stat & 63)==16) 
							{
								exec = *Addr(track,sect,256*half +243);
								exec += 256* *Addr(track, sect, 256*half +244);
						    												
							if (exec <= 65535)	// Valid start addresss
								{
								printf("%7.0f\n",(float)exec);
								}
								else
								{
								printf("\n");
								};
							}
							
							// Starting address if CODE file
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

							if (flen == 0) printf("\n");	// Allow PlusD/Disciple disks to be displayed, not that they are supported						
							
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


	



