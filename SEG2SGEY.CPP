
// 这个SEGY转换程序作字节交换，记录辅助道。

// Cvtseg.cpp for gqz240

//#define INCL_DOS
//#define INCL_SUBchar 
//#include "stdafx.h"
#include "stdio.h"
#include "stdlib.h"
#include "math.h"
#include "string.h"

#define MAXSAMP 16384  // max of 16k byte samples
#define MAXTRACES 16384   // SEG-2 limit
#define STRINGWIDTH 100   // allow up 100 char per keyword
#define MAXKEYWORDS 100  //
#define MAXPARMS 10  //   

//char *fpath = "E:\\yuncheng\\";  // add 2 lines by ytl
short NumAux = 2;

short outhead[120];  // output traceheader
long *outheadl;  // used when referncing outhead as a long array
float *outheadf;  // used when referncing outhead as a floating point array
char *outheadc;  // used when referncing outhead as a char array
//short i,j,k,kk,kk1,m,ssn,fstart,fend,count;
char string1[200];  
FILE *f1,*f2,*f3;
char temp;  //  
//
char prefix[STRINGWIDTH],start[10],end[10],exten[10],fname[STRINGWIDTH];
char outfile[STRINGWIDTH],answer[10],outfile1[STRINGWIDTH];
long tracepointers[MAXTRACES];  //
unsigned long numsamples,datalength;
//
char segykeyword[MAXKEYWORDS][STRINGWIDTH];
char input[STRINGWIDTH],inputbuf[STRINGWIDTH];
short segyfunction[MAXKEYWORDS],segyheader[MAXKEYWORDS],totalkeys;
double segyparms[MAXKEYWORDS][MAXPARMS];
short segyreelheader[1800];//
short first = 1;
FILE *keyfile;
char *token;
short reverse = 0;  //
double dinbuf[MAXSAMP];
long *linbuf;
short *iinbuf;
float *finbuf;
unsigned char *cinbuf;
long outbuf[MAXSAMP];  //
//
short revnum,pointerbytecount,numtrace;
unsigned short  blockid,blockleng;
char stringtermcount,stringterm1,stringterm2;
char linetermcount,lineterm1,lineterm2;
char reserved[19];
short stringlength;
unsigned char datatype;
short ftmp,fcomp;

void readsegykeys();
void keycheck();
void ieee2ibm(float *,short );

