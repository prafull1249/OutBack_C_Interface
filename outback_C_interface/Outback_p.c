/*	OutBackShell	version 0.92	Mar. 1, 2012
 *
 */

#include <stdio.h>

#ifndef WIN32
#include <unistd.h>
#endif
#include <getopt.h>

#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <math.h>
#include <time.h>
#include <ctype.h>
#include <string.h>

#include "outback.h"
#include "names.h"
#include "registers.h"

#define MB_SERV_PORT		502
#define DEF_SERV_IP_ADDR	"192.168.2.220"
#define BUFF_SIZE	128

int debug = 0;
int devices	= 0;
int connected = 0;
int pflag = 0;
char buff[BUFF_SIZE];
char address[16];
char c;
int	i, j;
char command = 0;
int	block;
int	field;
char value_write[BUFF_SIZE];
int mb_port = MB_SERV_PORT;
int mflag=0, cflag=0, fflag=0, bflag=0, aflag=0, mul_flag = 0, wflag=0, vflag=0, ret_val=-1;
int bGetVerFW=0, bVerbose=0, bGetEnergy=0, bGetMfg=0, bGetJoules=0, bJSON=0, yGetInput=0, yGetOutput=0, yGetGrid=0;

FILE *outfp;

static char *outfile = NULL;
#define BOOL int
#define TRUE 1
#define FALSE 0

/*--------------------------------------------------------------------------
    isNumeric
----------------------------------------------------------------------------*/

int string_split(void) {
  char st[] ="Where there is will, there is a way.";
  char *ch;
  ch = strtok(st, " ");
  while (ch != NULL) {
  printf("%s\n", ch);
  ch = strtok(NULL, " ,");
  }
  //getch();
  return 0;
}

BOOL isNumeric(char *p)
{
    int i;

    for (i = 0; i <= strlen(p); i++)
        if (p[i] != '\0' && ! isdigit(p[i])) return(FALSE);
    return(TRUE);
}

/*--------------------------------------------------------------------------
        getMemPtr
----------------------------------------------------------------------------*/
void *getMemPtr(size_t mSize)
{
    void *ptr;
    char *cptr;
    int i;

    ptr = malloc(mSize);
    if (!ptr) {
        fprintf(stderr, "\nvproweather: malloc failed\n");
        exit(2);
    }
    cptr = (char *)ptr;
    for (i = 0; i < mSize; i++) cptr[i] = '\0';
    return ptr;
}


void print_usage() {
    printf("Usage: outback [rwa] -c ip [-p port] -f field -b block -v value\n");
}

