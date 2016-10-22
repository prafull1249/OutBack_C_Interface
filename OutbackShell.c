/*	OutBackShell	version 0.92	Mar. 1, 2012
 *
 *	Written by Daniel Lloyd. 2011-2012.
 *	Property of OutBack Power Systems Inc.
 */

#include <stdio.h>

#ifndef WIN32
#include <unistd.h>
#endif

#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <math.h>
#include <time.h>

#include "outback.h"

#define MB_SERV_PORT		502
#define DEF_SERV_IP_ADDR	"192.168.2.220"

int main(void)
{
	int	devices	= 0;
	int	connected = 0;
	char buff[128];
	char address[16];
	int	i, j;
	char command = 0;
	int	block;
	int	field;
	int mb_port = MB_SERV_PORT;
	
	printf("OutBack Commnunications Shell\n\n");
	printf("Version 3.0.105\n");
	printf("Type command or 'h' for help\n");
	
	while ('q' != command)
	{		
		printf(">> ");
		scanf("%s", buff);
		command = buff[0];
		
		switch (command)
		{
			case 'H':
			case 'h':
				printf("Help\t\t'h'\n");
				printf("Connect\t\t'c'\n");
				printf("Disconnect\t'd'\n");
				printf("List Blocks\t'l'\n");
				printf("Read All\t'a'\n");
				printf("Read\t\t'r'\n");
				printf("Write\t\t'w'\n");
				printf("Write Multiple\t'm'\n");
				printf("Unlock\t\t'u'\n");
				printf("List Errors\t'e'\n");
				printf("Quit\t\t'q'\n\n");
				printf("To connect to the default IP address,\n");
				printf("enter 'c' then 'd'. Default is defined\n");
				printf("as DEF_SERV_IP_ADDR in OutBackShell.c.\n\n");
				printf("To disconnect and automatically\n");
				printf("reconnect to the same address, enter 'c'\n");
				printf("while connected.\n\n");
				
				break;
				
			case 'C':
			case 'c':
				if (0 != connected)
				{
					outbackClose();
					printf("Disconnected from %s\n", address);
#ifdef WIN32
					Sleep(1000);
#else
					sleep(1);
#endif
					devices = outbackInit(address, mb_port);
					printf("Reconnected to %s\n", address);
				}
				else
				{
				 	char *p_ptr;
				
					printf("Connect to: ");
					scanf("%s", address);
					
					if ('d' == address[0])
					{
						sprintf(address, DEF_SERV_IP_ADDR);
						mb_port = MB_SERV_PORT;
					}
					
					if (NULL != (p_ptr = strchr(address,':')))		// look for ':' indicating non default modbus port
					{
						*p_ptr = 0;									// make ip address a separate string
						++p_ptr;									// point to port
						mb_port = atoi(p_ptr);
						
						if ((mb_port <= 0) || (mb_port > 665535))	// ensure port number is valid
						{
							mb_port = MB_SERV_PORT;
						}						
					}
					
					devices = outbackInit(address, mb_port);
					
					if (devices > 0)
					{
						connected = 1;
						printf("Connected successfully to %s:%d\n", address, mb_port);
					}
					else
					{
						outbackClose();
						connected = 0;
						printf("Connection failed\n");
					}
				}
				break;
				
			case 'D':
			case 'd':
				if (0 != connected)
				{
					outbackClose();
					devices = 0;
					connected = 0;
					printf("Disconnected from %s\n", address);
				}
				else
				{
					printf("Not currently connected.\n");
				}
				break;
				
			case 'L':
			case 'l':
				//Output Device Blocks
				if (0 != connected)
				{
					for (i = 0; i < devices; i++)
					{
						printf("%d\t", i);
						block = getBlockType(i);

						switch (block)
						{
							case SUNSPEC_COMMON_MODEL_BLOCK_DID:
								printf("SunSpec Common Model Block\n");
								break;
								
							case SUNSPEC_OUTBACK_DID:
								printf("OutBack Gateway\n");
								break;
								
							case SUNSPEC_OUTBACK_SYS_CONTROL_DID:
								printf("OutBack System Control\n");
								break;
															
							case SUNSPEC_BASIC_CC_DID:
								printf("Basic Charge Controller\n");
								break;
								
							case SUNSPEC_OUTBACK_FM_CC_DID:
								printf("OutBack FM Charge Controller\n");
								break;
								
							case SUNSPEC_INVERTER_SINGLE_DID:
								printf("Single Phase Inverter\n");
								break;
								
							case SUNSPEC_INVERTER_SPLIT_DID:
								printf("Split Phase Inverter\n");
								break;
								
							case SUNSPEC_INVERTER_3PHASE_DID:
								printf("3 Phase Inverter\n");
								break;
								
							case SUNSPEC_OUTBACK_FX_DID:
								printf("OutBack FX Inverter\n");
								break;
							
							case SUNSPEC_OUTBACK_FX_CONFIG_DID:
								printf("OutBack FX Inverter Configuration\n");
								break;
								
							case SUNSPEC_OUTBACK_GS_CONFIG_DID:
								printf("OutBack Radian Inverter Configuration\n");
								break;
								
							case SUNSPEC_OUTBACK_GS_SPLIT_DID:
								printf("OutBack Radian Split Phase Inverter\n");
								break;
								
							case SUNSPEC_OUTBACK_GS_SINGLE_DID:
								printf("OutBack Radian Single Phase Inverter\n");
								break;
								
							case SUNSPEC_OUTBACK_FNDC_DID:
								printf("OutBack FLEXnet-DC Battery Monitor\n");
								break;
							
							case SUNSPEC_OUTBACK_FNDC_CONFIG_DID:
 								printf("OutBack FLEXnet-DC Battery Monitor Configuration\n");
								break;

							case SUNSPEC_OUTBACK_STATISTICS_DID:
								printf("OutBack OPTICS Packet Statistics\n");
								break;
															
							case SUNSPEC_END_BLOCK_DID:
								printf("SunSpec End Block\n");
								break;
								
							default:
								printf("Empty Block\n");
								break;
						}
					}					
				}
				else
				{
					printf("Not connected\n");
				}
				break;
				
			case 'A':
			case 'a':
				//Output Device Table Information
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
				}
				break;
				
			case 'R':
			case 'r':
				
				if (0 != connected)
				{
					block = 0;
					field = 0;
					
					printf("Block number: ");
					scanf("%d", &block);
					
					if ( (block < 0) || (block > devices) )
					{
						printf("Invalid block number\n");
					}
					else
					{						
						printf("Field number (0 to list): ");
						scanf("%d", &field);
						
						if (0 == field)
						{
							int addr;

							for (i = firstField(block); i <= lastField(block); i++)
							{
								int read_value = getField(block, i, buff);
								
								printf("read_value is 0x%x\n", read_value);
								addr = Get_ModBus_Register_address(block, i);
								printf("%d\t%d\t:\t%s\t%s\n", addr, i, fnames[i], buff);
 							}
						}
						else
						{
							if (!fieldIsInBlock(field, block))
							{
								printf("Field not in block\n");
								break;
							}
							else
							{
								//getField(block, field, buff);
								int read_value = getField(block, field, buff);
								printf("read_value is 0x%x\n", read_value);
								
								printf("%s\n", buff);
							}
						}
					}
				}
				else
				{
					printf("Not connected\n");
				}
				
				break;
				
			case 'W':
			case 'w':
				
				if (0 != connected)
				{
					block = 0;
					field = 0;
					
					printf("Block number: ");
					scanf("%d", &block);
					
					if (block < 0 || block > devices)
					{
						printf("Invalid block number\n");
					}
					else
					{						
						printf("Field number (0 to list): ");
						scanf("%d", &field);

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
								break;
							}
							else
							{
								memset(buff, 0, sizeof(buff));
								printf("New Value: ");
								scanf("%s", buff);

								if (setFieldWithString(block, field, buff) >= 0)
								{
									printf("Write successful\n");
								}
								else
								{
									printf("Write failed\n");
								}
							}
						}
					}
				}
				else
				{
					printf("Not connected\n");
				}
				
				break;

			case 'M':
			case 'm':
				
				if (0 != connected)
				{
					block = 0;
					field = 0;
					
					printf("Block number: ");
					scanf("%d", &block);
					
					if (block < 0 || block > devices)
					{
						printf("Invalid block number\n");
					}
					else
					{						
						printf("Starting Field number (0 to list): ");
						scanf("%d", &field);

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
								break;
							}
							else
							{
								memset(buff, 0, sizeof(buff));
								printf("Enter Values Seperated by commas: ");
								scanf("%s", buff);

								if (setFieldsWithString(block, field, buff) >= 0)
								{
									printf("Write successful\n");
								}
								else
								{
									printf("Write failed\n");
								}
							}
						}
					}
				}
				else
				{
					printf("Not connected\n");
				}
				
				break;

				
			case 'U':
			case 'u':
				
				if (0 != connected)
				{					
					printf("Password: ");
					scanf("%s", buff);

					if ((0 == strcmp(buff, "clear")) || (0 == strcmp(buff, "CLEAR")))
					{					
						if (1 == writeSunSpecRegister(1, OutBack_Write_Password, 0))
						{
							printf("Password cleared\n");
						}
						else
						{
							printf("Password clear failed\n");
						}
					}
					else
					{
						if (setFieldWithString(1, OutBack_Write_Password, buff))
						{
							printf("Write successful\n");
						}
						else
						{
							printf("Write failed\n");
						}
					}
				}
				else
				{
					printf("Not connected\n");
				}
				
				break;
				
			case 'E':
			case 'e':
				
				if (0 != connected)
				{
					uint16 errorField;
					
					errorField = getField(1, OutBack_Error, buff);
					
					if (0 != errorField)
					{
						if (OUTBACK_READ_ERROR(errorField, ERR_LAST_WRITE_LIMIT_HIGH))
						{
							printf("ERROR: Last Write Limit High\n");
						}
						
						if (OUTBACK_READ_ERROR(errorField, ERR_LAST_WRITE_LIMIT_LOW))
						{
							printf("ERROR: Last Write Limit Low\n");
						}
						
						if (OUTBACK_READ_ERROR(errorField, ERR_LAST_WRITE_INVALID))
						{
							printf("ERROR: Last Write Invalid Value\n");
						}
						
						if (OUTBACK_READ_ERROR(errorField, ERR_DHCP_FAILED))
						{
							printf("ERROR: DHCP Failed\n");
						}
						
						if (OUTBACK_READ_ERROR(errorField, ERR_DNS_RESOLVE_FAILED))
						{
							printf("ERROR: DNS Resolve Failed\n");
						}
						
						if (OUTBACK_READ_ERROR(errorField, ERR_SMTP_AUTH_FAILED))
						{
							printf("ERROR: SMTP Authorization Failed\n");
						}
						
						if (OUTBACK_READ_ERROR(errorField, ERR_SMTP_SEND_FAILED))
						{
							printf("ERROR: SMTP Send Failed\n");
						}
						
						if (OUTBACK_READ_ERROR(errorField, ERR_FTP_ERROR))
						{
							printf("ERROR: FTP Error\n");
						}
						
						if (OUTBACK_READ_ERROR(errorField, ERR_SD_CARD_ERROR))
						{
							printf("ERROR: SC Card Error\n");
						}
						
						if (OUTBACK_READ_ERROR(errorField, ERR_SNTP_FAILED))
						{
							printf("ERROR: SNTP Failed\n");
						}
						
						if (OUTBACK_READ_ERROR(errorField, ERR_WRITE_WHILE_LOCKED))
						{
							printf("ERROR: Write attempt while locked.\nUnlock with write password (command 'u')\n");
						}

						if (OUTBACK_READ_ERROR(errorField, ERR_DEVICE_FW_UPDATE_INVALID))
						{
							printf("ERROR: Device Firmware Update Invalid\n");
						}
						
						if (OUTBACK_READ_ERROR(errorField, ERR_DEVICE_FW_UPDATE_FILE_NOT_FOUND))
						{
							printf("ERROR: Device Firmware Update File Not Found\n");
						}
						
						if (OUTBACK_READ_ERROR(errorField, ERR_DEVICE_FW_UPDATE_FILE_INVALID))
						{
							printf("ERROR: Device Firmware Invalid Update File\n");
						}

						if (OUTBACK_READ_ERROR(errorField, ERR_DEVICE_FW_UPDATE_FAILURE))
						{
							printf("ERROR: Device Firmware Update Failure\n");
						}

						if (OUTBACK_READ_ERROR(errorField, ERR_OPERATION_NOT_SUPPORTED))
						{
							printf("ERROR: Device Unsupported Operation\n");
						}
						
						if (OUTBACK_DEVICE_ERROR(errorField))
						{
							printf("ERROR: Error reported on device %u\n", OUTBACK_DEVICE_ERROR(errorField));
						}						
					}
					else
					{
						printf("No Errors Reported\n");
					}
				}
				else
				{
					printf("Not connected\n");
				}
				
				break;
								
			default:
				break;
		}
	}
	
	if (0 != connected)
	{
		outbackClose();
		printf("Disconnected from %s\n", address);
	}
	
	printf("Exited successfully\n");
	
	return 0;
}