void main()
{
short i,j,k,kk,kk1,m,ssn,fstart,fend,count;
char sg(8);
	iinbuf = (short *)&dinbuf[0];  // used for interger input (16 bit)
	finbuf = (float *)&dinbuf[0];  // used for float point (4 byte)
	linbuf = (long *)&dinbuf[0];  // used for long int (32 bit)
	cinbuf = (unsigned char *)&dinbuf[0];  //used for 20 bitpacked
	outheadl = (long *)&outhead[0]; 
	outheadf = (float *)&outhead[0];
	system ("cls");  // clear screen
	printf("        SEG2SEGY V1.1 - OS/2 - MSDOS\n\n");
	printf("	  地震数据转换实用程序\n\n");
//	printf("      Seicmic Data Conversion Utility\n");
//	printf("    Copyright Kansas Geological Survey 1987-1990\n");
//	printf("  WELCOME OPRATPR TO THE SEG-2->SEG-Y PRE-PROCESSOR\n");
//	printf("      欢迎您使用 SEG-2->SEG-Y 予处理程序\n");
//	printf("\n\n这个SEGY转换程序作字节交换，记录辅助道。");

	printf
	("\n\n This program converts the SEG-2 data files to SEG-Y file format\n");
	printf
	(" NOTE:SEG-Y file is in fixed 32 bit int & byte reversed for main-frames\n");
	printf("\n Program assumes there are 1 or more files to convert");
	printf("\n with similar names containing ASCENDING numbers,i.e");
	printf("\n SEG00001.DAT,SEG00002.DAT,... SEG00099.DAT ");
	printf("\n\n Press RETURN when ready to proceed,CTRL-C to abort");
	gets(answer);
	system("cls");
 startovr:
	printf("\n\n Enter the PREFIX name of the files,(i.e SEG) -> ");
	gets(prefix);
badext:
	printf("\n Please enter the EXTENSION of the files if present.i.e .DAT ");
	printf("\n Iclude the (PERIOD) in name! -> ");
	gets(exten);
	if(4 < strlen(exten))
	{
		system("cls");
		printf("\nEXTENSION CAN'T BE MORE THAN 3 LETTERS");
		goto startovr;
	}
	if(strlen(exten) && exten[0] != '.')
	{
		printf("\nFirst character of an EXTENSION must be a period(.)\n");
			goto badext;
	}
tryagain:
	printf("\n Next enter the file numbers for the records to be");
	printf("\n changed. For instance if ther are to be 3 files");
	printf("\n converted named SEG00014,SEG00015,AND SEG00016,the first");
	printf("\n number you should enter is 000014,then when asked for ending");
	printf("\n enter 00016.\n REMEMBER TO ENTER LEADING 0's i.e.00014 not 14");
 	printf("\n\n Please enter STARTING number ->");
 	gets(start);
 	printf("\n Please enter ENDING number ->");
 	gets(end);

 //	if(strlen(start) != strlen(end))
//	{
 //		system("cls");
 //		printf("\n\n   Invalid numbers try again\n");
 //		goto tryagain;
//	} delete 6 lines by ytl 000720
	fstart = atoi(start);
 	fend = atoi(end);
//	if(fstart > fend)
//	{
//		system("cls");
//		printf("\n\n  Only ABCENDING file numbers!\n");
//		goto tryagain;
//	}delete 6 lines by ytl 000720
	system("cls");
	printf("\n You have chosen files :\n\n");
	printf("%s%s%s",prefix,start,exten);
	printf(" through %s%s%s\n\n",prefix,end,exten);
//	printf("Is this correct? (y/n) -> ");
//	gets(answer);
//	if(answer[0]=='n' || answer[0]=='N')
//	{
//		system("cls");
//		goto startovr;
//	}
	printf("\nPlease enter the name of the output file -> ");
	gets(outfile);
	f2 = fopen(outfile,"wb");  //
//	printf("\nPlease enter the name of the output file without headfile -> ");
//	gets(outfile1);
//	f3 = fopen(outfile1,"wb");  //
	if(f2 == NULL)
	{
		printf("**OUTPUT FILE OPEN FAILURE**\n**ABORTING**\n");
		exit(1);
	}
//	fclose(f2);  //////////////////////
//	printf("\nPlease enter the STARTING Shot Sequence Number(SSN) ->");
//	gets(answer);
//	ssn = atoi(answer);
//	system("cls");
//	readsegykeys();  // read in segy key words and save them
//	printf
//		("\n\nDo you want the input files deleted as they are converted?([n]/y) -> ");
//	answer[0] = '\0';
//	gets(answer);
//	fcomp = 0;	// add 11 lines by ytl 000720
	if(fstart > fend)
	{
		ftmp = fstart;
		fstart = fend;
		fend = ftmp;
		fcomp = 1;
		kk1 = fend;
	}
	else
		kk1 = fstart;

	for(kk=fstart; kk<=fend; kk++) // start a big loop!
	{
		fname[0] = '\0';
		strcat(fname,prefix);
		if(strlen(start))
			itoa(kk1,start,10);
//		count = strlen(end)-strlen(start);	
//
//		for(m=1; m<=count; m++)
//		{
//			strrev(start);
//			strcat(start,"0");
//			strrev(start);
//		} delete 8 lines by ytl 000720
//		strcat(fname,fpath);  // new add 1 lines by ytl 000623
		strcat(fname,start); //
		strcat(fname,exten);
		printf("\nWording on %s\n",fname);
		//+++++++++++++++++++++++++++++++++++++++++++++++++++++=
		f1 = fopen(fname,"rb");
		if(f1 == NULL)
		{
			printf("\n***SYSTEM REPORTS ERROR OPENING FILE %s***",fname);
			printf("Skipping to next file number\n");
			if(fcomp)	// add 4 lines by ytl 000714
				kk1--;
			else
				kk1++;

			continue;
		}
     //   fread(&sg,2,1,f1);
		fread(&blockid,2,1,f1);
		//
		if(blockid == 0x553a)
		{
			reverse = 1;
			printf("Byte reverse data not allowed.\n");
			exit(1);
		}
//		if(blockid /= 0x3a55)
//		{
//			printf("Not SEG-2 data can not continue\n");
//			exit(1);
//		}
		fread(&revnum,2,1,f1);//SEG-2的版本号
		fread(&pointerbytecount,2,1,f1);//道指针子块的大小
		fread(&numtrace,2,1,f1);//道数

		printf("Data revision number = %d\n",revnum);
		printf("Nunber of trace = %d\n",numtrace);
		fread(&stringtermcount,1,1,f1);
		fread(&stringterm1,1,1,f1);
		fread(&stringterm2,1,1,f1);
		fread(&linetermcount,1,1,f1);
		fread(&lineterm1,1,1,f1);
		fread(&lineterm2,1,1,f1);
		//
		fread(&reserved,1,18,f1);//以上共32个字节
		//
		//
		//
/*		if(numtrace != (pointerbytecount/4))
		{
			printf("Number of trace and number of trace pointers do not match\n");
			printf("Number of pointers = %d\n",pointerbytecount/4);
			printf("Due to this inconsistency processing must stop\n");
			exit(1);
		}
*/		//  ok new read in the list
		fread(tracepointers,4,numtrace,f1);//201*4=804字节，存放道指针数据
/* The next block is the FILE DESCRIPTOR BLOCK.It is a set of strings, each terminaterd
	by the string _term_character(s).Each string is preceded by 1 WORD which is the byte  
	count for the string+termchar+2.(2 for the length of the word giving the count.) A
	value of 0 represents the end of the block.
*/
		fread(&stringlength,2,1,f1);
		while(0 != stringlength)
		{
			fread(string1,1,stringlength-2,f1);//采集参数数据采集日期和时间“ACQUISITION_DATE 10/21/200”
			keycheck();
			fread(&stringlength,2,1,f1);//？
		}
		for(j=0; j<numtrace; j++)
		{
			printf("trace-%d- ",j+1);
			fseek(f1,tracepointers[j],SEEK_SET);  // SEEK_SET: go to beginning of data block
			fread(&blockid,2,1,f1);//道描述开始1-2
			if(blockid == 0x2244)
			{
				reverse = 1;
				printf("Reversed data not allowed at this time\n");
				exit(1);
			}
			if(blockid != 0x4422)
			{
				printf("Not SEG-2 data can not process %x\n",blockid);
				exit(1);
			}
			fread(&blockleng,2,1,f1);//3-4
			fread(&datalength,4,1,f1);//5-8
			fread(&numsamples,4,1,f1);//采样点数9-12
/* 
	1 = 16 bit fixed. (INT)
	2 = 32 bit fixed. (LONG)
	3 = 20 bit packed floating.
	4 = 32 bit ieee floating point.(FLOAT)
	5 = 64 bit ieee floating point.(DOUBLE)
*/
			fread(&datatype,1,1,f1);//无符号整型，用来指明该道数据的存储格式（此数据为2，即32位整数）
			if(datatype > 5 || datatype < 1)
			{
				printf("Data type %d not avilable/valid\n",(short)datatype);
				break;
			}
			outhead[57] = numsamples;
			fread(reserved,1,19,f1);
			fread(&stringlength,2,1,f1);
			while(0 != stringlength)
			{
				fread(string1,1,stringlength-2,f1);//采集的参数信息，其中包括采样率
				keycheck();
				fread(&stringlength,2,1,f1);
			}
			printf("\n");
			fseek(f1,blockleng+tracepointers[j],SEEK_SET);  // go to beginning of seicmic data
			switch(datatype)
			{
			case 1:
				{
					fread(iinbuf,2,(short)numsamples,f1);
					for(k=0; k<numsamples; k++)
						outbuf[k] = iinbuf[k];
					break;
				}
			case 2:
				{
					fread(linbuf,4,(short)numsamples,f1);
					for(k=0; k<numsamples; k++)
						outbuf[k] = linbuf[k];
					fwrite(&outbuf[k],2,1,f3);
					break;
				}
			case 3:
				{
					unsigned short totalbytes,subpointer;
					unsigned short expo;
					long longdat;
					totalbytes = (numsamples * 5)/2;
					fread(cinbuf,1,totalbytes,f1);
					for(k=0; k<(numsamples);  )
					{
						subpointer = (k / 4) * 5;
						expo = (unsigned short)iinbuf[subpointer++];
						for(i=0; i<4; i++)
						{
							if(0x8000 & iinbuf[subpointer])
								longdat = 0xffff8000;
							else
								longdat = 0;
							longdat = longdat | ((long)iinbuf[subpointer++] << (0x000f & expo));
							expo >>= 4;
							outbuf[k++] = longdat;
						}
					}
					break;
				}
			case 4:
				{
					fread(finbuf,4,(short)numsamples,f1);
					for(k=0; k<numsamples; k++)
						outbuf[k] = finbuf[k];
					break;
				}
			case 5:
				{
					fread(dinbuf,8,(short)numsamples,f1);
					for(k=0; k<numsamples; k++)
						outbuf[k] = dinbuf[k];
					break;
				}
			} // end switch
			//
			//
			//
			//
			//
//			if(outheadl[2] == 0)  // delete 1 line by ytl
				outheadl[2] = (long)kk1;
			//
			//
//			if(outhead[14] == 0) delete 1 line by ytl
				outhead[14] = (long)1; // seismic data.
			if(NumAux != 0 && j == numtrace-2)	// add 4 lines by ytl 000712		
				outhead[14] = (long)4;
			if(NumAux != 0 && j == numtrace-1)			
				outhead[14] = (long)5;

/*			if(j == numtrace-1 && outhead[87] == 0)
			{
				outhead[87] = 1;
				ssn = ssn + 1;
			}  	delte by ytl 000712	 */
			outheadl[9] = (long)(abs(outhead[93]-outhead[92]));
			outhead[85] = outhead[86] = (short)(1+labs((long)outhead[92] - outheadl[3]));
			if(first == 1)
			{
				first = 0;
				segyreelheader[1606] = numtrace-NumAux;  // was numtrace 
				segyreelheader[1607] =  NumAux;  // add 2 lines by ytl  000629
				segyreelheader[1627] = 1;  //  meters 
				segyreelheader[1608] = outhead[58];
				segyreelheader[1609] = outhead[58];
				segyreelheader[1610] = numsamples;
				segyreelheader[1611] = numsamples;
				segyreelheader[1612] = 2;
				for(k=1600; k<1606; k+=2)
				{
					outheadc = (char*)&segyreelheader[k];
					temp = outheadc[3];
					outheadc[3] = outheadc[0];
					outheadc[0] = temp;
					temp = outheadc[2];
					outheadc[2] = outheadc[1];
					outheadc[1] = temp;
				}
				for(k=1606; k<1630; k++)
				{
					outheadc = (char*)&segyreelheader[k];
					temp = outheadc[0];
					outheadc[0] = outheadc[1];
					outheadc[1] = temp;
				}
				fwrite(segyreelheader,1,3600,f2);
			}
			//
			//
			//
			//
			//
			//
			//
			//
			//
			//
			for(k=0; k<7; k++)	// convert byte 0--27
			{
					outheadc = (char*)&outheadl[k];
					temp = outheadc[3];
					outheadc[3] = outheadc[0];
					outheadc[0] = temp;
					temp = outheadc[2];
					outheadc[2] = outheadc[1];
					outheadc[1] = temp;
			}
			for(k=9; k<17; k++)	// convert byte 36--67
			{
					outheadc = (char*)&outheadl[k];
					temp = outheadc[3];
					outheadc[3] = outheadc[0];
					outheadc[0] = temp;
					temp = outheadc[2];
					outheadc[2] = outheadc[1];
					outheadc[1] = temp;
			}
			for(k=18; k<22; k++)	// convert byte 72--87
			{
					outheadc = (char*)&outheadl[k];
					temp = outheadc[3];
					outheadc[3] = outheadc[0];
					outheadc[0] = temp;
					temp = outheadc[2];
					outheadc[2] = outheadc[1];
					outheadc[1] = temp;
			}
			//
			for(k=14; k<18; k++)  // convert byte 28--35
			{
					outheadc = (char*)&outhead[k];
					temp = outheadc[0];
					outheadc[0] = outheadc[1];
					outheadc[1] = temp;
			}
			for(k=34; k<36; k++)  // convert byte 68--71
			{
					outheadc = (char*)&outhead[k];
					temp = outheadc[0];
					outheadc[0] = outheadc[1];
					outheadc[1] = temp;
			}
			for(k=44; k<90; k++)  // convert byte 88--179
			{
					outheadc = (char*)&outhead[k];
					temp = outheadc[0];
					outheadc[0] = outheadc[1];
					outheadc[1] = temp;
			}
			if(120 != (k = fwrite(outhead,2,120,f2)))
			{
				printf("Write failure during header write\n");
				exit(-1);
			}
			for(k=0; k<numsamples; k++)
			{
					outheadc = (char*)&outbuf[k];
					temp = outheadc[3];
					outheadc[3] = outheadc[0];
					outheadc[0] = temp;
					temp = outheadc[2];
					outheadc[2] = outheadc[1];
					outheadc[1] = temp;
			}
			if((short)numsamples != (k = fwrite(outbuf,4,(short)numsamples,f2)))
			{
				printf("Write failure during trace write\n");
				exit(-1);
			}
		}  // end trace loop
		fclose(f1);
		if((answer[0] == 'y') || (answer[0] == 'Y'))
		{
			printf("Deleting %s\n",fname);
			i = unlink(fname);
			if(i)
				printf("  DELETE FAILED ON %s!\n",fname);
		}
		outhead[87] = 0;
		if(fcomp)	// add 4 lines by ytl 000714
			kk1--;
		else
			kk1++;
	} // end of "for (kk=fstart; kk<=fend; kk++)"
	fclose(f2);	
	fclose(f3);// NEW ADD
}