int GetParams(int argc, char *argv[]) {
    int opt= 0;
    char* ip = DEF_SERV_IP_ADDR;
    ret_val = -1;
    //Specifying the expected options
    //The two options l and b expect numbers as argument
    static struct option long_options[] = {
        {"write",      no_argument,       0,  'w' },
        {"read", no_argument,       0,  'r' },
        {"all", no_argument,       0,  'a' },
		{"multiple", no_argument, 0, 'm'},
        {"ip",    required_argument, 0,  'c' },
        {"port",   required_argument, 0,  'p' },
        {"field",   required_argument, 0,  'f' },
        {"version",   optional_argument, 0,  'F' },
        {"block",   required_argument, 0,  'b' },
        {"value",   required_argument, 0,  'v' },
	//added from Aurora abb
        { "JSON",	no_argument,    0,	'J' },
        { "input",    optional_argument,	0,	'i' },
        { "output",	optional_argument,	0,	'j' },
        { "grid",	optional_argument,	0,	'g' },
		{ "serial-number",	no_argument,	0,	'n' },
        { "output-file",	required_argument,   0,   'o' },
        { "firmware-ver",	no_argument,	0,	'f' },
        { "mfg-date",	no_argument,	0,	'm' },
		{ "verbose",	no_argument,	0,	'l' },
        {0,    0,    0,  0   }
    };

    int long_index =0, i=0;
    if (outfile != NULL) {
       	if (!(outfp = fopen(outfile, "a"))) {
       	    fprintf(stderr, "Problem opening output file %s\n",outfile);
       	    //bFileError = TRUE;
        	}
 	} else {
        	outfp = stdout;
 	}
    while ((opt = getopt_long(argc, argv,"rmwaFc:p:f:b:v:Jd:njo:fgli:",
                   long_options, &long_index )) != -1) {
        switch (opt) {
	    case 'a' :// read all
	  	    aflag = 1;
       		    ret_val = 'a';
                break;
	    case 'm' :// read multiple
	  	    mflag = 1;
			ret_val = 'm';
			break;

            case 'B':  //Verbose
				bVerbose     = TRUE; break;

            case 'b' : // block to read
				 block = atoi(optarg);
 				 bflag = 1;
				 break;
            case 'c' ://Connect with ip given

				for (i =0; i<16; i++){
					address[i] = optarg[i];
				}
				if (i<=15){
					address[i] ='\0';
				}
			        /*
				for (i =0; i<16; i++){
					printf("%d  %d\n", i, address[i]);
					if (address[i] == 0 || address[i]=='\0')
						break;
				}
				*/
				
				 cflag = 1;
                 break;
            case 'F':  //framework version
				bGetVerFW       = TRUE; break;		
            case 'f' : // read field
				 field = atoi(optarg);
				 fflag = 1;
                 break;			
            case 'g':  
				//Get the manufacturer
				bGetMfg         = TRUE; break;

	    case 'i': // Get input parameters
                if (optarg == NULL) {
                    yGetInput = DC_ALL;
                } else {
				i = atoi(optarg);
                   switch(i){
                   case 1:
					yGetInput = DC_VOLTAGE;
                           break;
                   case 2:
        		    yGetInput = DC_CURRENT;
                           break;
                   case 3:
         		    yGetInput = DC_POWER;
                    break;
                   case 4:
         		    yGetInput = DC_ALL;
                    break;
		   default:
                           printf("Enter proper argument for input parameters\n");
                           break;
			   return -1;
        		}
		
		}
		
                break;

	    case 'j': 
				// get output parameters
				yGetOutput   = TRUE; break;
            case 'J': //JSON format
				bJSON     = TRUE; break;

	    case 'o':
				//output-file in which the output will be stored.
                outfile = getMemPtr(strlen(optarg)+1); 
                strcpy(outfile, optarg);
                outfile[strlen(outfile)] = '\0';
	

                break;

            case 'p' : // port for connection
				 mb_port = atoi(optarg);
                 break;

            case 'r' : // read register
				 mflag = 1;
				 ret_val = 'r';
                 break;
	 
	    case 'v' :
		//value when writing the register
		 for (i = 0; i<BUFF_SIZE; i++)
			value_write[i]=0;

		 for (i = 0; i<BUFF_SIZE; i++){
			if (optarg[i]== 0)
			    break;
			value_write[i] = optarg[i];
		 }

		 vflag = 1;
		 break;
					 
	   case 'w' : //write register
		 mflag = 1;
		 wflag=1;
		 ret_val = 'w';
		 break;
	   
	   default: print_usage();
                 return -1;
        }
    }
    if(cflag == 0){
     	   print_usage();
           return -1;
	}
    else if(mflag==1){
    	if(fflag==0 || bflag==0)
	{
		//printf("2nd check\n");
		print_usage();
		return -1;
	}
	if ((wflag==1) && (vflag==0)){
		print_usage();
		return -1;
	   }
	}

    return ret_val;
}

int outback_read(){
		ConnectAXS();
		if (0 != connected)
		{
			//block = 0;
			//field = 0;
		
			if (pflag)
				printf("Block number: %d \n", block);
			//scanf("%d", &block);
			if ( (block < 0) || (block > devices) )
			{
				if (pflag)
				    printf("Invalid block number\n");
				return -1;
			}
			else
			{
				
		        	if (pflag)
				    printf("Field number: %d \n", field);
				//printf("Field number (0 to list): ");
				//scanf("%d", &field);

				if (0 == field)
				{
					int addr;

					for (i = firstField(block); i <= lastField(block); i++)
					{
						int read_value = getField(block, i, buff);

						//printf("read_value is 0x%x\n", read_value);
						addr = Get_ModBus_Register_address(block, i);
						printf("%d\t%d\t:\t%s\t%s\n", addr, i, fnames[i], buff);
					}
				}
				else
				{
					if (!fieldIsInBlock(field, block))
					{
						printf("Field not in block\n");
						return -1;
					}
					else
					{
						//getField(block, field, buff);
						int read_value = getField(block, field, buff);
						//printf("read_value is 0x%x buff %s\n", read_value,buff);

					}
				}
			}
		}
		else
		{
			printf("Not connected\n");
			return -1;
		}

	if (0 != connected)
	{
		outbackClose();
		connected = 0;
		if(pflag)
			printf("Disconnected from %s\n", address);
	}

	return 0;

}

int outback_read_all(){
		//Output Device Table Information
	ConnectAXS();
	if (0 != connected)
	{
		int addr;

		for (i = 0; i < devices; i++)
		{
			block = getBlockType(i);

			switch (block)
			{
				case SUNSPEC_COMMON_MODEL_BLOCK_DID:
					printf("\n*** SunSpec Common Model Block ***\n");
					break;

				case SUNSPEC_OUTBACK_DID:
					printf("\n*** OutBack OutBack Block ***\n");
					break;

				case SUNSPEC_OUTBACK_SYS_CONTROL_DID:
					printf("\n*** OutBack System Control ***\n");
					break;

				case SUNSPEC_BASIC_CC_DID:
					printf("\n*** Basic Charge Controller ***\n");
					break;

				case SUNSPEC_OUTBACK_FM_CC_DID:
					printf("\n*** OutBack FM Charge Controller ***\n");
					break;

				case SUNSPEC_INVERTER_SINGLE_DID:
					printf("\n*** Single Phase Inverter ***\n");
					break;

				case SUNSPEC_INVERTER_SPLIT_DID:
					printf("\n*** Split Phase Inverter ***\n");
					break;

				case SUNSPEC_INVERTER_3PHASE_DID:
					printf("\n*** 3 Phase Inverter ***\n");
					break;

				case SUNSPEC_OUTBACK_FX_DID:
					printf("\n*** OutBack FX Inverter ***\n");
					break;

				case SUNSPEC_OUTBACK_FX_CONFIG_DID:
					printf("\n*** OutBack FX Inverter Configuration ***\n");
					break;

				case SUNSPEC_OUTBACK_GS_CONFIG_DID:
					printf("\n*** OutBack Radian Inverter Configuration ***\n");
					break;

				case SUNSPEC_OUTBACK_GS_SPLIT_DID:
					printf("\n*** OutBack Radian Split Phase Inverter ***\n");
					break;

				case SUNSPEC_OUTBACK_GS_SINGLE_DID:
					printf("\n*** OutBack Radian Single Phase Inverter ***\n");
					break;

				case SUNSPEC_OUTBACK_FNDC_DID:
					printf("\n*** OutBack FLEXnet-DC Battery Monitor ***\n");
					break;

				case SUNSPEC_OUTBACK_FNDC_CONFIG_DID:
					printf("\n*** OutBack FLEXnet-DC Battery Monitor Configuration ***\n");
					break;

				case SUNSPEC_OUTBACK_STATISTICS_DID:
					printf("\n*** OutBack OPTICS Packet Statistics ***\n");
					break;

				case SUNSPEC_END_BLOCK_DID:
					printf("\n*** SunSpec End Block ***\n");
					break;

				default:
					printf("\n*** Empty Block ***\n");
					break;
			} 

			for (j = firstField(i); j <= lastField(i); j++)
			{
				getField(i, j, buff);
				addr = Get_ModBus_Register_address(i, j);

				printf("%d\t%d\t:\t%s\t%s\n", addr, j, fnames[j], buff);
			}
		}
	}
	else
	{
		printf("Not connected\n");
		return -1;
	}
	return 0;
}



int outback_multiple_read(struct multiple_register registers[], int size){
		//Output Device Table Information
	int i = 0, j = 0;
	ConnectAXS();
	
	if (0 != connected)

	{
	
		int addr;
		for (j = 0; j < size-1; j++)
		{
			i = getField(registers[j].block, registers[j].field, buff);
			if (i == 484){
				fprintf(outfp, "\n\"%s\": \"%s\", ",registers[j].str, buff);
			}
			else{ 
	
				fprintf(outfp, "\n\"%s\": \"%s\", ",registers[j].str, buff);
				//return -1;
			}	
		}
	// This duplication is to get rid of the comma at the end
	i = getField(registers[j].block, registers[j].field, buff);
		if (i == 484){
			fprintf(outfp, "\n\"%s\": \"%s\" ",registers[j].str, buff);
		}
		else
			fprintf(outfp, "\n\"%s\": \"%s\" ",registers[j].str, buff);

	}
	else
	{
		
		if(pflag)
			printf("Not connected\n");
		return -1;
	}
	
	if (0 != connected)
	{
		outbackClose();
		connected = 0;
		if(pflag)
			printf("Disconnected from %s\n", address);
	}
	return 0;
}