void readsegykeys()
{
	short i,j,k;
	char keypath[STRINGWIDTH];  //

	_searchenv("SEGYKEYW.ORD","PATH",keypath);
	keyfile = fopen(keypath,"rb");

	if(keyfile == NULL)
	{
		printf("segy keyword file SEGYKEYW.ORD not found.Can not continue\n");
		exit(1);
	}
	i = 0;
	while(fgets(input,STRINGWIDTH,keyfile))
	{
		j = 0;
//		aa = strlen(input);
		if(strlen(input) > STRINGWIDTH)
		{
			printf("String too long!\n");
			exit(1);
		}
// ok now must parse the data if left most character = "*"
// then this line is a comment and should be ignored.
		if(input[0] == 42)  // 42 = *
		{
			continue;
		}
		strcpy(inputbuf,input);  //
		token = strtok(inputbuf," ");
		//
		strncpy(&segykeyword[i][0],token,1+strlen(token));
		//
		token = strtok(NULL," ");
		segyfunction[i] = atoi(token);  //
		token = strtok(NULL," ");
		segyheader[i] = atoi(token);
		token = strtok(NULL," ");
		j = 0;
		while(token != NULL)
		{
			segyparms[i][j] = atof(token);
			token = strtok(NULL," ");
			j++;
			if(j > MAXPARMS)
			{
				printf("Too many parameters in %s keyword\n",&segykeyword[i][0]);
				printf("No more than &d allowed per function\n",j-1);
				exit(-1);
			}
		}  // end parameter extraction while loop
		i++;
	}  //end keyword string while loop
	totalkeys = i;
}
void keycheck()
{
	short i,j,k;
	short matchfound;
	char string2[STRINGWIDTH];
	unsigned short aa;

	strcpy(string2,string1);
	matchfound = 0;
	for(i=0; i<totalkeys; i++)
	{
		strcpy(string1,string2);
		aa = strlen(&segykeyword[i][0]);
		if(0 == strnicmp(string1,&segykeyword[i][0],aa))//strlen(&segykeyword[i][0])));
		{
			matchfound = 1;
			switch(segyfunction[i])
			{
			case 0:
				{
					break;
				}
			case 1:
				{
					token = strtok(string1," ");
					token = strtok(NULL," ");
					//
					//
					//
					outhead[segyheader[i]-1] = atof(token) *segyparms[i][0];
					break;
				}
			case 2:
				{
					token = strtok(string1," ");
					token = strtok(NULL," ");
					if(0 == strcmpi("AS_ACQUIRED",token))
					{
						segyreelheader[segyheader[i]-1] = 1;
					}
					if(0 == strcmpi("CDP_GATHER",token))
					{
						segyreelheader[segyheader[i]-1] = 2;
					}
					if(0 == strcmpi("CDP_STACK",token))
					{
						segyreelheader[segyheader[i]-1] = 4;
					}
					if(0 == strcmpi("COMMON_OFFSET",token))
					{
						segyreelheader[segyheader[i]-1] = 3;
					}
					if(0 == strcmpi("COMMON_RECEIVER",token))
					{
						segyreelheader[segyheader[i]-1] = 1;
					}
					if(0 == strcmpi("COMMON_SOURCE",token))
					{
						segyreelheader[segyheader[i]-1] = 1;
					}
					break;
				}
			case 3:
				{
//					strncpy((char*)[segyreelheader[80*(segyheader[i]-1)],string1,80);
					strncpy((char*)&segyreelheader[80*(segyheader[i]-1)],string1,80);
					break;
				}
			case 4:
				{
					token = strtok(string1," ");
					outhead[segyheader[i]-1] = 1;
					token = strtok(NULL," ");
					if(0 == strcmpi("SEICMIC_DATA",token))
					{
						outhead[segyheader[i]-1] = 1;
					}
					if(0 == strcmpi("DEAD",token))
					{
						outhead[segyheader[i]-1] = 2;
					}
					if(0 == strcmpi("TEST_DATA",token))
					{
						outhead[segyheader[i]-1] = 3;
					}
					if(0 == strcmpi("UPHOLE",token))
					{
						outhead[segyheader[i]-1] = 5;
					}
					if(0 == strcmpi("RADAR_DATA",token))
					{
						outhead[segyheader[i]-1] = 1;
					}
					break;
				}
			case 5:
				{
//					strcpy(string1,string2);
					token = strtok(string1," ");
//					token = strtok(NULL," ");
					if(segyheader[i] == 0)
					{
						short paramcount = 1;
						short headindex;
						token = strtok(NULL," ");
						while(token != NULL && paramcount < 10)
						{
							headindex = segyparms[i][paramcount]-1;
							outhead[headindex] = atof(token) * segyparms[i][0];
							paramcount++;
							token = strtok(NULL," ");
						}
					}
					if(segyheader[i] == 1)
					{
						short paramcount = 1;
						short headindex;
						long *outpoint;
						token = strtok(NULL," ");
						while(token != NULL && paramcount < 10)
						{
							headindex = segyparms[i][paramcount] - 2;
							outpoint = (long*)&outhead[headindex];
							outpoint[0] = (long)(atof(token) * segyparms[i][0]);
							paramcount++;
							token = strtok(NULL," ");
						}
					}
					if(segyheader[i] == 2)
					{
						short paramcount = 1;
						short headindex;
						float *outpoint;
						token = strtok(NULL," ");
						while(token != NULL && paramcount < 10)
						{
							headindex = segyparms[i][paramcount] - 2;
							outpoint = (float*)&outhead[headindex];
							outpoint[0] = (float)(atof(token) * segyparms[i][0]);
							paramcount++;
							ieee2ibm(outpoint,0);
							token = strtok(NULL," ");
						}
					}
					break;
				} // enf case 5
			default:
				{
					printf("Function %d not defined.\n",segyfunction[i]);
					break;
				}
			} // end case switch
		}  // end if
	}  // end i loop
		if(!matchfound)
			printf("No match found for %s\n",string1);
}  //end of keysegy
void ieee2ibm(float *input,short swap)
{
	unsigned char *cbuf,exp,tem;
	unsigned long *umantis;
	long expl;
	unsigned long usignl;
	umantis = (unsigned long*)&input[0];
	cbuf = (unsigned char*)&input[0];
	//
	usignl = *umantis & 0x80000000;
	exp = (*umantis >> 23);
	*umantis= ((*umantis | 0x00800000)) &0x00ffffff;
	exp = (exp-127) + 1;
	//
	//
	//
	if(exp % 4)
	{
		*umantis = *umantis >> (exp % 4);
		exp = exp +4;
	}
	exp = exp >> 2;
	exp = exp + 64;
	expl = exp;
	expl = (expl << 24) & 0x7f000000;
	*umantis = *umantis | expl | usignl;
	if(swap)
	{
		tem = cbuf[0];
		cbuf[0] = cbuf[3];
		cbuf[3] = tem;
		tem = cbuf[1];
		cbuf[1] = cbuf[2];
		cbuf[2] = tem;
	}
}
ibm2ieee(float *input,short swap)
{
	unsigned char *cbuf,expp,tem,sign;
	unsigned long *umantis,expll,signl;
//	unsigned long usignl;
	long *mantis;
	short shift;

	
	cbuf = (unsigned char*)&input[0];
	umantis = (unsigned long*)&input[0];
	mantis = (long*)&input[0];
	//
	if(swap)
	{
		tem = cbuf[0];
		cbuf[0] = cbuf[3];
		cbuf[3] = tem;
		tem = cbuf[1];
		cbuf[1] = cbuf[2];
		cbuf[2] = tem;
	}
	//
	expp = *mantis >> 24;
	*mantis = (*mantis) << 8;
	shift = 1;
	while(*mantis > 0 && shift < 23)
	{
		*mantis = *mantis << 1;
		shift++;
	}
	*mantis = *mantis << 1;
	sign = expp & 0x80;
	expp = expp & 0x7f;
	if(expp != 0)
	{
		expp = expp -64;
		*umantis = *umantis >> 9;
		expll = 0x7f + (expp * 4 - shift);
		expll = expll << 23;
		if(sign)
			*umantis = expll | *mantis | 0x80000000;
		else
			*umantis = expll | *mantis;
	}
}