int outback_write(){

	ConnectAXS();
	if (0 != connected)
	{
		//block = 0;
		//field = 0;
		
		if(pflag)
			printf("Block number: %d",block);
		//scanf("%d", &block);

		if (block < 0 || block > devices)
		{
			printf("Invalid block number\n");
		}
		else
		{
			//printf("Field number (0 to list): ");
			//scanf("%d", &field);

			if (0 == field)
			{
				for (i = firstField(block); i <= lastField(block); i++)
				{
					printf("%d\t:\t%s\n", i, fnames[i]);
				}
			}
			else
			{
				if (!fieldIsInBlock(field, block))
				{
					printf("Field not in block\n");
					return -1;
				}
				else
				{
					memset(buff, 0, sizeof(buff));
					//printf("New Value: %s\n", value_write);
					//scanf("%s", buff);
					for (i = 0; i<BUFF_SIZE;i++){
					    buff[i] = value_write[i];
					}
					if (setFieldWithString(block, field, buff) >= 0)
					{
						 printf("write : successful\n");
					}
					else
					{
					 	 printf("write : failed\n");
					}
				}
			}
		}
	}
	else
	{
		printf("Not connected\n");
	}
	return 0;
}



int ConnectAXS(){
	
	if (0 != connected)
	{
	
		outbackClose();
		if(pflag)
		   printf("Disconnected from %s\n", address);
	#ifdef WIN32
		Sleep(1000);
	#else
		sleep(1);
	#endif
		devices = outbackInit(address, mb_port);
		if(pflag)
		    printf("Reconnected to %s\n", address);
	}
	else
	{
		char *p_ptr;
		if(pflag)
			printf("Connect to: %s\n", address);
		//scanf("%s", address);

		if ('d' == address[0])
		{
			sprintf(address, DEF_SERV_IP_ADDR);
			mb_port = MB_SERV_PORT;
		}
		

		devices = outbackInit(address, mb_port);

		if (devices > 0)
		{
			connected = 1;
			if(pflag)
				printf("Connected successfully to %s:%d\n", address, mb_port);
		}
		else
		{
			outbackClose();
			connected = 0;
			printf("Connection failed\n");
		}
	}
}

int CommCheck(int fdser, int yAddress)
{
 //check for version if its fine.


}


int main(int argc, char *argv[])
{
	
	char *COMMAND = NULL;
	int result = 0;
	
	int mode = GetParams(argc, argv);

	if(mode != -1){
		switch(mode){
			case 'r':
				if(pflag)
	     			    printf("read mode ");
				//result = outback_read();
				print_field(field, block, "custome read");
				break;
			case 'w':
				//printf("write mode ");
				result = outback_write();
				break;
			case 'a':
				if(pflag)
		    		   printf("all mode ");
				   result = PerformMultipleRead(ALL_REG);
				
				break;
			default:
				printf("\n*** Empty Block ***\n");
				break;
		}
	}
	return 0;
}

int PerformOperation(int mode){
		int value = -1, result = -1;
		switch(mode){
		case 'r':
			if (pflag)
			    printf(" Perform operation read mode \n");
			result = outback_read();
			break;
		case 'w':
			if(value==-1)
				return value;
			if(pflag)
				printf("write mode \n");
			result = outback_write();
			break;
		case 'a':
			if(pflag)
				printf("all mode \n");
			result = outback_read_all();
			break;
		default:
			printf("\n*** Empty Block ***\n");
			break;
		
	}
	return result;
}

int PerformMultipleRead(int regtype){
	struct multiple_register* reg;
	int size = 0;
	switch (regtype)
	{
		case INPUT:
			reg = DC_register;
			size = sizeof(DC_register)/sizeof(DC_register[0]);
			break;

		case AC:
			reg = AC_register;
			size = sizeof(reg_all)/sizeof(reg_all[0]);
			break;

		case GRID:
			reg = grid_register;
			size = sizeof(grid_register)/sizeof(grid_register[0]);
			break;
		case ALL_REG:
			reg = reg_all;
			size = sizeof(reg_all)/sizeof(reg_all[0]);
			break;
					
		default:
			if(pflag)
			printf("\n*** Empty Block ***\n");
			break;
	} 
	outback_multiple_read(reg,size);
}

int GetACParam(){
	
	int result=-1;
	bGetEnergy = 4;
	switch(bGetEnergy){
		case POWER:
			print_field(OUTPUT_POWER_FIELD,OUTBACK_RADIAN_SINGLE_PHASE_INVERTER,AC_POWER_STR);
			break;
		case FREQUENCY:
			print_field(AC_FREQUENCY_FIELD,OUTBACK_RADIAN_SINGLE_PHASE_INVERTER,AC_FREQUENCY_STR);
			break;
		case CURRENT:
			print_field(OUTPUT_CURRENT_FIELD,OUTBACK_RADIAN_SINGLE_PHASE_INVERTER,AC_CURRENT_STR);
			break;
		case VOLTAGE:
			print_field(OUTPUT_VOLTAGE_FIELD,OUTBACK_RADIAN_SINGLE_PHASE_INVERTER,AC_VOLTAGE_STR);
			break;		
		case ALL:
			print_field(OUTPUT_VOLTAGE_FIELD,OUTBACK_RADIAN_SINGLE_PHASE_INVERTER,AC_VOLTAGE_STR);
			print_field(OUTPUT_CURRENT_FIELD,OUTBACK_RADIAN_SINGLE_PHASE_INVERTER,AC_CURRENT_STR);
			print_field(AC_FREQUENCY_FIELD,OUTBACK_RADIAN_SINGLE_PHASE_INVERTER,AC_FREQUENCY_STR);
			print_field(OUTPUT_POWER_FIELD,OUTBACK_RADIAN_SINGLE_PHASE_INVERTER,AC_POWER_STR);
			break;
	
		default:
			
			if (pflag)
	                    printf("\n*** Empty Block ***\n");
			break;
	}
}

int GetVerFW(){

	print_field(VERSION_FIELD,SUNSPEC_COMMON_MODEL_BLOCK,"version");
}

int GetTemp(){
	
	block = SINGLE_PHASE_INVERTER;
	field = INTERNAL_TEMP_FIELD;
	
	print_field(INTERNAL_TEMP_FIELD,SINGLE_PHASE_INVERTER,"invertertemperature");
}

int GetSN(){
	
	print_field(VERSION_FIELD,SUNSPEC_COMMON_MODEL_BLOCK,TEMP_SINK_STR);
}

int GetGridParam(){
	
	int result = 0;
	switch(yGetGrid){
		case POWER:
			print_field(GRID_POWER_FIELD,OUTBACK_RADIAN_SINGLE_PHASE_INVERTER,BATT_POWER_STR);
			break;
		case CURRENT:
			field = GRID_CURRENT_FIELD;
			block = OUTBACK_RADIAN_SINGLE_PHASE_INVERTER;
			print_field(field,block,BATT_CURR_STR);
			break;
		case VOLTAGE:
			field = GRID_VOLTAGE_FIELD;
			block = OUTBACK_RADIAN_SINGLE_PHASE_INVERTER;
			print_field(field,block,BATT_VOLT_STR);
			break;		
			
		case ALL:
			field = GRID_CURRENT_FIELD;
			block = OUTBACK_RADIAN_SINGLE_PHASE_INVERTER;
			print_field(field,block,BATT_CURR_STR);
			print_field(GRID_VOLTAGE_FIELD,OUTBACK_RADIAN_SINGLE_PHASE_INVERTER,BATT_VOLT_STR);
			print_field(GRID_POWER_FIELD,OUTBACK_RADIAN_SINGLE_PHASE_INVERTER,BATT_POWER_STR);
			break;
			
		default:
			printf("\n*** Empty Block ***\n");
			break;
	}	
}


int GetInputParam(){
	
	int result = 0;
	switch(yGetInput){
		case VOLTAGE:
			
			print_field(INPUT_VOLTAGE_FIELD,SINGLE_PHASE_INVERTER,DC_VOLTAGE_STR);
			break;
		case CURRENT:
			
			print_field(INPUT_CURRENT_FIELD,SINGLE_PHASE_INVERTER,DC_CURRENT_STR);
			break;
		case POWER:
			
			print_field(INPUT_POWER_FIELD,SINGLE_PHASE_INVERTER,DC_POWER_STR);
			break;		
		
		case ALL:
			print_field(INPUT_POWER_FIELD,SINGLE_PHASE_INVERTER,DC_POWER_STR);
			print_field(INPUT_VOLTAGE_FIELD,SINGLE_PHASE_INVERTER,DC_VOLTAGE_STR);
			print_field(INPUT_CURRENT_FIELD,SINGLE_PHASE_INVERTER,DC_CURRENT_STR);

			break;
			
		default:
			printf("\n*** Empty Block ***\n");
			break;
	}
}

int print_field(int field_par, int block_par, char* print_str){
	field = field_par;
	block = block_par;
		
	int result = PerformOperation('r');
	if (result == 0){
               fprintf(outfp, "\n\"%s\": %s, \n",print_str, buff);
       }
       else{ 
               //fprintf(outfp, "\n\"%s\": %s, \n",print_str, ERROR);
               fprintf(outfp, "\n\"%s\": %s, \n",print_str, ERROR);
               return -1;
       }
}

