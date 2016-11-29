/*	outback.c	version 0.98	Jam. 7, 2014
 *
 *	Written by Daniel Lloyd. 2011-2014.
 *	Property of OutBack Power Inc.
 */


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <time.h>
#include <unistd.h>

#ifdef WIN32
#include <winsock2.h>
#else
#include <arpa/inet.h>
#endif

#include "modbus.h"
#include "outback.h"
#include "OBencrypt.h"

#define MB_MAX_REGISTERS			(125)
#define SUNSPEC_MODBUS_REGISTER_OFFSET		(40001)

#define SUNSPEC_MODBUS_MAP_ID				(0x53756e53)
#define SUNSPEC_MODBUS_MAP_ID_MSW			(SUNSPEC_MODBUS_MAP_ID >> 16)
#define SUNSPEC_MODBUS_MAP_ID_LSW			(SUNSPEC_MODBUS_MAP_ID & 0x0000FFFF)

#define BSwap32(a)   ( ((((uint32) (a)) << 24)&0xFF000000UL) | \
                       ((((uint32) (a)) <<  8)&0x00FF0000UL) | \
                       ((((uint32) (a)) >>  8)&0x0000FF00UL) | \
                       ((((uint32) (a)) >> 24)&0x000000FFUL) )


#define VOLTAGE_SF							(-1)
#define AMPERAGE_SF							(-1)
#define HOURS_SF							(-1)
#define KWH_SF								(-1)
#define NI_SF								(0x80)

#define NI_INT16							(0x8000)
#define NI_UINT16							(0xFFFF)
#define NI_ACC16							(0x0000)
#define NI_INT32							(0x80000000)
#define NI_UINT32							(0xFFFFFFFF)
#define NI_ACC32							(0x00000000)
#define NI_INT64							(0x8000000000000000)
#define NI_UINT64							(0xFFFFFFFFFFFFFFFF)
#define NI_ACC64							(0x0000000000000000)
#define NI_STRING							(0x0000)
#define NI_FLOAT32							(0x7FC00000)

#define C_STATUS_NORMAL						(0x00000000)
#define C_STATUS_ERROR						(0xFFFFFFFE)
#define C_STATUS_UNK						(0xFFFFFFFF)

#define ADDR_START							(SUNSPEC_MODBUS_REGISTER_OFFSET-1)


const char		CC_charge_mode[5][7]			= {"Silent","Float","Bulk","Absorb","EQ"};
const char		CC_aux_cntl[3][5]				= {"Off","Auto","On"};
const char		CC_AUX_op_Modes[3][5]			= {"Off","On","Auto"};
const char		AGS_op_states[6][12]			= {"Stopped", "Starting", "Running", "Warm Up", "Cool Down", "Awaiting AC"};
const char		HBX_op_modes[4][15]				= {"Disabled", "Voltage", "SOC", "Voltage or SOC"};
const char		disabled_enabled_text[2][9]		= {"Disabled","Enabled"};
const char		CC_mppt_mode[3][7]				= {"Auto","U-Pick","Wind"};
const char		CC_aux_polarity[2][5]			= {"High", "Low"};
const char		CC_aux_modes[9][20]				= {"Float","Diversion: Relay","Diversion: Solid St","Low Batt Disconnect","Remote","Vent Fan","PV Trigger","Error Output","Night Light"};
const char		CC_sweep_width_modes[2][5]		= {"Full","Half"};
const char		CC_temp_comp_modes[2][8]		= {"Wide","Limited"};
const char		CC_auto_restart_modes[3][36]	= {"Off","Every 90 Minutes","Every 90 Minutes if Absorb or Float"};
const uint16	CC_sweep_max[4]					= {80,85,90,99};
const char		I_status_mode[9][16]			= {"Not implemented","Off","Sleeping","Starting up","MPPT","Throttled","Shutting down","Fault","Standby"};
const char		I_status_vendor[16][16]			= {"Off","Searching","Inverting","Charging","Silent","Float Charging","EQ Charging","Charger Off","Support","Sell",
                                              	   "Pass Thru","","","","Offsetting"};
const char		AC_input_select_text[2][10]		= {"Grid", "Generator"};
const char		Inv_charger_op_mode_text[2][32]	= {"All Inverter Charging Disabled", "Bulk and Float Charging Enabled"};
const char 		*FX_charger_ctrl_mode_text[3]	= {"All Inverter Charging Disabled","Automatic Charging Enabled","Bulk and Float Charging Enabled"};
const char		FX_mode[16][12] 				= {"Off","Searching","Inverting","Charging","Silent","Float","EQ","Charger Off","Support","Sell","PassThru","Slave On","Slave Off","","Offsetting"};
const char		GS_AC_in_mode_text[6][10]		= {"Generator", "Support", "Grid Tied", "UPS" , "Backup", "Mini Grid"};
const char		GS_AUX_Relay_Modes_text[10][14]	= {"Load Shed", "Gen Alert", "Fault", "Vent Fan", "Cool Fan",
                                      			   "DC Divert", "IEEE", "Source Status", "AC Divert", "Batt Cut-Out"};
const char		*FX_AUX_Modes[9] 				= {"Remote", "Load Shed", "Gen Alert", "Fault", "Vent Fan", "Cool Fan", "Divert DC", "Divert AC","AC Drop"};
const char		GS_stacking_modes_text[9][15]	= {"Master","      ","Slave","Slave","","","","B Phase Master","C Phase Master"};
const char 		*FX_stacking_modes[20] 			= {"1-2phase Master","Classic Slave","OB Slave L1","OB Slave L2","3phase Master","3phase Slave","","","","",
													"Master","Classic Slave","OB Slave L1","OB Slave L2","3phase OB Slave A","3phase OB Slave B",
													"3phase OB Slave C","3phase Classic B","3phase Classic C","Independent"};		
const char		Grid_tie_window_text[2][5]		= {"IEEE","User"};
const char		AC_input_mode_text[2][5]		= {"Drop","Use"};
const char		*Day_of_week_text[7]			= {"Sun","Mon","Tue","Wed","Thr","Fri","Sat"};
const char		Month_text[12][4]				= {"Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"};
const char 		GS_module_text[4][6]			= {"Auto", "Left", "Right", "Both"};
const char 		*GS_AC_in_mode[8]				= {"---------", "Generator", "Support", "Grid Tied", "UPS" , "Backup", "Mini Grid", "Grid Zero"};
const char		FXR_sealed_vented_text[2][7]	= {"Vented", "Sealed"};
const char		*GS_dual_single_module_text[2]	= {"Dual Module", "Single Module"};

const uint16 	SD_card_logging_interval_table[11] = {1, 2, 3, 4, 5, 6, 10, 15, 20, 30, 60};	// seconds

//Prafull 

extern int pflag;
//field types
typedef enum {
	INT16_T,
	UINT16_T,
	ACC16_T,
	INT32_T,
	UINT32_T,
	ACC32_T,
	INT64_T,
	UINT64_T,
	ACC64_T,
	STRING_T,
	FLOAT32_T
} field_type;

//units
typedef enum {
	NI_U,
	REGISTERS_U,
	ENUMERATED_U,
	BITFIELD_U,
	PERCENTAGE_U,
	VOLTS_U,
	AMPS_U,
	WATTS_U,
	VA_U,
	VAR_U,
	AH_U,
	KAH_U,
	WH_U,
	KWH_U,
	KW_U,
	DEGREES_C_U,
	HERTZ_U,
	SECS_U,
	MINS_U,
	HOURS_U,
	DAYS_U,
	ADDR_U,
	CYCLES_U,
	TIME_U
}
field_units;


typedef struct meta_data
{
	uint32	position		:9;		//field's address offset from beginning of block in registers
    uint32	size			:5;		//number of registers required for field
	uint32	type			:4;		//data type for field (see field_type enum in sunspec.h)
    uint32	units			:5;		//units for field's data, if applicable
	uint16	scaleFactor		:10;	//scale factor for field's data, if applicable
    uint16	write			:1;		//write permission
    uint16	read			:1;		//read permission
}
meta_data;

//md[] is an array of meta_data structs for each field for each block. The array
//is accessed using the name of the field, which is enumerated in sunspec.h as type SunSpecField
const meta_data md[] =
{
	{ 1,	2,	UINT32_T,	NI_U,			NI_SF,					0,	1 },	//	0	C_SunSpec_ID,
	{ 3,	1,	UINT16_T,	NI_U,			NI_SF,					0,	1 },	//	1	C_SunSpec_DID,
	{ 4,	1,	UINT16_T,	REGISTERS_U,	NI_SF,					0,	1 },	//	2	C_SunSpec_Length,
	{ 5,	16,	STRING_T,	NI_U,			NI_SF,					0,	1 },	//	3	C_Manufacturer,
	{ 21,	16,	STRING_T,	NI_U,			NI_SF,					0,	1 },	//	4	C_Model,
	{ 37,	8,	STRING_T,	NI_U,			NI_SF,					0,	1 },	//	5	C_Options,
	{ 45,	8,	STRING_T,	NI_U,			NI_SF,					0,	1 },	//	6	C_Version,
	{ 53,	16,	STRING_T,	NI_U,			NI_SF,					0,	1 },	//	7	C_SerialNumber,
	{ 69,	1,	UINT16_T,	NI_U,			NI_SF,					1,	1 },	//	8	C_DeviceAddress,
//
	{ 1,	1,	UINT16_T,	NI_U,			NI_SF,					0,	1 },	//	9	OutBack_SunSpec_DID = 64110,
	{ 2,	1,	UINT16_T,	REGISTERS_U,	NI_SF,					0,	1 },	//	10	OutBack_SunSpec_Length,
	{ 3,	1,	UINT16_T,	NI_U,			NI_SF,					0,	1 },	//	11	OutBack_Major_Firmware_Number,
	{ 4,	1,	UINT16_T,	NI_U,			NI_SF,					0,	1 },	//	12	OutBack_Mid_Firmware_Number,
	{ 5,	1,	UINT16_T,	NI_U,			NI_SF,					0,	1 },	//	13	OutBack_Minor_Firmware_Number,
	{ 6,	1,	UINT16_T,	NI_U,			NI_SF,					0,	1 },	//	14	OutBack_Encryption_Key,
	{ 7,    7,	STRING_T,   NI_U,			NI_SF,					0,  1 },    //  15  OutBack_MAC_Address,
	{ 14,	OUTBACK_PASSWORD_SIZE,		STRING_T,	NI_U,	NI_SF,	1,	0 },	//	16	OutBack_Write_Password,
	{ 22,	1,	UINT16_T,	ENUMERATED_U,	NI_SF,					1,	1 },	//	17	OutBack_Enable_DHCP,
	{ 23,	2,	UINT32_T,	ADDR_U,			NI_SF,					1,	1 },	//	18	OutBack_TCPIP_Address,
	{ 25,	2,	UINT32_T,	ADDR_U,			NI_SF,					1,	1 },	//	19	OutBack_TCPIP_Gateway,
	{ 27,	2,	UINT32_T,	ADDR_U,			NI_SF,					1,	1 },	//	20	OutBack_TCPIP_Netmask,
	{ 29,	2,	UINT32_T,	ADDR_U,			NI_SF,					1,	1 },	//	21	OutBack_TCPIP_DNS_1,
	{ 31,	2,	UINT32_T,	ADDR_U,			NI_SF,					1,	1 },	//	22	OutBack_TCPIP_DNS_2,
	{ 33,	1,	UINT16_T,	NI_U,			NI_SF,					1,	1 },	//	23	OutBack_Modbus_Port,
	{ 34,	SMTP_SERVER_NAME_SIZE,		STRING_T,	NI_U,	NI_SF,	1,	1 },	//	24	OutBack_SMTP_Server_Name,
	{ 54,	SMTP_ACCOUNT_NAME_SIZE,		STRING_T,	NI_U,	NI_SF,	1,	1 },	//	25	OutBack_SMTP_Account_Name,
	{ 70,	1,	UINT16_T,	ENUMERATED_U,	NI_SF,					1,	1,},	//	26	OutBack_SMTP_SSL_Enable,
	{ 71,	SMTP_EMAIL_PASSWORD_SIZE,	STRING_T,	NI_U,	NI_SF,	1,	1 },	//	27	OutBack_SMTP_Email_Password,
	{ 79,	SMTP_EMAIL_USER_NAME_SIZE,	STRING_T,	NI_U,	NI_SF,	1,	1 },	//	28	OutBack_SMTP_Email_User_Name,
	{ 99,	1,	UINT16_T,	NI_U,			NI_SF,					1,	1 },	//	29	OutBack_Status_Email_Interval,
	{ 100,	1,	UINT16_T,	NI_U,			NI_SF,					1,	1 },	//	30	OutBack_Status_Email_Status_Time,
	{ 101,	STATUS_EMAIL_SUBJECT_SIZE,	STRING_T,	NI_U,	NI_SF,	1,	1 },	//	31	OutBack_Status_Email_Subject_Line,
	{ 126,	STATUS_EMAIL_TO_ADDR1_SIZE,	STRING_T,	NI_U,	NI_SF,	1,	1 },	//	32	OutBack_Status_Email_To_Address_1,
	{ 146,	STATUS_EMAIL_TO_ADDR2_SIZE,	STRING_T,	NI_U,	NI_SF,	1,	1 },	//	33	OutBack_Status_Email_To_Address_2,
	{ 166,	1,	UINT16_T,	NI_U,			NI_SF,					1,	1 },	//	34	OutBack_Alarm_Email_Enable,
	{ 167,	ALARM_EMAIL_SUBJECT_SIZE,	STRING_T,	NI_U,	NI_SF,	1,	1 },	//	35	OutBack_Alarm_Email_Subject_Line,
	{ 192,	ALARM_EMAIL_TO_ADDR1_SIZE,	STRING_T,	NI_U,	NI_SF,	1,	1 },	//	36	OutBack_Alarm_Email_To_Address_1,
	{ 212,	ALARM_EMAIL_TO_ADDR2_SIZE,	STRING_T,	NI_U,	NI_SF,	1,	1 },	//	37	OutBack_Alarm_Email_To_Address_2,
	{ 232,	FTP_PASSWORD_SIZE,			STRING_T,	NI_U,	NI_SF,	1,	0 },	//	38	OutBack_FTP_Password,
	{ 240,	TELNET_PASSWORD_SIZE,		STRING_T,	NI_U,	NI_SF,	1,	0 },	//	39	OutBack_Telnet_Password,
	{ 248,	1,	UINT16_T,	ENUMERATED_U,	NI_SF,					1,	1 },	//	40	OutBack_SD_Card_Log_Write_Interval,
	{ 249,	1,	UINT16_T,	NI_U,			NI_SF,					1,	1 },	//	41	OutBack_SD_Card_Log_Retain_Days,
	{ 250,	1,	UINT16_T,	ENUMERATED_U,	NI_SF,					1,	1 },	//	42	OutBack_SD_Card_Logging_Mode,
	{ 251,	TIME_SERVER_NAME_SIZE,		STRING_T,	NI_U,	NI_SF,	1,	1 },	//	43	OutBack_Time_Server_Name,
	{ 271,	1,	UINT16_T,	ENUMERATED_U,	NI_SF,					1,	1 },	//	44	OutBack_Enable_Time_Server,
	{ 272,	1,	INT16_T,	NI_U,			NI_SF,					1,	1 },	//	45	OutBack_Set_Time_Zone,
	{ 273,	1,	UINT16_T,	ENUMERATED_U,	NI_SF,					1,	1 },	//	46	OutBack_Enable_Float_Coordination,
	{ 274,	1,	UINT16_T,	ENUMERATED_U,	NI_SF,					1,	1 },	//	47	OutBack_Enable_FNDC_Charge_Termination,
	{ 275,	1,	UINT16_T,	ENUMERATED_U,	NI_SF,					1,	1 },	//	48	OutBack_Enable_FNDC_Grid_Tie_Control,
	{ 276,	1,	INT16_T,	NI_U,			NI_SF,					0,	1 },	//	49	OutBack_Voltage_SF,
	{ 277,	1,	INT16_T,	NI_U,			NI_SF,					0,	1 },	//	50	OutBack_Hour_SF,
	{ 278,	1,	UINT16_T,	ENUMERATED_U,	NI_SF,					1,	1 },	//	51	OutBack_AGS_Mode,
	{ 279,	1,	UINT16_T,	NI_U,			NI_SF,					1,	1 },	//	52	OutBack_AGS_Port,
	{ 280,	1,	UINT16_T,	ENUMERATED_U,	NI_SF,					1,	1 },	//	53	OutBack_AGS_Port_Type,
	{ 281,	1,	UINT16_T,	ENUMERATED_U,	NI_SF,					1,	1 },	//	54	OutBack_Generator_Type,
	{ 282,	1,	UINT16_T,	VOLTS_U,		OutBack_Voltage_SF,		1,	1 },	//	55	OutBack_AGS_DC_Gen_Absorb_Voltage,
	{ 283,	1,	UINT16_T,	HOURS_U,		OutBack_Hour_SF,		1,	1 },	//	56	OutBack_AGS_DC_Gen_Absorb_Time,
	{ 284,	1,	UINT16_T,	MINS_U,			NI_SF,					1,	1 },	//	57	OutBack_AGS_Fault_Time,
	{ 285,	1,	UINT16_T,	MINS_U,			NI_SF,					1,	1 },	//	58	OutBack_AGS_Gen_Cool_Down_Time,
	{ 286,	1,	UINT16_T,	MINS_U,			NI_SF,					1,	1 },	//	59	OutBack_AGS_Gen_Warm_Up_Time,
	{ 287,	1,	UINT16_T,	ENUMERATED_U,	NI_SF,					1,	1 },	//	60	OutBack_Generator_Exercise_Mode,
	{ 288,	1,	UINT16_T,	HOURS_U,		NI_SF,					1,	1 },	//	61	OutBack_Exercise_Start_Hour,
	{ 289,	1,	UINT16_T,	MINS_U,			NI_SF,					1,	1 },	//	62	OutBack_Exercise_Start_Minute,
	{ 290,	1,	UINT16_T,	ENUMERATED_U,	NI_SF,					1,	1 },	//	63	OutBack_Exercise_Day,
	{ 291,	1,	UINT16_T,	MINS_U,			NI_SF,					1,	1 },	//	64	OutBack_Exercise_Period,
	{ 292,	1,	UINT16_T,	NI_U,			NI_SF,					1,	1 },	//	65	OutBack_Exercise_Interval,
	{ 293,	1,	UINT16_T,	ENUMERATED_U,	NI_SF,					1,	1 },	//	66	OutBack_AGS_Sell_Mode,
	{ 294,	1,	UINT16_T,	ENUMERATED_U,	NI_SF,					1,	1 },	//	67	OutBack_AGS_2_Min_Start_Mode,
	{ 295,	1,	UINT16_T,	VOLTS_U,		OutBack_Voltage_SF,		1,	1 },	//	68	OutBack_AGS_2_Min_Start_Voltage,
 	{ 296,	1,	UINT16_T,	ENUMERATED_U,	NI_SF,					1,	1 },	//	69	OutBack_AGS_2_Hour_Start_Mode,
	{ 297,	1,	UINT16_T,	VOLTS_U,		OutBack_Voltage_SF,		1,	1 },	//	70	OutBack_AGS_2_Hour_Start_Voltage,
 	{ 298,	1,	UINT16_T,	ENUMERATED_U,	NI_SF,					1,	1 },	//	71	OutBack_AGS_24_Hour_Start_Mode,
	{ 299,	1,	UINT16_T,	VOLTS_U,		OutBack_Voltage_SF,		1,	1 },	//	72	OutBack_AGS_24_Hour_Start_Voltage,
 	{ 300,	1,	UINT16_T,	ENUMERATED_U,	NI_SF,					1,	1 },	//	73	OutBack_AGS_Load_Start_Mode,
 	{ 301,	1,	UINT16_T,	KW_U,			NI_SF,					1,	1 },	//	74	OutBack_AGS_Load_Start_kW,
	{ 302,	1,	UINT16_T,	MINS_U,			NI_SF,					1,	1 },	//	75	OutBack_AGS_Load_Start_Delay,
 	{ 303,	1,	UINT16_T,	KW_U,			NI_SF,					1,	1 },	//	76	OutBack_AGS_Load_Stop_kW,
	{ 304,	1,	UINT16_T,	MINS_U,			NI_SF,					1,	1 },	//	77	OutBack_AGS_Load_Stop_Delay,
 	{ 305,	1,	UINT16_T,	ENUMERATED_U,	NI_SF,					1,	1 },	//	78	OutBack_AGS_SOC_Start_Mode,
 	{ 306,	1,	UINT16_T,	PERCENTAGE_U,	NI_SF,					1,	1 },	//	79	OutBack_AGS_SOC_Start_Percentage,
 	{ 307,	1,	UINT16_T,	PERCENTAGE_U,	NI_SF,					1,	1 },	//	80	OutBack_AGS_SOC_Stop_Percentage,
  	{ 308,	1,	UINT16_T,	ENUMERATED_U,	NI_SF,					1,	1 },	//	81	OutBack_AGS_Enable_Full_Charge_Mode,
  	{ 309,	1,	UINT16_T,	DAYS_U,			NI_SF,					1,	1 },	//	82	OutBack_AGS_Full_Charge_Interval,
  	{ 310,	1,	UINT16_T,	ENUMERATED_U,	NI_SF,					1,	1 },	//	83	OutBack_AGS_Must_Run_Mode,
  	{ 311,	1,	UINT16_T,	HOURS_U,		NI_SF,					1,	1 },	//	84	OutBack_AGS_Must_Run_Weekday_Start_Hour,
  	{ 312,	1,	UINT16_T,	MINS_U,			NI_SF,					1,	1 },	//	85	OutBack_AGS_Must_Run_Weekday_Start_Minute,
  	{ 313,	1,	UINT16_T,	HOURS_U,		NI_SF,					1,	1 },	//	86	OutBack_AGS_Must_Run_Weekday_Stop_Hour,
  	{ 314,	1,	UINT16_T,	MINS_U,			NI_SF,					1,	1 },	//	87	OutBack_AGS_Must_Run_Weekday_Stop_Minute,
   	{ 315,	1,	UINT16_T,	HOURS_U,		NI_SF,					1,	1 },	//	88	OutBack_AGS_Must_Run_Weekend_Start_Hour,
  	{ 316,	1,	UINT16_T,	MINS_U,			NI_SF,					1,	1 },	//	89	OutBack_AGS_Must_Run_Weekend_Start_Minute,
  	{ 317,	1,	UINT16_T,	HOURS_U,		NI_SF,					1,	1 },	//	90	OutBack_AGS_Must_Run_Weekend_Stop_Hour,
  	{ 318, 	1,	UINT16_T,	MINS_U,			NI_SF,					1,	1 },	//	91	OutBack_AGS_Must_Run_Weekend_Stop_Minute,
  	{ 319,	1,	UINT16_T,	ENUMERATED_U,	NI_SF,					1,	1 },	//	92	OutBack_AGS_Quiet_Time_Mode,
  	{ 320,	1,	UINT16_T,	HOURS_U,		NI_SF,					1,	1 },	//	93	OutBack_AGS_Quiet_Time_Weekday_Start_Hour,
  	{ 321,	1,	UINT16_T,	MINS_U,			NI_SF,					1,	1 },	//	94	OutBack_AGS_Quiet_Time_Weekday_Start_Minute,
  	{ 322,	1,	UINT16_T,	HOURS_U,		NI_SF,					1,	1 },	//	95	OutBack_AGS_Quiet_Time_Weekday_Stop_Hour,
  	{ 323,	1,	UINT16_T,	MINS_U,			NI_SF,					1,	1 },	//	96	OutBack_AGS_Quiet_Time_Weekday_Stop_Minute,
   	{ 324,	1,	UINT16_T,	HOURS_U,		NI_SF,					1,	1 },	//	97	OutBack_AGS_Quiet_Time_Weekend_Start_Hour,
  	{ 325,	1,	UINT16_T,	MINS_U,			NI_SF,					1,	1 },	//	98	OutBack_AGS_Quiet_Time_Weekend_Start_Minute,
  	{ 326,	1,	UINT16_T,	HOURS_U,		NI_SF,					1,	1 },	//	99	OutBack_AGS_Quiet_Time_Weekend_Stop_Hour,
  	{ 327,	1,	UINT16_T,	MINS_U,			NI_SF,					1,	1 },	// 100	OutBack_AGS_Quiet_Time_Weekend_Stop_Minute,
  	{ 328,	2,	UINT32_T,	MINS_U,			NI_SF,					1,	1 },	// 101	OutBack_AGS_Total_Generator_Run_Time,
  	{ 330,	1,	UINT16_T,	ENUMERATED_U,	NI_SF,					1,	1 },	// 102	OutBack_HBX_Mode,
 	{ 331,	1,	UINT16_T,	VOLTS_U,		OutBack_Voltage_SF,		1,	1 },	// 103	OutBack_HBX_Grid_Connect_Voltage,
	{ 332,	1,	UINT16_T,	HOURS_U,		OutBack_Hour_SF,		1,	1 },	// 104	OutBack_HBX_Grid_Connect_Delay,
  	{ 333,	1,	UINT16_T,	VOLTS_U,		OutBack_Voltage_SF,		1,	1 },	// 105	OutBack_HBX_Grid_Disconnect_Voltage,
	{ 334,	1,	UINT16_T,	HOURS_U,		OutBack_Hour_SF,		1,	1 },	// 106	OutBack_HBX_Grid_Disconnect_Delay,
  	{ 335,	1,	UINT16_T,	PERCENTAGE_U,	NI_SF,					1,	1 },	// 107	OutBack_HBX_Grid_Connect_SOC,
	{ 336,	1,	UINT16_T,	PERCENTAGE_U,	NI_SF,					1,	1 },	// 108	OutBack_HBX_Grid_Disconnect_SOC,
   	{ 337,	1,	UINT16_T,	ENUMERATED_U,	NI_SF,					1,	1 },	// 109	OutBack_Grid_Use_Interval_1_Mode,
   	{ 338,	1,	UINT16_T,	HOURS_U,		NI_SF,					1,	1 },	// 110	OutBack_Grid_Use_Interval_1_Weekday_Start_Hour,
  	{ 339,	1,	UINT16_T,	MINS_U,			NI_SF,					1,	1 },	// 111	OutBack_Grid_Use_Interval_1_Weekday_Start_Minute,
  	{ 340,	1,	UINT16_T,	HOURS_U,		NI_SF,					1,	1 },	// 112	OutBack_Grid_Use_Interval_1_Weekday_Stop_Hour,
  	{ 341,	1,	UINT16_T,	MINS_U,			NI_SF,					1,	1 },	// 113	OutBack_Grid_Use_Interval_1_Weekday_Stop_Minute,
   	{ 342,	1,	UINT16_T,	HOURS_U,		NI_SF,					1,	1 },	// 114	OutBack_Grid_Use_Interval_1_Weekend_Start_Hour,
  	{ 343,	1,	UINT16_T,	MINS_U,			NI_SF,					1,	1 },	// 115	OutBack_Grid_Use_Interval_1_Weekend_Start_Minute,
  	{ 344,	1,	UINT16_T,	HOURS_U,		NI_SF,					1,	1 },	// 116	OutBack_Grid_Use_Interval_1_Weekend_Stop_Hour,
  	{ 345,	1,	UINT16_T,	MINS_U,			NI_SF,					1,	1 },	// 117	OutBack_Grid_Use_Interval_1_Weekend_Stop_Minute,
   	{ 346,	1,	UINT16_T,	ENUMERATED_U,	NI_SF,					1,	1 },	// 118	OutBack_Grid_Use_Interval_2_Mode,
   	{ 347,	1,	UINT16_T,	HOURS_U,		NI_SF,					1,	1 },	// 119	OutBack_Grid_Use_Interval_2_Weekday_Start_Hour,
  	{ 348,	1,	UINT16_T,	MINS_U,			NI_SF,					1,	1 },	// 120	OutBack_Grid_Use_Interval_2_Weekday_Start_Minute,
  	{ 349,	1,	UINT16_T,	HOURS_U,		NI_SF,					1,	1 },	// 121	OutBack_Grid_Use_Interval_2_Weekday_Stop_Hour,
  	{ 350,	1,	UINT16_T,	MINS_U,			NI_SF,					1,	1 },	// 122	OutBack_Grid_Use_Interval_2_Weekday_Stop_Minute,
   	{ 351,	1,	UINT16_T,	ENUMERATED_U,	NI_SF,					1,	1 },	// 123	OutBack_Grid_Use_Interval_3_Mode,
   	{ 352,	1,	UINT16_T,	HOURS_U,		NI_SF,					1,	1 },	// 124	OutBack_Grid_Use_Interval_3_Weekday_Start_Hour,
  	{ 353,	1,	UINT16_T,	MINS_U,			NI_SF,					1,	1 },	// 125	OutBack_Grid_Use_Interval_3_Weekday_Start_Minute,
  	{ 354,	1,	UINT16_T,	HOURS_U,		NI_SF,					1,	1 },	// 126	OutBack_Grid_Use_Interval_3_Weekday_Stop_Hour,
  	{ 355,	1,	UINT16_T,	MINS_U,			NI_SF,					1,	1 },	// 127	OutBack_Grid_Use_Interval_3_Weekday_Stop_Minute,
   	{ 356,	1,	UINT16_T,	ENUMERATED_U,	NI_SF,					1,	1 },	// 128	OutBack_Load_Grid_Transfer_Mode,
  	{ 357,	1,	UINT16_T,	KW_U,			OutBack_Voltage_SF,		1,	1 },	// 129	OutBack_Load_Grid_Transfer_Threshold,
  	{ 358,	1,	UINT16_T,	SECS_U,			NI_SF,					1,	1 },	// 130	OutBack_Load_Grid_Transfer_Connect_Delay,
  	{ 359,	1,	UINT16_T,	SECS_U,			NI_SF,					1,	1 },	// 131	OutBack_Load_Grid_Transfer_Disconnect_Delay,
   	{ 360,	1,	UINT16_T,	VOLTS_U,		OutBack_Voltage_SF,		1,	1 },	// 132	OutBack_Load_Grid_Transfer_Connect_Battery_Voltage,
   	{ 361,	1,	UINT16_T,	VOLTS_U,		OutBack_Voltage_SF,		1,	1 },	// 133	OutBack_Load_Grid_Transfer_Re_Connect_Battery_Voltage,
   	{ 362,	1,	UINT16_T,	ENUMERATED_U,	NI_SF,					1,	1 },	// 134	OutBack_Global_Charger_Control_Mode,
   	{ 363,	1,	UINT16_T,	AMPS_U,			NI_SF,					1,	1 },	// 135	OutBack_Global_Charger_Output_Limit,
   	{ 364,	1,	UINT16_T,	ENUMERATED_U,	NI_SF,					1,	1 },	// 136	OutBack_Radian_AC_Coupled_Mode,
   	{ 365,	1,	UINT16_T,	NI_U,			NI_SF,					1,	1 },	// 137	OutBack_Radian_AC_Coupled_AUX_Port,
   	{ 366,	2,	UINT32_T,	NI_U,			NI_SF,					1,	0 },	// 138	OutBack_URL_Lock,
   	{ 368,	20,	STRING_T,	NI_U,			NI_SF,					1,	0 },	// 139	OutBack_Web_Reporting_Base_URL,
   	{ 388,	1,	UINT16_T,	ENUMERATED_U,	NI_SF,					1,	1 },	// 140	OutBack_Web_User_Logged_In_Status
   	{ 389,	1,	UINT16_T,	ENUMERATED_U,	NI_SF,					0,	1 },	// 141	OutBack_HUB_Type
	{ 390,	1,	UINT16_T,	NI_U,			NI_SF,					0,	1 },	// 142	OutBack_HUB_Major_Firmware_Number,
	{ 391,	1,	UINT16_T,	NI_U,			NI_SF,					0,	1 },	// 143	OutBack_HUB_Mid_Firmware_Number,
	{ 392,	1,	UINT16_T,	NI_U,			NI_SF,					0,	1 },	// 144	OutBack_HUB_Minor_Firmware_Number,
	{ 393,	1,	UINT16_T,	NI_U,			NI_SF,					1,	1 },	// 145	OutBack_Year,
	{ 394,	1,	UINT16_T,	NI_U,			NI_SF,					1,	1 },	// 146	OutBack_Month,
	{ 395,	1,	UINT16_T,	NI_U,			NI_SF,					1,	1 },	// 147	OutBack_Day,
	{ 396,	1,	UINT16_T,	NI_U,			NI_SF,					1,	1 },	// 148	OutBack_Hour,
	{ 397,	1,	UINT16_T,	NI_U,			NI_SF,					1,	1 },	// 149	OutBack_Minute,
	{ 398,	1,	UINT16_T,	NI_U,			NI_SF,					1,	1 },	// 150	OutBack_Second,
	{ 399,	1,	INT16_T,	DEGREES_C_U,	OutBack_Temp_SF,		0,	1 },	// 151	OutBack_Temp_Batt,
	{ 400,	1,	INT16_T,	DEGREES_C_U,	OutBack_Temp_SF,		0,	1 },	// 152	OutBack_Temp_Ambient,
	{ 401,	1,	UINT16_T,	NI_U,			NI_SF,					0,	1 },	// 153	OutBack_Temp_SF,
	{ 402,	1,	UINT16_T,	BITFIELD_U,		NI_SF,					0,	1 },	// 154	OutBack_Error,
	{ 403,	1,	UINT16_T,	BITFIELD_U,		NI_SF,					0,	1 },	// 155	OutBack_Status,
	{ 404,	1,	UINT16_T,	NI_U,			NI_SF,					1,	1 },	// 156	OutBack_Update_Device_Firmware_Port,
   	{ 405,	1,	UINT16_T,	ENUMERATED_U,	NI_SF,					0,	1 },	// 157	OutBack_Gateway_Type
	{ 406,	1,	UINT16_T,	VOLTS_U,		NI_SF,					0,  1 },	// 158  OutBack_System_Voltage
	{ 407,	1,	UINT16_T,	VOLTS_U,		OutBack_Voltage_SF,		0,  1 },	// 159  OutBack_Measured_System_Voltage
	{ 408,	1,	UINT16_T,	MINS_U,			NI_SF,					1,	1 },	// 160	OutBack_AGS_AC_Reconnect_Delay,
	{ 409,	1,	UINT16_T,	ENUMERATED_U,	NI_SF,					1,	1 },	// 161	OutBack_Multi_Phase_Coordination,
	{ 410,	1,	INT16_T,	ENUMERATED_U,	NI_SF,					1,	1 },	// 162	OutBack_Sched_1_AC_Mode,
	{ 411,	1,	UINT16_T,	HOURS_U,		NI_SF,					1,	1 },	// 163	OutBack_Sched_1_AC_Mode_Hour,
	{ 412,	1,	UINT16_T,	MINS_U,			NI_SF,					1,	1 },	// 164	OutBack_Sched_1_AC_Mode_Min,
	{ 413,	1,	INT16_T,	ENUMERATED_U,	NI_SF,					1,	1 },	// 165	OutBack_Sched_2_AC_Mode,
	{ 414,	1,	UINT16_T,	HOURS_U,		NI_SF,					1,	1 },	// 166	OutBack_Sched_2_AC_Mode_Hour,
	{ 415,	1,	UINT16_T,	MINS_U,			NI_SF,					1,	1 },	// 167	OutBack_Sched_2_AC_Mode_Min,
	{ 416,	1,	INT16_T,	ENUMERATED_U,	NI_SF,					1,	1 },	// 168	OutBack_Sched_3_AC_Mode,
	{ 417,	1,	UINT16_T,	HOURS_U,		NI_SF,					1,	1 },	// 169	OutBack_Sched_3_AC_Mode_Hour,
	{ 418,	1,	UINT16_T,	MINS_U,			NI_SF,					1,	1 },	// 170	OutBack_Sched_3_AC_Mode_Min,
	{ 419,	1,	UINT16_T,	NI_U,			NI_SF,					1,	1 },	// 171	OutBack_Auto_reboot,
	{ 420,	1,	UINT16_T,	NI_U,			NI_SF,					1,	1 },	// 172	OutBack_Spare_Reg_2,
	{ 421,	1,	UINT16_T,	NI_U,			NI_SF,					1,	1 },	// 173	OutBack_Spare_Reg_3,
	{ 422,	1,	UINT16_T,	NI_U,			NI_SF,					1,	1 },	// 174	OutBack_Spare_Reg_4,
//
	{ 1,	1,	UINT16_T,	NI_U,			NI_SF,					0,	1 },	// 175	OB_SunSpec_DID = 64120,
	{ 2,	1,	UINT16_T,	REGISTERS_U,	NI_SF,					0,	1 },	// 176	OB_SunSpec_Length
	{ 3,	1,	INT16_T,	NI_U,			NI_SF,					0,	1 },	// 177	OB_DC_Voltage_SF,
	{ 4,	1,	INT16_T,	NI_U,			NI_SF,					0,	1 },	// 178	OB_AC_Current_SF,
	{ 5,	1,	INT16_T,	NI_U,			NI_SF,					0,	1 },	// 179	OB_Time_SF,
	{ 6,	1,	UINT16_T,	NI_U,			NI_SF,					1,	1 },	// 180	OB_Bulk_Charge_Enable_Disable
	{ 7,	1,	UINT16_T,	NI_U,			NI_SF,					1,	1 },	// 181	OB_Inverter_AC_Drop_Use
	{ 8,	1,	UINT16_T,	NI_U,			NI_SF,					1,	1 },	// 182	OB_Set_Inverter_Mode
	{ 9,	1,	UINT16_T,	NI_U,			NI_SF,					1,	1 },	// 183	OB_Grid_Tie_Mode
	{ 10,	1,	UINT16_T,	NI_U,			NI_SF,					1,	1 },	// 184	OB_Set_Inverter_Charger_Mode
	{ 11,	1,	UINT16_T,	NI_U,			NI_SF,					0,	1 },	// 185	OB_Control_Status
	{ 12,	1,	UINT16_T,	VOLTS_U,		OB_DC_Voltage_SF,		1,	1 },	// 186	OB_Set_Sell_Voltage
	{ 13,	1,	UINT16_T,	AMPS_U,			OB_AC_Current_SF,		1,	1 },	// 187	OB_Set_Sell_Radian_Inverter_Sell_Current_Limit
	{ 14,	1,	UINT16_T,	VOLTS_U,		OB_DC_Voltage_SF,		1,	1 },	// 188	OB_Set_Absorb_Voltage
	{ 15,	1,	UINT16_T,	HOURS_U,		OB_Time_SF,				1,	1 },	// 189	OB_Set_Absorb_Time
	{ 16,	1,	UINT16_T,	VOLTS_U,		OB_DC_Voltage_SF,		1,	1 },	// 190	OB_Set_Float_Voltage
	{ 17,	1,	UINT16_T,	HOURS_U,		OB_Time_SF,				1,	1 },	// 191	OB_Set_Float_Time
	{ 18,	1,	UINT16_T,	AMPS_U,			OB_AC_Current_SF,		1,	1 },	// 192	OB_Set_Inverter_Charger_Current_Limit
	{ 19,	1,	UINT16_T,	AMPS_U,			OB_AC_Current_SF,		1,	1 },	// 193	OB_Set_Inverter_AC1_Current_Limit
	{ 20,	1,	UINT16_T,	AMPS_U,			OB_AC_Current_SF,		1,	1 },	// 194	OB_Set_Inverter_AC2_Current_Limit
   	{ 21,	1,	UINT16_T,	ENUMERATED_U,	NI_SF,					1,	1 },	// 195	OB_Set_AGS_OP_Mode
   	{ 22,	1,	UINT16_T,	ENUMERATED_U,	NI_SF,					0,	1 },	// 196	OB_AGS_Operational_State
    { 23,	1,	UINT16_T,	SECS_U,			NI_SF,					0,	1 },	// 197	OB_AGS_Operational_State_Timer
    { 24,	2,	UINT32_T,	TIME_U,			NI_SF,					0,	1 },	// 198	OB_Gen_Last_Run_Start_Time_GMT
    { 26,	2,	UINT32_T,	SECS_U,			NI_SF,					0,	1 },	// 199	OB_Gen_Last_Run_Duration
	{ 28,	1,	UINT16_T,	NI_U,			NI_SF,					1,  1 },	// 200  OB_Set_AC_Output_Freq_Offline_Mode
	{ 29,	1,	UINT16_T,	NI_U,			OB_DC_Voltage_SF,		1,	1 },	// 201  OB_Set_AC_Output_Offline_Freq
//	
	{ 1,	1,	UINT16_T,	NI_U,			NI_SF,					0,	1 },	// 202	CC_SunSpec_DID = 64111,
	{ 2,	1,	UINT16_T,	REGISTERS_U,	NI_SF,					0,	1 },	// 203  CC_SunSpec_Length,
	{ 3,	1,	UINT16_T,	NI_U,			NI_SF,					0,	1 },	// 204	CC_port_number,
	{ 4,	1,	INT16_T,	NI_U,			NI_SF,					0,	1 },	// 205	CC_Voltage_SF,
	{ 5,	1,	INT16_T,	NI_U,			NI_SF,					0,	1 },	// 206	CC_Current_SF,
	{ 6,	1,	INT16_T,	NI_U,			NI_SF,					0,	1 },	// 207	CC_Power_SF,
	{ 7,	1,	INT16_T,	NI_U,			NI_SF,					0,	1 },	// 208	CC_AH_SF,
	{ 8,	1,	INT16_T,	NI_U,			NI_SF,					0,	1 },	// 209	CC_KWH_SF,
	{ 9,	1,	UINT16_T,	VOLTS_U,		CC_Voltage_SF,			0,	1 },	// 210	CC_Batt_Voltage,
	{ 10,	1,	UINT16_T,	VOLTS_U,		CC_Voltage_SF,			0,	1 },	// 211	CC_Array_Voltage,
	{ 11,	1,	UINT16_T,	AMPS_U,			CC_Current_SF,			0,	1 },	// 212	CC_Batt_Current,
	{ 12,	1,	UINT16_T,	AMPS_U,			CC_Power_SF,			0,	1 },	// 213	CC_Array_Current,
	{ 13,	1,	UINT16_T,	ENUMERATED_U,	NI_SF,					0,	1 },	// 214	CC_Charger_State,
	{ 14,	1,	UINT16_T,	WATTS_U,		CC_Power_SF,			0,	1 },	// 215	CC_Watts,
	{ 15,	1,	UINT16_T,	VOLTS_U,		CC_Voltage_SF,			0,	1 },	// 216	CC_Todays_Min_Battery_Volts,
	{ 16,	1,	UINT16_T,	VOLTS_U,		CC_Voltage_SF,			0,	1 },	// 217	CC_Todays_Max_Battery_Volts,
	{ 17,	1,	UINT16_T,	VOLTS_U,		CC_Voltage_SF,			0,	1 },	// 218	CC_VOC,
	{ 18,	1,	UINT16_T,	VOLTS_U,		NI_SF,					0,	1 },	// 219	CC_Todays_Peak_VOC,
	{ 19,	1,	UINT16_T,	KWH_U,			CC_KWH_SF,				0,	1 },	// 220	CC_Todays_kWH,
	{ 20,	1,	UINT16_T,	AH_U,			CC_AH_SF,				0,	1 },	// 221  CC_Todays_AH,
	{ 21,	1,	UINT16_T,	KWH_U,			NI_SF,					0,	1 },	// 222	CC_Lifetime_kWH_Hours,
	{ 22,	1,	UINT16_T,	KAH_U,			CC_KWH_SF,				0,	1 },	// 223	CC_Lifetime_kAmp_Hours,
	{ 23,	1,	UINT16_T,	WATTS_U,		CC_Power_SF,			0,	1 },	// 224	CC_Lifetime_Max_Watts,
	{ 24,	1,	UINT16_T,	VOLTS_U,		CC_Voltage_SF,			0,	1 },	// 225	CC_Lifetime_Max_Battery_Volts,
	{ 25,	1,	UINT16_T,	VOLTS_U,		CC_Voltage_SF,			0,	1 },	// 226  CC_Lifetime_Max_VOC,
	{ 26,	1,	UINT16_T,	NI_U,			NI_SF,					0,	1 },	// 227	CC_Temp_SF,
	{ 27,	1,	INT16_T,	DEGREES_C_U,	CC_Temp_SF,				0,	1 },	// 228	CC_Temp_Output_FETs,
	{ 28,	1,	INT16_T,	DEGREES_C_U,	CC_Temp_SF,				0,	1 },	// 229	CC_Temp_Enclosure,
//		
	{ 1,	1,	UINT16_T,	NI_U,			NI_SF,					0,	1 },	// 230  CCconfig_SunSpec_DID = 64112,
	{ 2,	1,	UINT16_T,	REGISTERS_U,	NI_SF,					0,	1 },	// 231	CCconfig_SunSpec_Length,
	{ 3,	1,	UINT16_T,	NI_U,			NI_SF,					0,	1 },	// 232  CCconfig_port_number,
	{ 4,	1,	INT16_T,	NI_U,			NI_SF,					0,	1 },	// 233	CCconfig_Voltage_SF,
	{ 5,	1,	INT16_T,	NI_U,			NI_SF,					0,	1 },	// 234  CCconfig_Current_SF,
	{ 6,	1,	INT16_T,	NI_U,			NI_SF,					0,	1 },	// 235  CCconfig_Hours_SF,
	{ 7,	1,	INT16_T,	NI_U,			NI_SF,					0,	1 },	// 236	CCconfig_Power_SF,
	{ 8,	1,	INT16_T,	NI_U,			NI_SF,					0,	1 },	// 237  CCconfig_AH_SF,
	{ 9,	1,	INT16_T,	NI_U,			NI_SF,					0,	1 },	// 238	CCconfig_KWH_SF,
	{ 10,	1,	UINT16_T,	BITFIELD_U,		NI_SF,					0,	1 },	// 239	CCconfig_Faults,
	{ 11,	1,	UINT16_T,	VOLTS_U,		CCconfig_Voltage_SF,	1,	1 },	// 240	CCconfig_Absorb_Volts,
	{ 12,	1,	UINT16_T,	HOURS_U,		CCconfig_Hours_SF,		1,	1 },	// 241	CCconfig_Absorb_Time_Hours,
	{ 13,	1,	UINT16_T,	AMPS_U,			CCconfig_Voltage_SF,	1,	1 },	// 242	CCconfig_Absorb_End_Amps,
	{ 14,	1,	UINT16_T,	VOLTS_U,		CCconfig_Voltage_SF,	1,	1 },	// 243	CCconfig_Rebulk_Volts,
	{ 15,	1,	UINT16_T,	VOLTS_U,		CCconfig_Voltage_SF,	1,	1 },	// 244	CCconfig_Float_Volts,
	{ 16,	1,	UINT16_T,	AMPS_U,			CCconfig_Current_SF,	1,	1 },	// 245	CCconfig_Bulk_Current,
	{ 17,	1,	UINT16_T,	VOLTS_U,		CCconfig_Voltage_SF,	1,	1 },	// 246	CCconfig_EQ_Volts,
	{ 18,	1,	UINT16_T,	HOURS_U,		NI_SF,					1,	1 },	// 247  CCconfig_EQ_Time_Hours,
	{ 19,	1,	UINT16_T,	DAYS_U,			NI_SF,					1,	1 },	// 248	CCconfig_Auto_EQ_Days,
	{ 20,	1,	UINT16_T,	ENUMERATED_U,	NI_SF,					1,	1 },	// 249	CCconfig_MPPT_Mode,
	{ 21,	1,	UINT16_T,	ENUMERATED_U,	NI_SF,					1,	1 },	// 250	CCconfig_Sweep_Width,
	{ 22,	1,	UINT16_T,	ENUMERATED_U,	NI_SF,					1,	1 },	// 251	CCconfig_Sweep_Max_Percentage,
	{ 23,	1,	UINT16_T,	PERCENTAGE_U,	CCconfig_Voltage_SF,	1,	1 },	// 252	CCconfig_U_Pick_PWM_Duty_Cycle,
	{ 24,	1,	UINT16_T,	ENUMERATED_U,	NI_SF,					1,	1 },	// 253	CCconfig_Grid_Tie_Mode,
	{ 25,	1,	UINT16_T,	ENUMERATED_U,	NI_SF,					1,	1 },	// 254	CCconfig_Temp_Comp_Mode,
	{ 26,	1,	UINT16_T,	VOLTS_U,		CCconfig_Voltage_SF,	1,	1 },	// 255	CCconfig_Temp_Comp_Lower_Limit_Volts,
	{ 27,	1,	UINT16_T,	VOLTS_U,		CCconfig_Voltage_SF,	1,	1 },	// 256	CCconfig_Temp_Comp_Upper_Limit_Volts,
	{ 28,	1,	UINT16_T,	NI_U,			NI_SF,					1,	1 },	// 257	CCconfig_Temp_Comp_Slope,	
	{ 29,	1,	UINT16_T,	ENUMERATED_U,	NI_SF,					1,	1 },	// 258	CCconfig_Auto_Restart_Mode,
	{ 30,	1,	UINT16_T,	VOLTS_U,		CCconfig_Voltage_SF,	1,	1 },	// 259	CCconfig_Wakeup_VOC,
	{ 31,	1,	UINT16_T,	AMPS_U,			CCconfig_Voltage_SF,	1,	1 },	// 260  CCconfig_Snooze_Mode_Amps,
	{ 32,	1,	UINT16_T,	MINS_U,			NI_SF,					1,	1 },	// 261	CCconfig_Wakeup_Interval,
	{ 33,	1,	UINT16_T,	ENUMERATED_U,	NI_SF,					1,	1 },	// 262	CCconfig_AUX_Mode,
	{ 34,	1,	UINT16_T,	ENUMERATED_U,	NI_SF,					1,	1 },	// 263	CCconfig_AUX_Control,
	{ 35,	1,	UINT16_T,	ENUMERATED_U,	NI_SF,					0,	1 },	// 264	CCconfig_AUX_State,
	{ 36,	1,	UINT16_T,	ENUMERATED_U,	NI_SF,					1,	1 },	// 265	CCconfig_AUX_Polarity,
	{ 37,	1,	UINT16_T,	VOLTS_U,		CCconfig_Voltage_SF,	1,	1 },	// 266	CCconfig_AUX_Low_Batt_Disconnect,
	{ 38,	1,	UINT16_T,	VOLTS_U,		CCconfig_Voltage_SF,	1,	1 },	// 267	CCconfig_AUX_Low_Batt_Reconnect,
	{ 39,	1,	UINT16_T,	SECS_U,			NI_SF,					1,	1 },	// 268	CCconfig_AUX_Low_Batt_Disconnect_Delay,
	{ 40,	1,	UINT16_T,	VOLTS_U,		CCconfig_Voltage_SF,	1,	1 },	// 269	CCconfig_AUX_Vent_Fan_Volts,
	{ 41,	1,	UINT16_T,	VOLTS_U,		CCconfig_Voltage_SF,	1,	1 },	// 270	CCconfig_AUX_PV_Limit_Volts,
	{ 42,	1,	UINT16_T,	SECS_U,			CCconfig_Hours_SF,		1,	1 },	// 271	CCconfig_AUX_PV_Limit_Hold_Time,
	{ 43,	1,	UINT16_T,	VOLTS_U,		CCconfig_Voltage_SF,	1,	1 },	// 272	CCconfig_AUX_Night_Light_Thres_Volts,
	{ 44,	1,	UINT16_T,	HOURS_U,		CCconfig_Hours_SF,		1,	1 },	// 273	CCconfig_Night_Light_ON_Hours,
	{ 45,	1,	UINT16_T,	MINS_U,			NI_SF,					1,	1 },	// 274	CCconfig_Night_Light_ON_Hyst_Time,
	{ 46,	1,	UINT16_T,	MINS_U,			NI_SF,					1,	1 },	// 275	CCconfig_Night_Light_OFF_Hyst_Time,
	{ 47,	1,	UINT16_T,	VOLTS_U,		CCconfig_Voltage_SF,	1,	1 },	// 276	CCconfig_AUX_Error_Battery_Volts,
	{ 48,	1,	UINT16_T,	SECS_U,			CCconfig_Voltage_SF,	1,	1 },	// 277	CCconfig_AUX_Divert_Hold_Time,
	{ 49,	1,	UINT16_T,	SECS_U,			NI_SF,					1,	1 },	// 278	CCconfig_AUX_Divert_Delay_Time,
	{ 50,	1,	INT16_T,	VOLTS_U,		CCconfig_Voltage_SF,	1,	1 },	// 279	CCconfig_AUX_Divert_Relative_Volts,
	{ 51,	1,	UINT16_T,	VOLTS_U,		CCconfig_Voltage_SF,	1,	1 },	// 280	CCconfig_AUX_Divert_Hyst_Volts,
	{ 52,	1,	UINT16_T,	NI_U,			NI_SF,					0,	1 },	// 281	CCconfig_Major_Firmware_Number,
	{ 53,	1,	UINT16_T,	NI_U,			NI_SF,					0,	1 },	// 282	CCconfig_Mid_Firmware_Number,
	{ 54,	1,	UINT16_T,	NI_U,			NI_SF,					0,	1 },	// 283	CCconfig_Minor_Firmware_Number,
	{ 55,	1,	UINT16_T,	DAYS_U,			NI_SF,					1,	1 },	// 284  CCconfig_Set_Log_Day_Offset,
	{ 56,	1,	UINT16_T,	DAYS_U,			NI_SF,					0,	1 },	// 285	CCconfig_Get_Current_Log_Day_Offset,
	{ 57,	1,	UINT16_T,	AH_U,			CCconfig_AH_SF,			0,	1 },	// 286	CCconfig_Log_Daily_AH,
	{ 58,	1,	UINT16_T,	KWH_U,			CCconfig_KWH_SF,		0,	1 },	// 287	CCconfig_Log_Daily_kWH,
	{ 59,	1,	UINT16_T,	AMPS_U,			CCconfig_Voltage_SF,	0,	1 },	// 288	CCconfig_Log_Daily_Max_Output_Amps,
	{ 60,	1,	UINT16_T,	WATTS_U,		CCconfig_Power_SF,		0,	1 },	// 289	CCconfig_Log_Daily_Max_Output_Watts,
	{ 61,	1,	UINT16_T,	MINS_U,			NI_SF,					0,	1 },	// 290	CCconfig_Log_Daily_Absorb_Time,
	{ 62,	1,	UINT16_T,	MINS_U,			NI_SF,					0,	1 },	// 291	CCconfig_Log_Daily_Float_Time,
	{ 63,	1,	UINT16_T,	VOLTS_U,		CCconfig_Voltage_SF,	0,	1 },	// 292	CCconfig_Log_Daily_Min_Batt_Volts,
	{ 64,	1,	UINT16_T,	VOLTS_U,		CCconfig_Voltage_SF,	0,	1 },	// 293	CCconfig_Log_Daily_Max_Batt_Volts,
	{ 65,	1,	UINT16_T,	VOLTS_U,		NI_SF,					0,	1 },	// 294	CCconfig_Log_Daily_Max_Input_Volts,
	{ 66,	1,	UINT16_T,	NI_U,			NI_SF,					0,	1 },	// 295	CCconfig_Clear_Log_Read,
	{ 67,	1,	UINT16_T,	NI_U,			NI_SF,					1,	1 },	// 296	CCconfig_Clear_Log_Write_Complement,
	{ 68,	1,	UINT16_T,	NI_U,			NI_SF,					0,	1 },	// 297	CCconfig_Stats_Maximum_Reset_Read
	{ 69,	1,	UINT16_T,	NI_U,			NI_SF,					1,	1 },	// 298	CCconfig_Stats_Maximum_Write_Complement
	{ 70,	1,	UINT16_T,	NI_U,			NI_SF,					0,	1 },	// 299	CCconfig_Stats_Totals_Reset_Read
	{ 71,	1,	UINT16_T,	NI_U,			NI_SF,					1,	1 },	// 300	CCconfig_Stats_Totals_Write_Complement
	{ 72,	1,	INT16_T,	VOLTS_U,		CCconfig_Voltage_SF,	1,	1 },	// 301	CCconfig_Battery_Voltage_Calibrate_Offset
	{ 73,	9,	STRING_T,	NI_U,			NI_SF,					0,	1 },	// 302	CCconfig_Serial_Number,
	{ 82,	9,  STRING_T,	NI_U,			NI_SF,					0,	1 },	// 303	CCconfig_Model_Number,
//
	{ 1,	1,	UINT16_T,	NI_U,			NI_SF,					0,	1 },	// 304	I_SunSpec_DID = 101,
	{ 2,	1,	UINT16_T,	REGISTERS_U,	NI_SF,					0,	1 },	// 305	I_SunSpec_Length,
	{ 3,	1,	UINT16_T,	AMPS_U,			I_AC_Current_SF,		0,	1 },	// 306	I_AC_Current,
	{ 4,	1,	UINT16_T,	AMPS_U,			I_AC_Current_SF,		0,	1 },	// 307	I_AC_CurrentA,
	{ 5,	1,	UINT16_T,	AMPS_U,			I_AC_Current_SF,		0,	1 },	// 308	I_AC_CurrentB,
	{ 6,	1,	UINT16_T,	AMPS_U,			I_AC_Current_SF,		0,	1 },	// 309  I_AC_CurrentC,
	{ 7,	1,	INT16_T,	NI_U,			NI_SF,					0,	1 },	// 310	I_AC_Current_SF,
	{ 8,	1,	UINT16_T,	VOLTS_U,		I_AC_Voltage_SF,		0,	1 },	// 311	I_AC_VoltageAB,
	{ 9,	1,	UINT16_T,	VOLTS_U,		I_AC_Voltage_SF,		0,	1 },	// 312	I_AC_VoltageBC,
	{ 10,	1,	UINT16_T,	VOLTS_U,		I_AC_Voltage_SF,		0,	1 },	// 313	I_AC_VoltageCA,
	{ 11,	1,	UINT16_T,	VOLTS_U,		I_AC_Voltage_SF,		0,	1 },	// 314	I_AC_VoltageAN,
	{ 12,	1,	UINT16_T,	VOLTS_U,		I_AC_Voltage_SF,		0,	1 },	// 315	I_AC_VoltageBN,
	{ 13,	1,	UINT16_T,	VOLTS_U,		I_AC_Voltage_SF,		0,	1 },	// 316	I_AC_VoltageCN,
	{ 14,	1,	INT16_T,	NI_U,			NI_SF,					0,	1 },	// 317	I_AC_Voltage_SF,
	{ 15,	1,	INT16_T,	KW_U,			I_AC_Power_SF,			0,	1 },	// 318	I_AC_Power,
	{ 16,	1,	INT16_T,	NI_U,			NI_SF,					0,	1 },	// 319	I_AC_Power_SF,
	{ 17,	1,	UINT16_T,	HERTZ_U,		I_AC_Frequency_SF,		0,	1 },	// 320	I_AC_Frequency,
	{ 18,	1,	INT16_T,	NI_U,			NI_SF,					0,	1 },	// 321	I_AC_Frequency_SF,
	{ 19,	1,	INT16_T,	VA_U,			I_AC_VA_SF,				0,	1 },	// 322	I_AC_VA,
	{ 20,	1,	INT16_T,	NI_U,			NI_SF,					0,	1 },	// 323	I_AC_VA_SF,
	{ 21,	1,	INT16_T,	VAR_U,			I_AC_VAR_SF,			0,	1 },	// 324	I_AC_VAR,
	{ 22,	1,	INT16_T,	NI_U,			NI_SF,					0,	1 },	// 325	I_AC_VAR_SF,
	{ 23,	1,	INT16_T,	PERCENTAGE_U,	I_AC_PF_SF,				0,	1 },	// 326	I_AC_PF,
	{ 24,	1,	INT16_T,	NI_U,			NI_SF,					0,	1 },	// 327	I_AC_PF_SF,
	{ 25,	2,	ACC32_T,	WH_U,			I_AC_Energy_WH_SF,		0,	1 },	// 328	I_AC_Energy_WH,
	{ 27,	1,	UINT16_T,	NI_U,			NI_SF,					0,	1 },	// 329	I_AC_Energy_WH_SF,
	{ 28,	1,	UINT16_T,	AMPS_U,			I_DC_Current_SF,		0,	1 },	// 330	I_DC_Current,
	{ 29,	1,	INT16_T,	NI_U,			NI_SF,					0,	1 },	// 331	I_DC_Current_SF,
	{ 30,	1,	UINT16_T,	VOLTS_U,		I_DC_Voltage_SF,		0,	1 },	// 332	I_DC_Voltage,
	{ 31,	1,	INT16_T,	NI_U,			NI_SF,					0,	1 },	// 333	I_DC_Voltage_SF,
	{ 32,	1,	INT16_T,	WATTS_U,		I_DC_Power_SF,			0,	1 },	// 334	I_DC_Power,
	{ 33,	1,	INT16_T,	NI_U,			NI_SF,					0,	1 },	// 335	I_DC_Power_SF,
	{ 34,	1,	INT16_T,	DEGREES_C_U,	I_Temp_SF,				0,	1 },	// 336	I_Temp_Cab,
	{ 35,	1,	INT16_T,	DEGREES_C_U,	I_Temp_SF,				0,	1 },	// 337	I_Temp_Sink,
	{ 36,	1,	INT16_T,	DEGREES_C_U,	I_Temp_SF,				0,	1 },	// 338	I_Temp_Trans,
	{ 37,	1,	INT16_T,	DEGREES_C_U,	I_Temp_SF,				0,	1 },	// 339	I_Temp_Other,
	{ 38,	1,	INT16_T,	NI_U,			NI_SF,					0,	1 },	// 340	I_Temp_SF,
	{ 39,	1,	UINT16_T,	ENUMERATED_U,	NI_SF,					0,	1 },	// 341	I_Status,
	{ 40,	1,	UINT16_T,	BITFIELD_U,		NI_SF,					0,	1 },	// 342	I_Status_Vendor,
	{ 41,	2,	UINT32_T,	BITFIELD_U,		NI_SF,					0,	1 },	// 343	I_Event_1,
	{ 43,	2,	UINT32_T,	BITFIELD_U,		NI_SF,					0,	1 },	// 344	I_Event_2,
	{ 45,	2,	UINT32_T,	BITFIELD_U,		NI_SF,					0,	1 },	// 345  I_Event_1_Vendor,
	{ 48,	2,	UINT32_T,	BITFIELD_U,		NI_SF,					0,	1 },	// 346	I_Event_2_Vendor,
	{ 50,	2,	UINT32_T,	BITFIELD_U,		NI_SF,					0,	1 },	// 347	I_Event_3_Vendor,
	{ 52,	2,	UINT32_T,	BITFIELD_U,		NI_SF,					0,	1 },	// 348	I_Event_4_Vendor,
//
	{ 1,	1,	UINT16_T,	NI_U,			NI_SF,					0,	1 },	// 349 GSconfig_SunSpec_DID = 64116,
	{ 2,	1,	UINT16_T,	REGISTERS_U,	NI_SF,					0,	1 },	// 350 GSconfig_SunSpec_Length,
	{ 3,	1,	UINT16_T,	NI_U,			NI_SF,					0,	1 },	// 351 GSconfig_Port_Number,
	{ 4,	1,	INT16_T,	NI_U,			NI_SF,					0,	1 },	// 352 GSconfig_DC_Voltage_SF,
	{ 5,	1,	INT16_T,	NI_U,			NI_SF,					0,	1 },	// 353 GSconfig_AC_Current_SF,
	{ 6,	1,	INT16_T,	NI_U,			NI_SF,					0,	1 },	// 354 GSconfig_AC_Voltage_SF,
	{ 7,	1,	INT16_T,	NI_U,			NI_SF,					0,	1 },	// 355 GSconfig_Time_SF,
	{ 8,	1,	UINT16_T,	NI_U,			NI_SF,					0,	1 },	// 356 GSconfig_Major_firmware_number,
	{ 9,	1,	UINT16_T,	NI_U,			NI_SF,					0,	1 },	// 357 GSconfig_Mid_firmware_number,
	{ 10,	1,	UINT16_T,	NI_U,			NI_SF,					0,	1 },	// 358 GSconfig_Minor_firmware_number,	
	{ 11,	1,	UINT16_T,	VOLTS_U,		GSconfig_DC_Voltage_SF,	1,	1 },	// 359 GSconfig_Absorb_Volts,
	{ 12,	1,	UINT16_T,	HOURS_U,		GSconfig_Time_SF,		1,	1 },	// 360 GSconfig_Absorb_Time_Hours,
	{ 13,	1,	UINT16_T,	VOLTS_U,		GSconfig_DC_Voltage_SF,	1,	1 },	// 361 GSconfig_Float_Volts,
	{ 14,	1,	UINT16_T,	HOURS_U,		GSconfig_Time_SF,		1,	1 },	// 362 GSconfig_Float_Time_Hours,
	{ 15,	1,	UINT16_T,	VOLTS_U,		GSconfig_DC_Voltage_SF,	1,	1 },	// 363 GSconfig_ReFloat_Volts,
	{ 16,	1,	UINT16_T,	VOLTS_U,		GSconfig_DC_Voltage_SF,	1,	1 },	// 364 GSconfig_EQ_Volts,
	{ 17,	1,	UINT16_T,	HOURS_U,		GSconfig_Time_SF,		1,	1 },	// 365 GSconfig_EQ_Time_Hours,
	{ 18,	1,	UINT16_T,	NI_U,			NI_SF,					1,	1 },	// 366 GSconfig_Search_Sensitivity,
	{ 19,	1,	UINT16_T,	CYCLES_U,		NI_SF,					1,	1 },	// 367 GSconfig_Search_Pulse_Length,
	{ 20,	1,	UINT16_T,	CYCLES_U,		NI_SF,					1,	1 },	// 368 GSconfig_Search_Pulse_Spacing,
	{ 21,	1,	UINT16_T,	ENUMERATED_U,	NI_SF,					1,	1 },	// 369 GSconfig_AC_Input_Select_Priority,
	{ 22,	1,	UINT16_T,	NI_U,			GSconfig_AC_Current_SF,	1,	1 },	// 370 GSconfig_Grid_AC_Input_Current_Limit,
	{ 23,	1,	UINT16_T,	NI_U,			GSconfig_AC_Current_SF,	1,	1 },	// 371 GSconfig_Gen_AC_Input_Current_Limit,
	{ 24,	1,	UINT16_T,	NI_U,			GSconfig_AC_Current_SF,	1,	1 },	// 372 GSconfig_Charger_AC_Input_Current_Limit,	
	{ 25,	1,	UINT16_T,	ENUMERATED_U,	NI_SF,					1,	1 },	// 373 GSconfig_Charger_Operating_Mode	
	{ 26,	1,	UINT16_T,	ENUMERATED_U,	NI_SF,					1,	1 },	// 374 GSconfig_AC_Coupled,
	{ 27,	1,	UINT16_T,	ENUMERATED_U,	NI_SF,					1,	1 },	// 375 GSconfig_Grid_Input_Mode,
	{ 28,	1,	UINT16_T,	VOLTS_U,		GSconfig_AC_Voltage_SF,	1,	1 },	// 376 GSconfig_Grid_Lower_Input_Voltage_Limit,
	{ 29,	1,	UINT16_T,	VOLTS_U,		GSconfig_AC_Voltage_SF,	1,	1 },	// 377 GSconfig_Grid_Upper_Input_Voltage_Limit,
	{ 30,	1,	UINT16_T,	NI_U,			NI_SF,					1,	1 },	// 378 GSconfig_Grid_Transfer_Delay,
	{ 31,	1,	UINT16_T,	MINS_U,			GSconfig_Time_SF,		1,	1 },	// 379 GSconfig_Grid_Connect_Delay,
	{ 32,	1,	UINT16_T,	ENUMERATED_U,	NI_SF,					1,	1 },	// 380 GSconfig_Gen_Input_Mode,
	{ 33,	1,	UINT16_T,	VOLTS_U,		GSconfig_AC_Voltage_SF,	1,	1 },	// 381 GSconfig_Gen_Lower_Input_Voltage_Limit,
	{ 34,	1,	UINT16_T,	VOLTS_U,		GSconfig_AC_Voltage_SF,	1,	1 },	// 382 GSconfig_Gen_Upper_Input_Voltage_Limit,
	{ 35,	1,	UINT16_T,	NI_U,			NI_SF,					1,	1 },	// 383 GSconfig_Gen_Transfer_Delay,
	{ 36,	1,	UINT16_T,	MINS_U,			GSconfig_Time_SF,		1,	1 },	// 384 GSconfig_Gen_Connect_Delay,
	{ 37,	1,	UINT16_T,	VOLTS_U,		GSconfig_AC_Voltage_SF,	1,	1 },	// 385 GSconfig_AC_Output_Voltage,
	{ 38,	1,	UINT16_T,	VOLTS_U,		GSconfig_DC_Voltage_SF,	1,	1 },	// 386 GSconfig_Low_Battery_Cut_Out_Voltage,
	{ 39,	1,	UINT16_T,	VOLTS_U,		GSconfig_DC_Voltage_SF,	1,	1 },	// 387 GSconfig_Low_Battery_Cut_In_Voltage,
	{ 40,	1,	UINT16_T,	ENUMERATED_U,	NI_SF,					1,	1 },	// 388 GSconfig_AUX_Mode,
	{ 41,	1,	UINT16_T,	ENUMERATED_U,	NI_SF,					1,	1 },	// 389 GSconfig_AUX_Control
	{ 42,	1,	UINT16_T,	VOLTS_U,		GSconfig_DC_Voltage_SF,	1,	1 },	// 390 GSconfig_AUX_ON_Battery_Voltage,
	{ 43,	1,	UINT16_T,	MINS_U,			GSconfig_Time_SF,		1,	1 },	// 391 GSconfig_AUX_ON_Delay_Time,
	{ 44,	1,	UINT16_T,	VOLTS_U,		GSconfig_DC_Voltage_SF,	1,	1 },	// 392 GSconfig_AUX_OFF_Battery_Voltage,
	{ 45,	1,	UINT16_T,	MINS_U,			GSconfig_Time_SF,		1,	1 },	// 393 GSconfig_AUX_OFF_Delay_Time,
	{ 46,	1,	UINT16_T,	ENUMERATED_U,	NI_SF,					1,	1 },	// 394 GSconfig_AUX_Relay_Mode,
	{ 47,	1,	UINT16_T,	ENUMERATED_U,	NI_SF,					1,	1 },	// 395 GSconfig_AUX_Relay_Control,	
	{ 48,	1,	UINT16_T,	VOLTS_U,		GSconfig_DC_Voltage_SF,	1,	1 },	// 396 GSconfig_AUX_Relay_ON_Battery_Voltage,
	{ 49,	1,	UINT16_T,	MINS_U,			GSconfig_Time_SF,		1,	1 },	// 397 GSconfig_AUX_Relay_ON_Delay_Time,
	{ 50,	1,	UINT16_T,	VOLTS_U,		GSconfig_DC_Voltage_SF,	1,	1 },	// 398 GSconfig_AUX_Relay_OFF_Battery_Voltage,
	{ 51,	1,	UINT16_T,	MINS_U,			GSconfig_Time_SF,		1,	1 },	// 399 GSconfig_AUX_Relay_OFF_Delay_Time,
	{ 52,	1,	UINT16_T,	ENUMERATED_U,	NI_SF,					1,	1 },	// 400 GSconfig_Stacking_Mode,
	{ 53,	1,	UINT16_T,	NI_U,			NI_SF,					1,	1 },	// 401 GSconfig_Master_Power_Save_Level,
	{ 54,	1,	UINT16_T,	NI_U,			NI_SF,					1,	1 },	// 402 GSconfig_Slave_Power_Save_Level,
	{ 55,	1,	UINT16_T,	VOLTS_U,		GSconfig_DC_Voltage_SF,	1,	1 },	// 403 GSconfig_Sell_Volts,
	{ 56,	1,	UINT16_T,	ENUMERATED_U,	NI_SF,					1,	1 },	// 404 GSconfig_Grid_Tie_Window,
	{ 57,	1,	UINT16_T,	ENUMERATED_U,	NI_SF,					1,	1 },	// 405 GSconfig_Grid_Tie_Enable,
	{ 58,	1,	INT16_T,	VOLTS_U,		NI_SF,					1,	1 },	// 406 GSconfig_Grid_AC_Input_Voltage_Cal_Factor,
	{ 59,	1,	INT16_T,	VOLTS_U,		NI_SF,					1,	1 },	// 407 GSconfig_Gen_AC_Input_Voltage_Cal_Factor,
	{ 60,	1,	INT16_T,	VOLTS_U,		NI_SF,					1,	1 },	// 408 GSconfig_AC_Output_Voltage_Cal_Factor,
	{ 61,	1,	INT16_T,	VOLTS_U,		GSconfig_DC_Voltage_SF,	1,	1 },	// 409 GSconfig_Battery_Voltage_Cal_Factor,
	{ 62,	1,	UINT16_T,	VOLTS_U,		GSconfig_DC_Voltage_SF,	1,	1 },	// 410 GSconfig_ReBulk_Volts,
	{ 63,	1,	UINT16_T,	VOLTS_U,		GSconfig_DC_Voltage_SF,	1,	1 },	// 411 GSconfig_Mini_Grid_LBX_Volts,
	{ 64,	1,	UINT16_T,	HOURS_U,		GSconfig_Time_SF,		1,	1 },	// 412 GSconfig_Mini_Grid_LBX_Delay,
	{ 65,	1,	UINT16_T,	VOLTS_U,		GSconfig_DC_Voltage_SF,	1,	1 },	// 413 GSconfig_Grid_Zero_DoD_Volts, 
	{ 66,	1,	UINT16_T,	NI_U,			GSconfig_AC_Current_SF,	1,	1 },	// 414 GSconfig_Grid_Zero_DoD_Max_Offset_AC_Amps,
	{ 67,	9,	STRING_T,	NI_U,			NI_SF,					1,	1 },	// 415 GSconfig_Serial_Number,
	{ 76,	9,  STRING_T,	NI_U,			NI_SF,					0,	1 },	// 416 GSconfig_Model_Number,
	{ 85,	1,  UINT16_T,	ENUMERATED_U,	NI_SF,					1,	1 },	// 417 GSconfig_Module_Control,
	{ 86,	1,  UINT16_T,	ENUMERATED_U,	NI_SF,					1,	1 },	// 418 GSconfig_Model_Select,
	{ 87,	1,	UINT16_T,	SECS_U,			GSconfig_DC_Voltage_SF,	1,	1 },	// 419 GSconfig_Low_Battery_Cut_Out_Delay,
	{ 88,	1,	UINT16_T,	VOLTS_U,		GSconfig_DC_Voltage_SF,	1,	1 },	// 420 GSconfig_High_Battery_Cut_Out_Voltage,
	{ 89,	1,	UINT16_T,	VOLTS_U,		GSconfig_DC_Voltage_SF,	1,	1 },	// 421 GSconfig_High_Battery_Cut_In_Voltage,
	{ 90,	1,	UINT16_T,	SECS_U,			GSconfig_DC_Voltage_SF,	1,	1 },	// 422 GSconfig_High_Battery_Cut_Out_Delay,
//
	{ 1,	1,	UINT16_T,	NI_U,			NI_SF,					0,	1 },	// 423 GS_Single_SunSpec_DID = 64117,
	{ 2,	1,	UINT16_T,	REGISTERS_U,	NI_SF,					0,	1 },	// 424 GS_Single_SunSpec_Length,
	{ 3,	1,	UINT16_T,	NI_U,			NI_SF,					0,	1 },	// 425 GS_Single_Port_Number,
	{ 4,	1,	INT16_T,	NI_U,			NI_SF,					0,	1 },	// 426 GS_Single_DC_Voltage_SF,
	{ 5,	1,	INT16_T,	NI_U,			NI_SF,					0,	1 },	// 427 GS_Single_AC_Current_SF,
	{ 6,	1,	INT16_T,	NI_U,			NI_SF,					0,	1 },	// 428 GS_Single_AC_Voltage_SF,
	{ 7,	1,	INT16_T,	NI_U,			NI_SF,					0,	1 },	// 429 GS_Single_Frequency_SF,
	{ 8,	1,	UINT16_T,	NI_U,			GS_Single_AC_Current_SF,0,	1 },	// 430 GS_Single_Inverter_Output_Current,
	{ 9,	1,	UINT16_T,	NI_U,			GS_Single_AC_Current_SF,0,	1 },	// 431 GS_Single_Inverter_Charge_Current,
	{ 10,	1,	UINT16_T,	NI_U,			GS_Single_AC_Current_SF,0,	1 },	// 432 GS_Single_Inverter_Buy_Current,
	{ 11,	1,	UINT16_T,	NI_U,			GS_Single_AC_Current_SF,0,	1 },	// 433 GS_Single_Inverter_Sell_Current,
	{ 12,	1,	UINT16_T,	VOLTS_U,		GS_Single_AC_Voltage_SF,0,	1 },	// 434 GS_Single_Grid_Input_AC_Voltage,
	{ 13,	1,	UINT16_T,	VOLTS_U,		GS_Single_AC_Voltage_SF,0,	1 },	// 435 GS_Single_Gen_Input_AC_Voltage,
	{ 14,	1,	UINT16_T,	VOLTS_U,		GS_Single_AC_Voltage_SF,0,	1 },	// 436 GS_Single_AC_Output_Voltage,
	{ 15,	1,	UINT16_T,	ENUMERATED_U,	NI_SF,					0,	1 },	// 437 GS_Single_Inverter_Operating_Mode,
	{ 16,	1,	UINT16_T,	BITFIELD_U,		NI_SF,					0,	1 },	// 438 GS_Single_Error_Flags,
	{ 17,	1,	UINT16_T,	BITFIELD_U,		NI_SF,					0,	1 },	// 439 GS_Single_Warning_Flags,
	{ 18,	1,	UINT16_T,	VOLTS_U,		GS_Single_DC_Voltage_SF,0,	1 },	// 440 GS_Single_Battery_Voltage,
	{ 19,	1,	UINT16_T,	VOLTS_U,		GS_Single_DC_Voltage_SF,0,	1 },	// 441 GS_Single_Temp_Comp_Target_Voltage,
	{ 20,	1,	UINT16_T,	ENUMERATED_U,	NI_SF,					0,	1 },	// 442 GS_Single_AUX_Output_State,
	{ 21,	1,	UINT16_T,	ENUMERATED_U,	NI_SF,					0,	1 },	// 443 GS_Single_AUX_Relay_Output_State,
	{ 22,	1,	INT16_T,	DEGREES_C_U,	NI_SF,					0,	1 },	// 444 GS_Single_L_Module_Transformer_Temp,
	{ 23,	1,	INT16_T,	DEGREES_C_U,	NI_SF,					0,	1 },	// 445 GS_Single_L_Module_Capacitor_Temp,
	{ 24,	1,	INT16_T,	DEGREES_C_U,	NI_SF,					0,	1 },	// 446 GS_Single_L_Module_FET_Temp,
	{ 25,	1,	INT16_T,	DEGREES_C_U,	NI_SF,					0,	1 },	// 447 GS_Single_R_Module_Transformer_Temp,
	{ 26,	1,	INT16_T,	DEGREES_C_U,	NI_SF,					0,	1 },	// 448 GS_Single_R_Module_Capacitor_Temp,
	{ 27,	1,	INT16_T,	DEGREES_C_U,	NI_SF,					0,	1 },	// 449 GS_Single_R_Module_FET_Temp,
	{ 28,	1,	INT16_T,	DEGREES_C_U,	NI_SF,					0,	1 },	// 450 GS_Single_Battery_Temperature,
	{ 29,	1,	UINT16_T,	ENUMERATED_U,	NI_SF,					0,	1 },	// 451 GS_Single_AC_Input_Selection,
	{ 30,	1,	UINT16_T,	CYCLES_U,		GS_Single_Frequency_SF,	0,	1 },	// 452 GS_Single_AC_Input_Frequency,
	{ 31,	1,	UINT16_T,	VOLTS_U,		GS_Single_AC_Voltage_SF,0,	1 },	// 453 GS_Single_AC_Input_Voltage,
	{ 32,	1,	UINT16_T,	ENUMERATED_U,	NI_SF,					0,	1 },	// 454 GS_Single_AC_Input_State,
	{ 33,	1,	UINT16_T,	VOLTS_U,		GS_Single_AC_Voltage_SF,1,	1 },	// 455 GS_Single_Minimum_AC_Input_Voltage,
	{ 34,	1,	UINT16_T,	VOLTS_U,		GS_Single_AC_Voltage_SF,1,	1 },	// 456 GS_Single_Maximum_AC_Input_Voltage,
	{ 35,	1,	UINT16_T,	BITFIELD_U,		NI_SF,					0,	1 },	// 457 GS_Single_Sell_Status,
	{ 36,	1,	INT16_T,	NI_U,			NI_SF,					0,	1 },	// 458 GS_Single_kWh_SF
 	{ 37,	1,	UINT16_T,	KWH_U,			GS_Single_kWh_SF,		0,	1 },	// 459 GS_Single_AC1_Buy_kWh
 	{ 38,	1,	UINT16_T,	KWH_U,			GS_Single_kWh_SF,		0,	1 },	// 460 GS_Single_AC2_Buy_kWh
 	{ 39,	1,	UINT16_T,	KWH_U,			GS_Single_kWh_SF,		0,	1 },	// 461 GS_Single_AC1_Sell_kWh
 	{ 40,	1,	UINT16_T,	KWH_U,			GS_Single_kWh_SF,		0,	1 },	// 462 GS_Single_AC2_Sell_kWh
 	{ 41,	1,	UINT16_T,	KWH_U,			GS_Single_kWh_SF,		0,	1 },	// 463 GS_Single_Output_kWh
 	{ 42,	1,	UINT16_T,	KWH_U,			GS_Single_kWh_SF,		0,	1 },	// 464 GS_Single_Charger_kWh
 	{ 43,	1,	UINT16_T,	KW_U,			GS_Single_kWh_SF,		0,	1 },	// 465 GS_Single_Output_kW
 	{ 44,	1,	UINT16_T,	KW_U,			GS_Single_kWh_SF,		0,	1 },	// 466 GS_Single_Buy_kW
 	{ 45,	1,	UINT16_T,	KW_U,			GS_Single_kWh_SF,		0,	1 },	// 467 GS_Single_Sell_kW
 	{ 46,	1,	UINT16_T,	KW_U,			GS_Single_kWh_SF,		0,	1 },	// 468 GS_Single_Charge_kW
 	{ 47,	1,	UINT16_T,	KW_U,			GS_Single_kWh_SF,		0,	1 },	// 469 GS_Single_Load_kW
 	{ 48,	1,	UINT16_T,	KW_U,			GS_Single_kWh_SF,		0,	1 },	// 470 GS_Single_AC_Couple_kW	
//
	{ 1,	1,	UINT16_T,	NI_U,			NI_SF,					0,	1 },	// 471 GS_Split_SunSpec_DID = 64115,
	{ 2,	1,	UINT16_T,	REGISTERS_U,	NI_SF,					0,	1 },	// 472 GS_Split_SunSpec_Length,
	{ 3,	1,	UINT16_T,	NI_U,			NI_SF,					0,	1 },	// 473 GS_Split_Port_Number,
	{ 4,	1,	INT16_T,	NI_U,			NI_SF,					0,	1 },	// 474 GS_Split_DC_Voltage_SF,
	{ 5,	1,	INT16_T,	NI_U,			NI_SF,					0,	1 },	// 475 GS_Split_AC_Current_SF,
	{ 6,	1,	INT16_T,	NI_U,			NI_SF,					0,	1 },	// 476 GS_Split_AC_Voltage_SF,
	{ 7,	1,	INT16_T,	NI_U,			NI_SF,					0,	1 },	// 477 GS_Split_Frequency_SF,
	{ 8,	1,	UINT16_T,	NI_U,			GS_Split_AC_Current_SF,	0,	1 },	// 478 GS_Split_L1_Inverter_Output_Current,
	{ 9,	1,	UINT16_T,	NI_U,			GS_Split_AC_Current_SF,	0,	1 },	// 479 GS_Split_L1_Inverter_Charge_Current,
	{ 10,	1,	UINT16_T,	NI_U,			GS_Split_AC_Current_SF,	0,	1 },	// 480 GS_Split_L1_Inverter_Buy_Current,
	{ 11,	1,	UINT16_T,	NI_U,			GS_Split_AC_Current_SF,	0,	1 },	// 481 GS_Split_L1_Inverter_Sell_Current,
	{ 12,	1,	UINT16_T,	VOLTS_U,		GS_Split_AC_Voltage_SF,	0,	1 },	// 482 GS_Split_L1_Grid_Input_AC_Voltage,
	{ 13,	1,	UINT16_T,	VOLTS_U,		GS_Split_AC_Voltage_SF,	0,	1 },	// 483 GS_Split_L1_Gen_Input_AC_Voltage,
	{ 14,	1,	UINT16_T,	VOLTS_U,		GS_Split_AC_Voltage_SF,	0,	1 },	// 484 GS_Split_L1_AC_Output_Voltage,
	{ 15,	1,	UINT16_T,	NI_U,			GS_Split_AC_Current_SF,	0,	1 },	// 485 GS_Split_L2_Inverter_Output_Current,
	{ 16,	1,	UINT16_T,	NI_U,			GS_Split_AC_Current_SF,	0,	1 },	// 486 GS_Split_L2_Inverter_Charge_Current,
	{ 17,	1,	UINT16_T,	NI_U,			GS_Split_AC_Current_SF,	0,	1 },	// 487 GS_Split_L2_Inverter_Buy_Current,
	{ 18,	1,	UINT16_T,	NI_U,			GS_Split_AC_Current_SF,	0,	1 },	// 488 GS_Split_L2_Inverter_Sell_Current,
	{ 19,	1,	UINT16_T,	VOLTS_U,		GS_Split_AC_Voltage_SF,	0,	1 },	// 489 GS_Split_L2_Grid_Input_AC_Voltage,
	{ 20,	1,	UINT16_T,	VOLTS_U,		GS_Split_AC_Voltage_SF,	0,	1 },	// 490 GS_Split_L2_Gen_Input_AC_Voltage,
	{ 21,	1,	UINT16_T,	VOLTS_U,		GS_Split_AC_Voltage_SF,	0,	1 },	// 491 GS_Split_L2_AC_Output_Voltage,
	{ 22,	1,	UINT16_T,	ENUMERATED_U,	NI_SF,					0,	1 },	// 492 GS_Split_Inverter_Operating_Mode,
	{ 23,	1,	UINT16_T,	BITFIELD_U,		NI_SF,					0,	1 },	// 493 GS_Split_Error_Flags,
	{ 24,	1,	UINT16_T,	BITFIELD_U,		NI_SF,					0,	1 },	// 494 GS_Split_Warning_Flags,
	{ 25,	1,	UINT16_T,	VOLTS_U,		GS_Split_DC_Voltage_SF,	0,	1 },	// 495 GS_Split_Battery_Voltage,
	{ 26,	1,	UINT16_T,	VOLTS_U,		GS_Split_DC_Voltage_SF,	0,	1 },	// 496 GS_Split_Temp_Comp_Target_Voltage,
	{ 27,	1,	UINT16_T,	ENUMERATED_U,	NI_SF,					0,	1 },	// 497 GS_Split_AUX_Output_State,
	{ 28,	1,	UINT16_T,	ENUMERATED_U,	NI_SF,					0,	1 },	// 498 GS_Split_AUX_Relay_Output_State,
	{ 29,	1,	INT16_T,	DEGREES_C_U,	NI_SF,					0,	1 },	// 499 GS_Split_L_Module_Transformer_Temp,
	{ 30,	1,	INT16_T,	DEGREES_C_U,	NI_SF,					0,	1 },	// 500 GS_Split_L_Module_Capacitor_Temp,
	{ 31,	1,	INT16_T,	DEGREES_C_U,	NI_SF,					0,	1 },	// 501 GS_Split_L_Module_FET_Temp,
	{ 32,	1,	INT16_T,	DEGREES_C_U,	NI_SF,					0,	1 },	// 502 GS_Split_R_Module_Transformer_Temp,
	{ 33,	1,	INT16_T,	DEGREES_C_U,	NI_SF,					0,	1 },	// 503 GS_Split_R_Module_Capacitor_Temp,
	{ 34,	1,	INT16_T,	DEGREES_C_U,	NI_SF,					0,	1 },	// 504 GS_Split_R_Module_FET_Temp,
	{ 35,	1,	INT16_T,	DEGREES_C_U,	NI_SF,					0,	1 },	// 505 GS_Split_Battery_Temperature,
	{ 36,	1,	UINT16_T,	ENUMERATED_U,	NI_SF,					0,	1 },	// 506 GS_Split_AC_Input_Selection,
	{ 37,	1,	UINT16_T,	CYCLES_U,		GS_Split_Frequency_SF,	0,	1 },	// 507 GS_Split_AC_Input_Frequency,
	{ 38,	1,	UINT16_T,	VOLTS_U,		GS_Split_AC_Voltage_SF,	0,	1 },	// 508 GS_Split_AC_Input_Voltage,
	{ 39,	1,	UINT16_T,	ENUMERATED_U,	NI_SF,					0,	1 },	// 509 GS_Split_AC_Input_State,
	{ 40,	1,	UINT16_T,	VOLTS_U,		GS_Split_AC_Voltage_SF,	1,	1 },	// 510 GS_Split_Minimum_AC_Input_Voltage,
	{ 41,	1,	UINT16_T,	VOLTS_U,		GS_Split_AC_Voltage_SF,	1,	1 },	// 511 GS_Split_Maximum_AC_Input_Voltage,
	{ 42,	1,	UINT16_T,	BITFIELD_U,		NI_SF,					0,	1 },	// 512 GS_Split_Sell_Status,
	{ 43,	1,	INT16_T,	NI_U,			NI_SF,					0,	1 },	// 513 GS_Split_kWh_SF
 	{ 44,	1,	UINT16_T,	KWH_U,			GS_Split_kWh_SF,		0,	1 },	// 514 GS_Split_AC1_L1_Buy_kWh
 	{ 45,	1,	UINT16_T,	KWH_U,			GS_Split_kWh_SF,		0,	1 },	// 515 GS_Split_AC2_L1_Buy_kWh
 	{ 46,	1,	UINT16_T,	KWH_U,			GS_Split_kWh_SF,		0,	1 },	// 516 GS_Split_AC1_L1_Sell_kWh
 	{ 47,	1,	UINT16_T,	KWH_U,			GS_Split_kWh_SF,		0,	1 },	// 517 GS_Split_AC2_L1_Sell_kWh
 	{ 48,	1,	UINT16_T,	KWH_U,			GS_Split_kWh_SF,		0,	1 },	// 518 GS_Split_L1_Output_kWh
 	{ 49,	1,	UINT16_T,	KWH_U,			GS_Split_kWh_SF,		0,	1 },	// 519 GS_Split_AC1_L2_Buy_kWh
 	{ 50,	1,	UINT16_T,	KWH_U,			GS_Split_kWh_SF,		0,	1 },	// 520 GS_Split_AC2_L2_Buy_kWh
 	{ 51, 	1,	UINT16_T,	KWH_U,			GS_Split_kWh_SF,		0,	1 },	// 521 GS_Split_AC1_L2_Sell_kWh
 	{ 52,	1,	UINT16_T,	KWH_U,			GS_Split_kWh_SF,		0,	1 },	// 522 GS_Split_AC2_L2_Sell_kWh
 	{ 53,	1,	UINT16_T,	KWH_U,			GS_Split_kWh_SF,		0,	1 },	// 523 GS_Split_L2_Output_kWh
 	{ 54,	1,	UINT16_T,	KWH_U,			GS_Split_kWh_SF,		0,	1 },	// 524 GS_Split_Charger_kWh
 	{ 55,	1,	UINT16_T,	KW_U,			GS_Split_kWh_SF,		0,	1 },	// 525 GS_Split_Output_kW
 	{ 56,	1,	UINT16_T,	KW_U,			GS_Split_kWh_SF,		0,	1 },	// 526 GS_Split_Buy_kW
 	{ 57,	1,	UINT16_T,	KW_U,			GS_Split_kWh_SF,		0,	1 },	// 527 GS_Split_Sell_kW
 	{ 58,	1,	UINT16_T,	KW_U,			GS_Split_kWh_SF,		0,	1 },	// 528 GS_Split_Charge_kW
 	{ 59,	1,	UINT16_T,	KW_U,			GS_Split_kWh_SF,		0,	1 },	// 529 GS_Split_Load_kW
 	{ 60,	1,	UINT16_T,	KW_U,			GS_Split_kWh_SF,		0,	1 },	// 530 GS_Split_AC_Couple_kW	
//
	{ 1,	1,	UINT16_T,	NI_U,			NI_SF,					0,	1 },	// 531 FXconfig_SunSpec_DID = 64114,
	{ 2,	1,	UINT16_T,	REGISTERS_U,	NI_SF,					0,	1 },	// 532 FXconfig_SunSpec_Length,
	{ 3,	1,	UINT16_T,	NI_U,			NI_SF,					0,	1 },	// 533 FXconfig_Port_Number,
	{ 4,	1,	INT16_T,	NI_U,			NI_SF,					0,	1 },	// 534 FXconfig_DC_Voltage_SF,
	{ 5,	1,	INT16_T,	NI_U,			NI_SF,					0,	1 },	// 535 FXconfig_AC_Current_SF,
	{ 6,	1,	INT16_T,	NI_U,			NI_SF,					0,	1 },	// 536 FXconfig_AC_Voltage_SF,
	{ 7,	1,	INT16_T,	NI_U,			NI_SF,					0,	1 },	// 537 FXconfig_Time_SF,
	{ 8,	1,	UINT16_T,	NI_U,			NI_SF,					0,	1 },	// 538 FXconfig_Major_Firmware_Number,
	{ 9,	1,	UINT16_T,	NI_U,			NI_SF,					0,	1 },	// 539 FXconfig_Mid_Firmware_Number,
	{ 10,	1,	UINT16_T,	NI_U,			NI_SF,					0,	1 },	// 540 FXconfig_Minor_Firmware_Number,	
	{ 11,	1,	UINT16_T,	VOLTS_U,		FXconfig_DC_Voltage_SF,	1,	1 },	// 541 FXconfig_Absorb_Volts,
	{ 12,	1,	UINT16_T,	HOURS_U,		FXconfig_Time_SF,		1,	1 },	// 542 FXconfig_Absorb_Time_Hours,
	{ 13,	1,	UINT16_T,	VOLTS_U,		FXconfig_DC_Voltage_SF,	1,	1 },	// 543 FXconfig_Float_Volts,
	{ 14,	1,	UINT16_T,	HOURS_U,		FXconfig_Time_SF,		1,	1 },	// 544 FXconfig_Float_Time_Hours,
	{ 15,	1,	UINT16_T,	VOLTS_U,		FXconfig_DC_Voltage_SF,	1,	1 },	// 545 FXconfig_ReFloat_Volts,
	{ 16,	1,	UINT16_T,	VOLTS_U,		FXconfig_DC_Voltage_SF,	1,	1 },	// 546 FXconfig_EQ_Volts,
	{ 17,	1,	UINT16_T,	HOURS_U,		FXconfig_Time_SF,		1,	1 },	// 547 FXconfig_EQ_Time_Hours,
	{ 18,	1,	UINT16_T,	NI_U,			NI_SF,					1,	1 },	// 548 FXconfig_Search_Sensitivity,
	{ 19,	1,	UINT16_T,	CYCLES_U,		NI_SF,					1,	1 },	// 549 FXconfig_Search_Pulse_Length,
	{ 20,	1,	UINT16_T,	CYCLES_U,		NI_SF,					1,	1 },	// 550 FXconfig_Search_Pulse_Spacing,
	{ 21,	1,	UINT16_T,	ENUMERATED_U,	NI_SF,					1,	1 },	// 551 FXconfig_AC_Input_Type,
	{ 22,	1,	UINT16_T,	ENUMERATED_U,	NI_SF,					1,	1 },	// 552 FXconfig_Input_Support,
	{ 23,	1,	UINT16_T,	NI_U,			FXconfig_AC_Current_SF,	1,	1 },	// 553 FXconfig_Grid_AC_Input_Current_Limit,
	{ 24,	1,	UINT16_T,	NI_U,			FXconfig_AC_Current_SF,	1,	1 },	// 554 FXconfig_Gen_AC_Input_Current_Limit,
	{ 25,	1,	UINT16_T,	NI_U,			FXconfig_AC_Current_SF,	1,	1 },	// 555 FXconfig_Charger_AC_Input_Current_Limit,	
	{ 26,	1,	UINT16_T,	ENUMERATED_U,	NI_SF,					1,	1 },	// 556 FXconfig_Charger_Operating_Mode	
	{ 27,	1,	UINT16_T,	VOLTS_U,		FXconfig_AC_Voltage_SF,	1,	1 },	// 557 FXconfig_Grid_Lower_Input_Voltage_Limit,
	{ 28,	1,	UINT16_T,	VOLTS_U,		FXconfig_AC_Voltage_SF,	1,	1 },	// 558 FXconfig_Grid_Upper_Input_Voltage_Limit,
	{ 29,	1,	UINT16_T,	CYCLES_U,		NI_SF,					1,	1 },	// 559 FXconfig_Grid_Transfer_Delay,
	{ 30,	1,	UINT16_T,	VOLTS_U,		FXconfig_AC_Voltage_SF,	1,	1 },	// 560 FXconfig_Gen_lower_Input_Voltage_Limit,
	{ 31,	1,	UINT16_T,	VOLTS_U,		FXconfig_AC_Voltage_SF,	1,	1 },	// 561 FXconfig_Gen_Upper_Input_Voltage_Limit,
	{ 32,	1,	UINT16_T,	CYCLES_U,		NI_SF,					1,	1 },	// 562 FXconfig_Gen_Transfer_Delay,
	{ 33,	1,	UINT16_T,	MINS_U,			FXconfig_Time_SF,		1,	1 },	// 563 FXconfig_Gen_Connect_Delay,
	{ 34,	1,	UINT16_T,	VOLTS_U,		FXconfig_AC_Voltage_SF,	1,	1 },	// 564 FXconfig_AC_Output_Voltage,
	{ 35,	1,	UINT16_T,	VOLTS_U,		FXconfig_DC_Voltage_SF,	1,	1 },	// 565 FXconfig_Low_Battery_Cut_Out_Voltage,
	{ 36,	1,	UINT16_T,	VOLTS_U,		FXconfig_DC_Voltage_SF,	1,	1 },	// 566 FXconfig_Low_Battery_Cut_In_Voltage,
	{ 37,	1,	UINT16_T,	ENUMERATED_U,	NI_SF,					1,	1 },	// 567 FXconfig_AUX_Mode,
	{ 38,	1,	UINT16_T,	ENUMERATED_U,	NI_SF,					1,	1 },	// 568 FXconfig_AUX_Control
	{ 39,	1,	UINT16_T,	VOLTS_U,		FXconfig_DC_Voltage_SF,	1,	1 },	// 569 FXconfig_AUX_Load_Shed_Enable_Voltage,
	{ 40,	1,	UINT16_T,	VOLTS_U,		FXconfig_DC_Voltage_SF,	1,	1 },	// 570 FXconfig_AUX_Gen_Alert_On_Voltage,
	{ 41,	1,	UINT16_T,	MINS_U,			NI_SF,					1,	1 },	// 571 FXconfig_AUX_Gen_Alert_On_Delay,
	{ 42,	1,	UINT16_T,	VOLTS_U,		FXconfig_DC_Voltage_SF,	1,	1 },	// 572 FXconfig_AUX_Gen_Alert_OFF_Voltage,
	{ 43,	1,	UINT16_T,	MINS_U,			NI_SF,					1,	1 },	// 573 FXconfig_AUX_Gen_Alert_OFF_Delay,
	{ 44,	1,	UINT16_T,	VOLTS_U,		FXconfig_DC_Voltage_SF,	1,	1 },	// 574 FXconfig_AUX_Vent_Fan_Enable_Voltage,
	{ 45,	1,	UINT16_T,	MINS_U,			NI_SF,					1,	1 },	// 575 FXconfig_AUX_Vent_Fan_Off_Period,
	{ 46,	1,	UINT16_T,	VOLTS_U,		FXconfig_DC_Voltage_SF,	1,	1 },	// 576 FXconfig_AUX_Divert_Enable_Voltage,
	{ 47,	1,	UINT16_T,	MINS_U,			NI_SF,					1,	1 },	// 577 FXconfig_AUX_Divert_Off_Delay,
	{ 48,	1,	UINT16_T,	ENUMERATED_U,	NI_SF,					1,	1 },	// 578 FXconfig_Stacking_Mode,
	{ 49,	1,	UINT16_T,	NI_U,			NI_SF,					1,	1 },	// 579 FXconfig_Master_Power_Save_Level,
	{ 50,	1,	UINT16_T,	NI_U,			NI_SF,					1,	1 },	// 580 FXconfig_Slave_Power_Save_Level,
	{ 51,	1,	UINT16_T,	VOLTS_U,		FXconfig_DC_Voltage_SF,	1,	1 },	// 581 FXconfig_Sell_Volts,
	{ 52,	1,	UINT16_T,	ENUMERATED_U,	NI_SF,					1,	1 },	// 582 FXconfig_Grid_Tie_Window,
	{ 53,	1,	UINT16_T,	ENUMERATED_U,	NI_SF,					1,	1 },	// 583 FXconfig_Grid_Tie_Enable,
	{ 54,	1,	INT16_T,	VOLTS_U,		NI_SF,					1,	1 },	// 584 FXconfig_AC_Input_Voltage_Cal_Factor,
	{ 55,	1,	INT16_T,	VOLTS_U,		NI_SF,					1,	1 },	// 585 FXconfig_AC_Output_Voltage_Cal_Factor,
	{ 56,	1,	INT16_T,	VOLTS_U,		FXconfig_DC_Voltage_SF,	1,	1 },	// 586 FXconfig_Battery_Voltage_Cal_Factor,
	{ 57,	9,	STRING_T,	NI_U,			NI_SF,					0,	1 },	// 587 FXconfig_Serial_Number,
	{ 66,	9,  STRING_T,	NI_U,			NI_SF,					0,	1 },	// 588 FXconfig_Model_Number,
//
	{ 1,	1,	UINT16_T,	NI_U,			NI_SF,					0,	1 },	// 589 FX_SunSpec_DID = 64113,
	{ 2,	1,	UINT16_T,	REGISTERS_U,	NI_SF,					0,	1 },	// 590 FX_SunSpec_Length,
	{ 3,	1,	UINT16_T,	NI_U,			NI_SF,					0,	1 },	// 591 FX_Port_Number,
	{ 4,	1,	INT16_T,	NI_U,			NI_SF,					0,	1 },	// 592 FX_DC_Voltage_SF,
	{ 5,	1,	INT16_T,	NI_U,			NI_SF,					0,	1 },	// 593 FX_AC_Current_SF,
	{ 6,	1,	INT16_T,	NI_U,			NI_SF,					0,	1 },	// 594 FX_AC_Voltage_SF,
	{ 7,	1,	INT16_T,	NI_U,			NI_SF,					0,	1 },	// 595 FX_AC_Frequency_SF,
	{ 8,	1,	UINT16_T,	NI_U,			FX_AC_Current_SF,		0,	1 },	// 596 FX_Inverter_Output_Current,
	{ 9,	1,	UINT16_T,	NI_U,			FX_AC_Current_SF,		0,	1 },	// 597 FX_Inverter_Charge_Current,
	{ 10,	1,	UINT16_T,	NI_U,			FX_AC_Current_SF,		0,	1 },	// 598 FX_Inverter_Buy_Current,
	{ 11,	1,	UINT16_T,	NI_U,			FX_AC_Current_SF,		0,	1 },	// 599 FX_Inverter_Sell_Current,
	{ 12,	1,	UINT16_T,	VOLTS_U,		FX_AC_Voltage_SF,		0,	1 },	// 600 FX_AC_Output_Voltage,
	{ 13,	1,	UINT16_T,	ENUMERATED_U,	NI_SF,					0,	1 },	// 601 FX_Inverter_Operating_Mode,
	{ 14,	1,	UINT16_T,	BITFIELD_U,		NI_SF,					0,	1 },	// 602 FX_Error_Flags,
	{ 15,	1,	UINT16_T,	BITFIELD_U,		NI_SF,					0,	1 },	// 603 FX_Warning_Flags,
	{ 16,	1,	UINT16_T,	VOLTS_U,		FX_DC_Voltage_SF,		0,	1 },	// 604 FX_Battery_Voltage,
	{ 17,	1,	UINT16_T,	VOLTS_U,		FX_DC_Voltage_SF,		0,	1 },	// 605 FX_Temp_Comp_Target_Voltage,
	{ 18,	1,	UINT16_T,	ENUMERATED_U,	NI_SF,					0,	1 },	// 606 FX_AUX_Output_State,
	{ 19,	1,	INT16_T,	DEGREES_C_U,	NI_SF,					0,	1 },	// 607 FX_Transformer_Temp,
	{ 20,	1,	INT16_T,	DEGREES_C_U,	NI_SF,					0,	1 },	// 608 FX_Capacitor_Temp,
	{ 21,	1,	INT16_T,	DEGREES_C_U,	NI_SF,					0,	1 },	// 609 FX_FET_Temp,
	{ 22,	1,	UINT16_T,	CYCLES_U,		FX_AC_Frequency_SF,		0,	1 },	// 610 FX_AC_Input_Frequency,
	{ 23,	1,	UINT16_T,	VOLTS_U,		FX_AC_Voltage_SF,		0,	1 },	// 611 FX_AC_Input_Voltage,
	{ 24,	1,	UINT16_T,	ENUMERATED_U,	NI_SF,					0,	1 },	// 612 FX_AC_Input_State,
	{ 25,	1,	UINT16_T,	VOLTS_U,		FX_AC_Voltage_SF,		1,	1 },	// 613 FX_Minimum_AC_Input_Voltage,
	{ 26,	1,	UINT16_T,	VOLTS_U,		FX_AC_Voltage_SF,		1,	1 },	// 614 FX_Maximum_AC_Input_Voltage,
	{ 27,	1,	UINT16_T,	BITFIELD_U,		NI_SF,					0,	1 },	// 615 FX_Sell_Status,	
	{ 28,	1,	INT16_T,	NI_U,			NI_SF,					0,	1 },	// 616 FX_kWh_SF
 	{ 29,	1,	UINT16_T,	KWH_U,			FX_kWh_SF,				0,	1 },	// 617 FX_Buy_kWh
 	{ 30,	1,	UINT16_T,	KWH_U,			FX_kWh_SF,				0,	1 },	// 618 FX_Sell_kWh
 	{ 31,	1,	UINT16_T,	KWH_U,			FX_kWh_SF,				0,	1 },	// 619 FX_Output_kWh
 	{ 32,	1,	UINT16_T,	KWH_U,			FX_kWh_SF,				0,	1 },	// 620 FX_Charger_kWh
 	{ 33,	1,	UINT16_T,	KW_U,			FX_kWh_SF,				0,	1 },	// 621 FX_Output_kW
 	{ 34,	1,	UINT16_T,	KW_U,			FX_kWh_SF,				0,	1 },	// 622 FX_Buy_kW
 	{ 35,	1,	UINT16_T,	KW_U,			FX_kWh_SF,				0,	1 },	// 623 FX_Sell_kW
 	{ 36,	1,	UINT16_T,	KW_U,			FX_kWh_SF,				0,	1 },	// 624 FX_Charge_kW
 	{ 37,	1,	UINT16_T,	KW_U,			FX_kWh_SF,				0,	1 },	// 625 FX_Load_kW
 	{ 38,	1,	UINT16_T,	KW_U,			FX_kWh_SF,				0,	1 },	// 626 FX_AC_Couple_kW	
//
	{ 1,	1,	UINT16_T,	NI_U,			NI_SF,					0,	1 },	// 627 FNconfig_SunSpec_DID = 64119,
	{ 2,	1,	UINT16_T,	REGISTERS_U,	NI_SF,					0,	1 },	// 628 FNconfig_SunSpec_Length,
	{ 3,	1,	UINT16_T,	NI_U,			NI_SF,					0,	1 },	// 629 FNconfig_Port_Number,
	{ 4,	1,	INT16_T,	NI_U,			NI_SF,					0,	1 },	// 630 FNconfig_DC_Voltage_SF,
	{ 5,	1,	INT16_T,	NI_U,			NI_SF,					0,	1 },	// 631 FNconfig_DC_Current_SF,
	{ 6,	1,	INT16_T,	NI_U,			NI_SF,					0,	1 },	// 632 FNconfig_kWh_SF,	
	{ 7,	1,	UINT16_T,	NI_U,			NI_SF,					0,	1 },	// 633 FNconfig_Major_Firmware_Number,
	{ 8,	1,	UINT16_T,	NI_U,			NI_SF,					0,	1 },	// 634 FNconfig_Mid_Firmware_Number,
	{ 9,	1,	UINT16_T,	NI_U,			NI_SF,					0,	1 },	// 635 FNconfig_Minor_Firmware_Number,
	{ 10,	1,	UINT16_T,	AH_U,			NI_SF,					1,	1 },	// 636 FNconfig_Battery_Capacity,
	{ 11,	1,	UINT16_T,	VOLTS_U,		FNconfig_DC_Voltage_SF,	1,	1 },	// 637 FNconfig_Charged_Volts,
	{ 12,	1,	UINT16_T,	MINS_U,			NI_SF,					1,	1 },	// 638 FNconfig_Charged_Time,
	{ 13,	1,	UINT16_T,	AMPS_U,			FNconfig_DC_Current_SF,	1,	1 },	// 639 FNconfig_Battery_Charged_Amps,
	{ 14,	1,	UINT16_T,	PERCENTAGE_U,	NI_SF,					1,	1 },	// 640 FNconfig_Charge_Factor,
	{ 15,	1,	UINT16_T,	ENUMERATED_U,	NI_SF,					1,	1 },	// 641 FNconfig_Shunt_A_Enabled,
	{ 16,	1,	UINT16_T,	ENUMERATED_U,	NI_SF,					1,	1 },	// 642 FNconfig_Shunt_B_Enabled,
	{ 17,	1,	UINT16_T,	ENUMERATED_U,	NI_SF,					1,	1 },	// 643 FNconfig_Shunt_C_Enabled,
	{ 18,	1,	UINT16_T,	ENUMERATED_U,	NI_SF,					1,	1 },	// 644 FNconfig_Relay_Control,
	{ 19,	1,	UINT16_T,	ENUMERATED_U,	NI_SF,					1,	1 },	// 645 FNconfig_Relay_Invert_Logic,
	{ 20,	1,	UINT16_T,	VOLTS_U,		FNconfig_DC_Voltage_SF,	1,	1 },	// 646 FNconfig_Relay_High_Voltage,
	{ 21,	1,	UINT16_T,	VOLTS_U,		FNconfig_DC_Voltage_SF,	1,	1 },	// 647 FNconfig_Relay_Low_Voltage,
	{ 22,	1,	UINT16_T,	PERCENTAGE_U,	NI_SF,					1,	1 },	// 648 FNconfig_Relay_SOC_High,
	{ 23,	1,	UINT16_T,	PERCENTAGE_U,	NI_SF,					1,	1 },	// 649 FNconfig_Relay_SOC_Low,
	{ 24,	1,	UINT16_T,	MINS_U,			NI_SF,					1,	1 },	// 650 FNconfig_Relay_High_Enable_Delay,
	{ 25,	1,	UINT16_T,	MINS_U,			NI_SF,					1,	1 },	// 651 FNconfig_Relay_Low_Enable_Delay,
	{ 26,	1,	UINT16_T,	DAYS_U,			NI_SF,					1,	1 },	// 652 FNconfig_Set_Data_Log_Day_Offset,
	{ 27,	1,	UINT16_T,	DAYS_U,			NI_SF,					0,	1 },	// 653 FNconfig_Get_Current_Data_Log_Day_Offset,
	{ 28,	1,	UINT16_T,	PERCENTAGE_U,	NI_SF,					0,	1 },	// 654 FNconfig_Datalog_Minimum_SOC,
	{ 29,	1,	UINT16_T,	AH_U,			NI_SF,					0,	1 },	// 655 FNconfig_Datalog_Input_AH,
	{ 30,	1,	UINT16_T,	KWH_U,			FNconfig_kWh_SF,		0,	1 },	// 656 FNconfig_Datalog_Input_kWh,
	{ 31,	1,	UINT16_T,	AH_U,			NI_SF,					0,	1 },	// 657 FNconfig_Datalog_Output_AH,
	{ 32,	1,	UINT16_T,	KWH_U,			FNconfig_kWh_SF,		0,	1 },	// 658 FNconfig_Datalog_Output_kWh,
	{ 33,	1,	INT16_T,	AH_U,			NI_SF,					0,	1 },	// 659 FNconfig_Datalog_NET_AH,
	{ 34,	1,	INT16_T,	KWH_U,			FNconfig_kWh_SF,		0,	1 },	// 660 FNconfig_Datalog_NET_kWh,
	{ 35,	1,	UINT16_T,	NI_U,			NI_SF,					0,	1 },	// 661 FNconfig_Clear_Data_Log_Read,
	{ 36,	1,	UINT16_T,	NI_U,			NI_SF,					1,	1 },	// 662 FNconfig_Clear_Data_Log_Write_Complement,
	{ 37,	9,	STRING_T,	NI_U,			NI_SF,					0,	1 },	// 663 FNconfig_Serial_Number,
	{ 46,	9,  STRING_T,	NI_U,			NI_SF,					0,	1 },	// 664 FNconfig_Model_Number,
//
 	{ 1,	1,	UINT16_T,	NI_U,			NI_SF,					0,	1 },	// 665 FN_SunSpec_DID = 64118,
	{ 2,	1,	UINT16_T,	REGISTERS_U,	NI_SF,					0,	1 },	// 666 FN_SunSpec_Length,
	{ 3,	1,	UINT16_T,	NI_U,			NI_SF,					0,	1 },	// 667 FN_Port_Number,
	{ 4,	1,	INT16_T,	NI_U,			NI_SF,					0,	1 },	// 668 FN_DC_Voltage_SF,
	{ 5,	1,	INT16_T,	NI_U,			NI_SF,					0,	1 },	// 669 FN_DC_Current_SF,
	{ 6,	1,	INT16_T,	NI_U,			NI_SF,					0,	1 },	// 670 FN_Time_SF,
	{ 7,	1,	INT16_T,	NI_U,			NI_SF,					0,	1 },	// 671 FN_kWh_SF,	
	{ 8,	1,	INT16_T,	NI_U,			NI_SF,					0,	1 },	// 672 FN_kW_SF,
	{ 9,	1,	INT16_T,	AMPS_U,			FN_DC_Current_SF,		0,	1 },	// 673 FN_Shunt_A_Current,
	{ 10,	1,	INT16_T,	AMPS_U,			FN_DC_Current_SF,		0,	1 },	// 674 FN_Shunt_B_Current,
	{ 11,	1,	INT16_T,	AMPS_U,			FN_DC_Current_SF,		0,	1 },	// 675 FN_Shunt_C_Current,
	{ 12,	1,	UINT16_T,	VOLTS_U,		FN_DC_Voltage_SF,		0,	1 },	// 676 FN_Battery_Voltage,
	{ 13,	1,	INT16_T,	AMPS_U,			FN_DC_Current_SF,		0,	1 },	// 677 FN_Battery_Current,
	{ 14,	1,	INT16_T,	DEGREES_C_U,	NI_SF,					0,	1 },	// 678 FN_Battery_Temperature,
	{ 15,	1,	UINT16_T,	BITFIELD_U,		NI_SF,					0,	1 },	// 679 FN_Status_Flags,
	{ 16,	1,	INT16_T,	AH_U,			NI_SF,					0,	1 },	// 680 FN_Shunt_A_Accumulated_AH,
	{ 17,	1,	INT16_T,	KWH_U,			FN_kWh_SF,				0,	1 },	// 681 FN_Shunt_A_Accumulated_kWh,
	{ 18,	1,	INT16_T,	AH_U,			NI_SF,					0,	1 },	// 682 FN_Shunt_B_Accumulated_AH,
	{ 19,	1,	INT16_T,	KWH_U,			FN_kWh_SF,				0,	1 },	// 682 FN_Shunt_B_Accumulated_kWh,
	{ 20,	1,	INT16_T,	AH_U,			NI_SF,					0,	1 },	// 684 FN_Shunt_C_Accumulated_AH,
	{ 21,	1,	INT16_T,	KWH_U,			FN_kWh_SF,				0,	1 },	// 685 FN_Shunt_C_Accumulated_kWh,
	{ 22,	1,	UINT16_T,	AMPS_U,			FN_DC_Current_SF,		0,	1 },	// 686 FN_Input_Current,
	{ 23,	1,	UINT16_T,	AMPS_U,			FN_DC_Current_SF,		0,	1 },	// 687 FN_Output_Current,
	{ 24,	1,	UINT16_T,	KW_U,			FN_kW_SF,				0,	1 },	// 688 FN_Input_kW,
	{ 25,	1,	UINT16_T,	KW_U,			FN_kW_SF,				0,	1 },	// 689 FN_Output_kW,
	{ 26,	1,	INT16_T,	KW_U,			FN_kW_SF,				0,	1 },	// 690 FN_Net_kW,
	{ 27,	1,	UINT16_T,	DAYS_U,			FN_Time_SF,				0,	1 },	// 691 FN_Days_Since_Charge_Parameters_Met,
	{ 28,	1,	UINT16_T,	PERCENTAGE_U,	NI_SF,					0,	1 },	// 692 FN_State_Of_Charge,
	{ 29,	1,	UINT16_T,	PERCENTAGE_U,	NI_SF,					0,	1 },	// 693 FN_Todays_Minimum_SOC,
	{ 30,	1,	UINT16_T,	PERCENTAGE_U,	NI_SF,					0,	1 },	// 694 FN_Todays_Maximum_SOC,
	{ 31,	1,	UINT16_T,	AH_U,			NI_SF,					0,	1 },	// 695 FN_Todays_NET_Input_AH,
	{ 32,	1,	UINT16_T,	KWH_U,			FN_kWh_SF,				0,	1 },	// 696 FN_Todays_NET_Input_kWh,
	{ 33,	1,	UINT16_T,	AH_U,			NI_SF,					0,	1 },	// 697 FN_Todays_NET_Output_AH,
	{ 34,	1,	UINT16_T,	KWH_U,			FN_kWh_SF,				0,	1 },	// 698 FN_Todays_NET_Output_kWh,	
	{ 35,	1,	INT16_T,	AH_U,			NI_SF,					0,	1 },	// 699 FN_Todays_NET_Battery_AH,
	{ 36,	1,	INT16_T,	KWH_U,			FN_kWh_SF,				0,	1 },	// 700 FN_Todays_NET_Battery_kWh,	
	{ 37,	1,	INT16_T,	AH_U,			NI_SF,					0,	1 },	// 701 FN_Charge_Factor_Corrected_NET_Battery_AH,
	{ 38,	1,	INT16_T,	KWH_U,			FN_kWh_SF,				0,	1 },	// 702 FN_Charge_Factor_Corrected_NET_Battery_kWh,
	{ 39,	1,	UINT16_T,	VOLTS_U,		FN_DC_Voltage_SF,		1,	1 },	// 703 FN_Todays_Minimum_Battery_Voltage,
	{ 40,	2,	UINT32_T,	TIME_U,			NI_SF,					0,	1 },	// 704 FN_Todays_Minimum_Battery_Time,
	{ 42,	1,	UINT16_T,	VOLTS_U,		FN_DC_Voltage_SF,		1,	1 },	// 705 FN_Todays_Maximum_Battery_Voltage,
	{ 43,	2,	UINT32_T,	TIME_U,			NI_SF,					0,	1 },	// 706 FN_Todays_Maximum_Battery_Time,
	{ 45,	1,	UINT16_T,	PERCENTAGE_U,	NI_SF,					0,	1 },	// 707 FN_Cycle_Charge_Factor,
	{ 46,	1,	UINT16_T,	PERCENTAGE_U,	NI_SF,					0,	1 },	// 708 FN_Cycle_kWh_Charge_Efficiency,
	{ 47,	1,	UINT16_T,	DAYS_U,			FN_Time_SF,				0,	1 },	// 709 FN_Total_Days_At_100_Percent,
	{ 48,	1,	UINT16_T,	AH_U,			NI_SF,					0,	1 },	// 710 FN_Lifetime_kAH_Removed,
	{ 49,	1,	UINT16_T,	AH_U,			NI_SF,					0,	1 },	// 711 FN_Shunt_A_Historical_Returned_To_Battery_AH,
	{ 50,	1,	UINT16_T,	KWH_U,			FN_kWh_SF,				0,	1 },	// 712 FN_Shunt_A_Historical_Returned_To_Battery_kWh,
	{ 51,	1,	UINT16_T,	AH_U,			NI_SF,					0,	1 },	// 713 FN_Shunt_A_Historical_Removed_From_Battery_AH,
	{ 52,	1,	UINT16_T,	KWH_U,			FN_kWh_SF,				0,	1 },	// 714 FN_Shunt_A_Historical_Removed_From_Battery_kWh,
	{ 53,	1,	UINT16_T,	AMPS_U,			FN_DC_Current_SF,		0,	1 },	// 715 FN_Shunt_A_Maximum_Charge_Rate,
	{ 54,	1,	UINT16_T,	KW_U,			FN_kWh_SF,				0,	1 },	// 716 FN_Shunt_A_Maximum_Charge_Rate_kW,
	{ 55,	1,	INT16_T,	AMPS_U,			FN_DC_Current_SF,		0,	1 },	// 717 FN_Shunt_A_Maximum_Discharge_Rate,
	{ 56,	1,	INT16_T,	KW_U,			FN_kWh_SF,				0,	1 },	// 718 FN_Shunt_A_Maximum_Discharge_Rate_kW,
	{ 57,	1,	UINT16_T,	AH_U,			NI_SF,					0,	1 },	// 719 FN_Shunt_B_Historical_Returned_To_Battery_AH,
	{ 58,	1,	UINT16_T,	KWH_U,			FN_kWh_SF,				0,	1 },	// 720 FN_Shunt_B_Historical_Returned_To_Battery_kWh,
	{ 59,	1,	UINT16_T,	AH_U,			NI_SF,					0,	1 },	// 721 FN_Shunt_B_Historical_Removed_From_Battery_AH,
	{ 60,	1,	UINT16_T,	KWH_U,			FN_kWh_SF,				0,	1 },	// 722 FN_Shunt_B_Historical_Removed_From_Battery_kWh,
   	{ 61,	1,	UINT16_T,	AMPS_U,			FN_DC_Current_SF,		0,	1 },	// 723 FN_Shunt_B_Maximum_Charge_Rate,
	{ 62,	1,	UINT16_T,	KW_U,			FN_kWh_SF,				0,	1 },	// 724 FN_Shunt_B_Maximum_Charge_Rate_kW,
	{ 63,	1,	INT16_T,	AMPS_U,			FN_DC_Current_SF,		0,	1 },	// 725 FN_Shunt_B_Maximum_Discharge_Rate,
	{ 64,	1,	INT16_T,	KW_U,			FN_kWh_SF,				0,	1 },	// 726 FN_Shunt_B_Maximum_Discharge_Rate_kW,
	{ 65,	1,	UINT16_T,	AH_U,			NI_SF,					0,	1 },	// 727 FN_Shunt_C_Historical_Returned_To_Battery_AH,
	{ 66,	1,	UINT16_T,	KWH_U,			FN_kWh_SF,				0,	1 },	// 728 FN_Shunt_C_Historical_Returned_To_Battery_kWh,
	{ 67,	1,	UINT16_T,	AH_U,			NI_SF,					0,	1 },	// 729 FN_Shunt_C_Historical_Removed_From_Battery_AH,
	{ 68,	1,	UINT16_T,	KWH_U,			FN_kWh_SF,				0,	1 },	// 730 FN_Shunt_C_Historical_Removed_From_Battery_kWh,
   	{ 69,	1,	UINT16_T,	AMPS_U,			FN_DC_Current_SF,		0,	1 },	// 731 FN_Shunt_C_Maximum_Charge_Rate,
	{ 70,	1,	UINT16_T,	KW_U,			FN_kWh_SF,				0,	1 },	// 732 FN_Shunt_C_Maximum_Charge_Rate_kW,
	{ 71,	1,	INT16_T,	AMPS_U,			FN_DC_Current_SF,		0,	1 },	// 733 FN_Shunt_C_Maximum_Discharge_Rate,
	{ 72,	1,	INT16_T,	KW_U,			FN_kWh_SF,				0,	1 },	// 734 FN_Shunt_C_Maximum_Discharge_Rate_kW,
	{ 73,	1,	UINT16_T,	NI_U,			NI_SF,					0,	1 },	// 735 FN_Shunt_A_Reset_Maximum_Data,
	{ 74,	1,	UINT16_T,	NI_U,			NI_SF,					1,	1 },	// 736 FN_Shunt_A_Reset_Maximum_Data_Write_Complement,
	{ 75,	1,	UINT16_T,	NI_U,			NI_SF,					0,	1 },	// 737 FN_Shunt_B_Reset_Maximum_Data,
	{ 76,	1,	UINT16_T,	NI_U,			NI_SF,					1,	1 },	// 738 FN_Shunt_B_Reset_Maximum_Data_Write_Complement,
	{ 77,	1,	UINT16_T,	NI_U,			NI_SF,					0,	1 },	// 739 FN_Shunt_C_Reset_Maximum_Data,
	{ 78,	1,	UINT16_T,	NI_U,			NI_SF,					1,	1 },	// 740 FN_Shunt_C_Reset_Maximum_Data_Write_Complement,
//                                                                                    
	{ 1,	1,	UINT16_T,	NI_U,			NI_SF,					0,	1 },	// 741 OP_stats_DID = 64255,
	{ 2,	1,	UINT16_T,	REGISTERS_U,	NI_SF,					0,	1 },	// 742 OP_stats_Length,
	{ 3,	1,	UINT16_T,	NI_U,			NI_SF,					0,	1 },	// 743 OP_stats_Bt_min,
	{ 4,	1,	UINT16_T,	NI_U,			NI_SF,					0,	1 },	// 744 OP_stats_Bt_max,
	{ 5,	1,	UINT16_T,	NI_U,			NI_SF,					0,	1 },	// 745 OP_stats_Bt_ave,
	{ 6,	1,	UINT16_T,	NI_U,			NI_SF,					0,	1 },	// 746 OP_stats_Bt_attempts,
	{ 7,	1,	UINT16_T,	NI_U,			NI_SF,					0,	1 },	// 747 OP_stats_Bt_errors,
	{ 8,	1,	UINT16_T,	NI_U,			NI_SF,					0,	1 },	// 748 OP_stats_Bt_timeouts,
	{ 9,	1,	UINT16_T,	NI_U,			NI_SF,					0,	1 },	// 749 OP_stats_Bt_packet_timeout,
	{ 10,	1,	UINT16_T,	NI_U,			NI_SF,					0,	1 },	// 750 OP_stats_Mp_min,
	{ 11,	1,	UINT16_T,	NI_U,			NI_SF,					0,	1 },	// 751 OP_stats_Mp_max,
	{ 12,	1,	UINT16_T,	NI_U,			NI_SF,					0,	1 },	// 752 OP_stats_Mp_ave,
	{ 13,	1,	UINT16_T,	NI_U,			NI_SF,					0,	1 },	// 753 OP_stats_Mp_attempts,
	{ 14,	1,	UINT16_T,	NI_U,			NI_SF,					0,	1 },	// 754 OP_stats_Mp_errors,
	{ 15,	1,	UINT16_T,	NI_U,			NI_SF,					0,	1 },	// 755 OP_stats_Mp_timeouts,
	{ 16,	1,	UINT16_T,	NI_U,			NI_SF,					0,	1 },	// 756 OP_stats_Mp_packet_timeout,
	{ 17,	1,	UINT16_T,	NI_U,			NI_SF,					0,	1 },	// 757 OP_stats_Cu_min,
	{ 18,	1,	UINT16_T,	NI_U,			NI_SF,					0,	1 },	// 758 OP_stats_Cu_max,
	{ 19,	1,	UINT16_T,	NI_U,			NI_SF,					0,	1 },	// 759 OP_stats_Cu_ave,
	{ 20,	1,	UINT16_T,	NI_U,			NI_SF,					0,	1 },	// 760 OP_stats_Cu_attempts,
	{ 21,	1,	UINT16_T,	NI_U,			NI_SF,					0,	1 },	// 761 OP_stats_Cu_errors,
	{ 22,	1,	UINT16_T,	NI_U,			NI_SF,					0,	1 },	// 762 OP_stats_Cu_timeouts,
	{ 23,	1,	UINT16_T,	NI_U,			NI_SF,					0,	1 },	// 763 OP_stats_Cu_packet_timeout,
	{ 24,	1,	UINT16_T,	NI_U,			NI_SF,					0,	1 },	// 764 OP_stats_Su_min,
	{ 25,	1,	UINT16_T,	NI_U,			NI_SF,					0,	1 },	// 765 OP_stats_Su_max,
	{ 26,	1,	UINT16_T,	NI_U,			NI_SF,					0,	1 },	// 766 OP_stats_Su_ave,
	{ 27,	1,	UINT16_T,	NI_U,			NI_SF,					0,	1 },	// 767 OP_stats_Su_attempts,
	{ 28,	1,	UINT16_T,	NI_U,			NI_SF,					0,	1 },	// 768 OP_stats_Su_errors,
	{ 29,	1,	UINT16_T,	NI_U,			NI_SF,					0,	1 },	// 769 OP_stats_Su_timeouts,
	{ 30,	1,	UINT16_T,	NI_U,			NI_SF,					0,	1 },	// 770 OP_stats_Su_packet_timeout,
	{ 31,	1,	UINT16_T,	NI_U,			NI_SF,					0,	1 },	// 771 OP_stats_Pg_min,
	{ 32,	1,	UINT16_T,	NI_U,			NI_SF,					0,	1 },	// 772 OP_stats_Pg_max,
	{ 33,	1,	UINT16_T,	NI_U,			NI_SF,					0,	1 },	// 773 OP_stats_Pg_ave,
	{ 34,	1,	UINT16_T,	NI_U,			NI_SF,					0,	1 },	// 774 OP_stats_Pg_attempts,
	{ 35,	1,	UINT16_T,	NI_U,			NI_SF,					0,	1 },	// 775 OP_stats_Pg_errors,
	{ 36,	1,	UINT16_T,	NI_U,			NI_SF,					0,	1 },	// 776 OP_stats_Pg_timeouts,
	{ 37,	1,	UINT16_T,	NI_U,			NI_SF,					0,	1 },	// 777 OP_stats_Pg_packet_timeout,
	{ 38,	1,	UINT16_T,	NI_U,			NI_SF,					0,	1 },	// 778 OP_stats_Mb_min,
	{ 39,	1,	UINT16_T,	NI_U,			NI_SF,					0,	1 },	// 779 OP_stats_Mb_max,
	{ 40,	1,	UINT16_T,	NI_U,			NI_SF,					0,	1 },	// 780 OP_stats_Mb_ave,
	{ 41,	1,	UINT16_T,	NI_U,			NI_SF,					0,	1 },	// 781 OP_stats_Mb_attempts,
	{ 42,	1,	UINT16_T,	NI_U,			NI_SF,					0,	1 },	// 782 OP_stats_Mb_errors,
	{ 43,	1,	UINT16_T,	NI_U,			NI_SF,					0,	1 },	// 783 OP_stats_Mb_timeouts,
	{ 44,	1,	UINT16_T,	NI_U,			NI_SF,					0,	1 },	// 784 OP_stats_Mb_packet_timeout,
	{ 45,	1,	UINT16_T,	NI_U,			NI_SF,					0,	1 },	// 785 OP_stats_Fu_min,
	{ 46,	1,	UINT16_T,	NI_U,			NI_SF,					0,	1 },	// 786 OP_stats_Fu_max,
	{ 47,	1,	UINT16_T,	NI_U,			NI_SF,					0,	1 },	// 787 OP_stats_Fu_ave,
	{ 48,	1,	UINT16_T,	NI_U,			NI_SF,					0,	1 },	// 788 OP_stats_Fu_attempts,
	{ 49,	1,	UINT16_T,	NI_U,			NI_SF,					0,	1 },	// 789 OP_stats_Fu_errors,
	{ 50,	1,	UINT16_T,	NI_U,			NI_SF,					0,	1 },	// 790 OP_stats_Fu_timeouts,
	{ 51,	1,	UINT16_T,	NI_U,			NI_SF,					0,	1 },	// 791 OP_stats_Fu_packet_timeout,
	{ 52,	1,	UINT16_T,	NI_U,			NI_SF,					0,	1 },	// 792 OP_stats_Ev_min,
	{ 53,	1,	UINT16_T,	NI_U,			NI_SF,					0,	1 },	// 793 OP_stats_Ev_max,
	{ 54,	1,	UINT16_T,	NI_U,			NI_SF,					0,	1 },	// 794 OP_stats_Ev_ave,
	{ 55,	1,	UINT16_T,	NI_U,			NI_SF,					0,	1 },	// 795 OP_stats_Ev_attempts,
	{ 56,	1,	UINT16_T,	NI_U,			NI_SF,					0,	1 },	// 796 OP_stats_Ev_errors,
	{ 57,	1,	UINT16_T,	NI_U,			NI_SF,					0,	1 },	// 797 OP_stats_Ev_timeouts,
	{ 58,	1,	UINT16_T,	NI_U,			NI_SF,					0,	1 },	// 798 OP_stats_Ev_packet_timeout
//
	{ 1,	1,	UINT16_T,	NI_U,			NI_SF,					0,	1 },	// 799 A_SunSpec_DID = 0002,
	{ 2,	1,	UINT16_T,	REGISTERS_U,	NI_SF,					0,	1 },	// 800 A_SunSpec_Length,
	{ 3,	1,	UINT16_T,	NI_U,			NI_SF,					0,	1 },	// 801 A_Devices,
	{ 4,	1,	UINT16_T,	NI_U,			NI_SF,					0,	1 },	// 802 A_Count,
	{ 5,	1,	UINT16_T,	NI_U,			NI_SF,					0,	1 },	// 803 A_Update_Number,
	{ 6,	1,	UINT16_T,	ENUMERATED_U,	NI_SF,					0,	1 },	// 804 A_Status,
	{ 7,	1,	UINT16_T,	ENUMERATED_U,	NI_SF,					0,	1 },	// 805 A_Status_Vendor,
	{ 8,	2,	UINT32_T,	BITFIELD_U,		NI_SF,					0,	1 },	// 806 A_Event,
	{ 10,	2,	UINT32_T,	BITFIELD_U,		NI_SF,					0,	1 },	// 807 A_Event_Vendor,
	{ 12,	1,	UINT16_T,	ENUMERATED_U,	NI_SF,					1,	1 },	// 808 A_Control,
	{ 13,	2,	UINT32_T,	ENUMERATED_U,	NI_SF,					1,	1 },	// 809 A_Control_Vendor,
	{ 15,	2,	UINT32_T,	ENUMERATED_U,	NI_SF,					1,	1 },	// 810 A_Control_Value,
	{ 1,	1,	UINT16_T,	NI_U,			NI_SF,					0,	1 },	// 811 END_SunSpec_DID = 65535,
	{ 2,	1,	UINT16_T,	REGISTERS_U,	NI_SF,					0,	1 }		// 812 END_SunSpec_Length
};

//field names
const char* fnames[] = {
	"C_SunSpec_ID    ",																// 0
	"C_SunSpec_DID   ",
	"C_SunSpec_Length",
	"C_Manufacturer  ",
	"C_Model         ",
	"C_Options       ",
	"C_Version       ",
	"C_SerialNumber  ",
	"C_DeviceAddress ",																// 8
	"OutBack_SunSpec_DID                                  ",						// 9
	"OutBack_SunSpec_Length                               ",						// 10
	"OutBack_Major_Firmware_Number                        ",
	"OutBack_Mid_Firmware_Number                          ",
	"OutBack_Minor_Firmware_Number                        ",
	"OutBack_Encryption_Key                               ",
	"OutBack_MAC_Address                                  ",
	"OutBack_Write_Password                               ",
	"OutBack_Enable_DHCP                                  ",
	"OutBack_TCPIP_Address                                ",
	"OutBack_TCPIP_Gateway                                ",
	"OutBack_TCPIP_Netmask                                ",						// 20
	"OutBack_TCPIP_DNS_1                                  ",
	"OutBack_TCPIP_DNS_2                                  ",
	"OutBack_Modbus_Port                                  ",
	"OutBack_SMTP_Server_Name                             ",
	"OutBack_SMTP_Account_Name                            ",
	"OutBack_SMTP_SSL_Enable                              ",
	"OutBack_SMTP_Email_Password                          ",
	"OutBack_SMTP_Email_User_Name                         ",
	"OutBack_Status_Email_Interval                        ",
	"OutBack_Status_Email_Status_Time                     ",						// 30
	"OutBack_Status_Email_Subject_Line                    ",
	"OutBack_Status_Email_To_Address_1                    ",
	"OutBack_Status_Email_To_Address_2                    ",
	"OutBack_Alarm_Email_Enable                           ",
	"OutBack_Alarm_Email_Subject_Line                     ",
	"OutBack_Alarm_Email_To_Address_1                     ",
	"OutBack_Alarm_Email_To_Address_2                     ",
	"OutBack_FTP_Password                                 ",
	"OutBack_Telnet_Password                              ",
	"OutBack_SD_Card_Log_Write_Interval                   ",						// 40
	"OutBack_SD_Card_Log_Retain_Days                      ",
	"OutBack_SD_Card_Logging_Mode                         ",
	"OutBack_Time_Server_Name                             ",
	"OutBack_Enable_Time_Server                           ",
	"OutBack_Set_Time_Zone                                ",
	"OutBack_Enable_Float_Coordination                    ",
	"OutBack_Enable_FNDC_Charge_Termination               ",
	"OutBack_Enable_FNDC_Grid_Tie_Control                 ",
	"OutBack_Voltage_SF                                   ",                                     
	"OutBack_Hour_SF                                      ",						// 50                                        
	"OutBack_AGS_Mode                                     ",                                       
	"OutBack_AGS_Port                                     ",
	"OutBack_AGS_Port_Type                                ",                                       
	"OutBack_Generator_Type                               ",                                 
	"OutBack_AGS_DC_Gen_Absorb_Voltage                    ",                      
	"OutBack_AGS_DC_Gen_Absorb_Time                       ",                         
	"OutBack_AGS_Fault_Time                               ",                                 
	"OutBack_AGS_Gen_Cool_Down_Time                       ",                         
	"OutBack_AGS_Gen_Warm_Up_Time                         ",                           
	"OutBack_Generator_Exercise_Mode                      ",						// 60                        
	"OutBack_Exercise_Start_Hour                          ",                            
	"OutBack_Exercise_Start_Minute                        ",                          
	"OutBack_Exercise_Day                                 ",                                   
	"OutBack_Exercise_Period                              ",                                
	"OutBack_Exercise_Interval                            ",                              
	"OutBack_AGS_Sell_Mode                                ",                                  
	"OutBack_AGS_2_Min_Start_Mode                         ",                           
	"OutBack_AGS_2_Min_Start_Voltage                      ",                        
	"OutBack_AGS_2_Hour_Start_Mode                        ",                          
	"OutBack_AGS_2_Hour_Start_Voltage                     ",						// 70                       
	"OutBack_AGS_24_Hour_Start_Mode                       ",                         
	"OutBack_AGS_24_Hour_Start_Voltage                    ",                      
	"OutBack_AGS_Load_Start_Mode                          ",                            
	"OutBack_AGS_Load_Start_kW                            ",                              
	"OutBack_AGS_Load_Start_Delay                         ",                           
	"OutBack_AGS_Load_Stop_kW                             ",                               
	"OutBack_AGS_Load_Stop_Delay                          ",                            
	"OutBack_AGS_SOC_Start_Mode                           ",                             
	"OutBack_AGS_SOC_Start_Percentage                     ",                       
	"OutBack_AGS_SOC_Stop_Percentage                      ",						// 80                        
	"OutBack_AGS_Enable_Full_Charge_Mode                  ",                    
	"OutBack_AGS_Full_Charge_Interval                     ",                       
	"OutBack_AGS_Must_Run_Mode                            ",                              
	"OutBack_AGS_Must_Run_Weekday_Start_Hour              ",                
	"OutBack_AGS_Must_Run_Weekday_Start_Minute            ",              
	"OutBack_AGS_Must_Run_Weekday_Stop_Hour               ",                 
	"OutBack_AGS_Must_Run_Weekday_Stop_Minute             ",               
	"OutBack_AGS_Must_Run_Weekend_Start_Hour              ",                
	"OutBack_AGS_Must_Run_Weekend_Start_Minute            ",              
	"OutBack_AGS_Must_Run_Weekend_Stop_Hour               ",						// 90                 
	"OutBack_AGS_Must_Run_Weekend_Stop_Minute             ",               
	"OutBack_AGS_Quiet_Time_Mode                          ",                            
	"OutBack_AGS_Quiet_Time_Weekday_Start_Hour            ",              
	"OutBack_AGS_Quiet_Time_Weekday_Start_Minute          ",            
	"OutBack_AGS_Quiet_Time_Weekday_Stop_Hour             ",               
	"OutBack_AGS_Quiet_Time_Weekday_Stop_Minute           ",             
	"OutBack_AGS_Quiet_Time_Weekend_Start_Hour            ",              
	"OutBack_AGS_Quiet_Time_Weekend_Start_Minute          ",            
	"OutBack_AGS_Quiet_Time_Weekend_Stop_Hour             ",             
	"OutBack_AGS_Quiet_Time_Weekend_Stop_Minute           ",						// 100             
	"OutBack_AGS_Total_Generator_Run_Time                 ",                   
	"OutBack_HBX_Mode                                     ",                                       
	"OutBack_HBX_Grid_Connect_Voltage                     ",                       
	"OutBack_HBX_Grid_Connect_Delay                       ",                         
	"OutBack_HBX_Grid_Disconnect_Voltage                  ",                    
	"OutBack_HBX_Grid_Disconnect_Delay                    ",                      
	"OutBack_HBX_Grid_Connect_SOC                         ",                           
	"OutBack_HBX_Grid_Disconnect_SOC                      ",                        
	"OutBack_Grid_Use_Interval_1_Mode                     ",                      
	"OutBack_Grid_Use_Interval_1_Weekday_Start_Hour       ",						// 110         
	"OutBack_Grid_Use_Interval_1_Weekday_Start_Minute     ",       
	"OutBack_Grid_Use_Interval_1_Weekday_Stop_Hour        ",          
	"OutBack_Grid_Use_Interval_1_Weekday_Stop_Minute      ",        
	"OutBack_Grid_Use_Interval_1_Weekend_Start_Hour       ",         
	"OutBack_Grid_Use_Interval_1_Weekend_Start_Minute     ",       
	"OutBack_Grid_Use_Interval_1_Weekend_Stop_Hour        ",          
	"OutBack_Grid_Use_Interval_1_Weekend_Stop_Minute      ",        
	"OutBack_Grid_Use_Interval_2_Mode                     ",                       
	"OutBack_Grid_Use_Interval_2_Weekday_Start_Hour       ",         
	"OutBack_Grid_Use_Interval_2_Weekday_Start_Minute     ",						// 120       
	"OutBack_Grid_Use_Interval_2_Weekday_Stop_Hour        ",          
	"OutBack_Grid_Use_Interval_2_Weekday_Stop_Minute      ",        
	"OutBack_Grid_Use_Interval_3_Mode                     ",                       
	"OutBack_Grid_Use_Interval_3_Weekday_Start_Hour       ",         
	"OutBack_Grid_Use_Interval_3_Weekday_Start_Minute     ",       
	"OutBack_Grid_Use_Interval_3_Weekday_Stop_Hour        ",          
	"OutBack_Grid_Use_Interval_3_Weekday_Stop_Minute      ",        
	"OutBack_Load_Grid_Transfer_Mode                      ",                        
	"OutBack_Load_Grid_Transfer_Threshold                 ",                   
	"OutBack_Load_Grid_Transfer_Connect_Delay             ",						// 130               
	"OutBack_Load_Grid_Transfer_Disconnect_Delay          ",            
	"OutBack_Load_Grid_Transfer_Connect_Battery_Voltage   ",     
	"OutBack_Load_Grid_Transfer_Re_Connect_Battery_Voltage",  
	"OutBack_Global_Charger_Control_Mode                  ",                    
	"OutBack_Global_Charger_Output_Limit                  ",                    
	"OutBack_Radian_AC_Coupled_Mode                       ",                         
	"OutBack_Radian_AC_Coupled_AUX_Port                   ",                     
	"OutBack_URL_Lock                                     ",                                       
	"OutBack_Web_Reporting_Base_URL                       ",                         
	"OutBack_Web_User_Logged_In_Status                    ",						// 140                      
	"OutBack_HUB_Type                                     ",                                       
	"OutBack_HUB_Major_Firmware_Number                    ",                      
	"OutBack_HUB_Mid_Firmware_Number                      ",                        
	"OutBack_HUB_Minor_Firmware_Number                    ",                      
	"OutBack_Year                                         ",
	"OutBack_Month                                        ",
	"OutBack_Day                                          ",
	"OutBack_Hour                                         ",
	"OutBack_Minute                                       ",
	"OutBack_Second                                       ",						// 150
	"OutBack_Temp_Batt                                    ",
	"OutBack_Temp_Ambient                                 ",
	"OutBack_Temp_SF                                      ",
	"OutBack_Error                                        ",
	"OutBack_Status                                       ",
	"OutBack_Update_Device_Firmware_Port                  ",
	"OutBack_Gateway_Type                                 ",
	"OutBack_System_Voltage                               ",                                   
    "OutBack_Measured_System_Voltage                      ",
	"OutBack_AGS_AC_Reconnect_Delay                       ",						// 160 
    "OutBack_Multi_Phase_Coordination                     ",
    "OutBack_Sched_1_AC_Mode                              ",        
    "OutBack_Sched_1_AC_Mode_Hour                         ",   
    "OutBack_Sched_1_AC_Mode_Min                          ",    
    "OutBack_Sched_2_AC_Mode                              ",        
    "OutBack_Sched_2_AC_Mode_Hour                         ",   
    "OutBack_Sched_2_AC_Mode_Min                          ",    
    "OutBack_Sched_3_AC_Mode                              ",        
    "OutBack_Sched_3_AC_Mode_Hour                         ", 
    "OutBack_Sched_3_AC_Mode_Min                          ",						// 170    
    "OutBack_Auto_reboot                                  ",            
    "OutBack_Spare_Reg_2                                  ",            
    "OutBack_Spare_Reg_3                                  ",            
    "OutBack_Spare_Reg_4                                  ",						// 174            
	"OB_SunSpec_DID                           ",									// 175
	"OB_SunSpec_Length                        ",
	"OB_DC_Voltage_SF                         ",
	"OB_AC_Current_SF                         ",
	"OB_Time_SF                               ",
	"OB_Bulk_Charge_Enable_Disable            ",									// 180
	"OB_Inverter_AC_Drop_Use                  ",
	"OB_Set_Inverter_Mode                     ",
	"OB_Grid_Tie_Mode                         ",
	"OB_Set_Inverter_Charger_Mode             ",
	"OB_Control_Status                        ",
	"OB_Set_Sell_Voltage                      ",
	"OB_Set_Radian_Inverter_Sell_Current_Limit",
	"OB_Set_Absorb_Voltage                    ",
	"OB_Set_Absorb_Time                       ",
	"OB_Set_Float_Voltage                     ",									// 190
	"OB_Set_Float_Time                        ",
	"OB_Set_Inverter_Charger_Current_Limit    ",
	"OB_Set_Inverter_AC1_Current_Limit        ",
	"OB_Set_Inverter_AC2_Current_Limit        ",
	"OB_Set_AGS_OP_Mode                       ",      	
	"OB_AGS_Operational_State                 ",       	
	"OB_AGS_Operational_State_Timer           ", 	
	"OB_Gen_Last_Run_Start_Time_GMT           ", 	
	"OB_Gen_Last_Run_Duration                 ",
	"OB_Set_AC_Output_Freq_Offline_Mode       ",									// 200 
    "OB_Set_AC_Output_Offline_Freq            ",									// 201      	       		
	"CC_SunSpec_DID               ",												// 202
	"CC_SunSpec_Length            ",
	"CC_port_number               ",
	"CC_Voltage_SF                ",
	"CC_Current_SF                ",
	"CC_Power_SF                  ",
	"CC_AH_SF                     ",
	"CC_KWH_SF                    ",
	"CC_Batt_Voltage              ",												// 210
	"CC_Array_Voltage             ",
	"CC_Batt_Current              ",
	"CC_Array_Current             ",
	"CC_Charger_State             ",
	"CC_Watts                     ",
	"CC_Todays_Min_Battery_Volts  ",
	"CC_Todays_Max_Battery_Volts  ",
	"CC_VOC                       ",
	"CC_Todays_Peak_VOC           ",
	"CC_Todays_kWH                ",												// 220
	"CC_Todays_AH                 ",
	"CC_Lifetime_kWH_Hours        ",
	"CC_Lifetime_kAmp_Hours       ",
	"CC_Lifetime_Max_Watts        ",
	"CC_Lifetime_Max_Battery_Volts",
	"CC_Lifetime_Max_VOC          ",
	"CC_Temp_SF                   ",
	"CC_Temp_Output_FETs          ",
	"CC_Temp_Enclosure            ",												// 229
	"CCconfig_SunSpec_DID                     ",									// 230   
	"CCconfig_SunSpec_Length                  ",                  
	"CCconfig_port_number                     ",                     
	"CCconfig_Voltage_SF                      ",                      
	"CCconfig_Current_SF                      ",                      
	"CCconfig_Hours_SF                        ",                        
	"CCconfig_Power_SF                        ",                        
	"CCconfig_AH_SF                           ",                         
	"CCconfig_KWH_SF                          ",                          
	"CCconfig_Faults                          ",                          
	"CCconfig_Absorb_Volts                    ",									// 240                    
	"CCconfig_Absorb_Time_Hours               ",               
	"CCconfig_Absorb_End_Amps                 ",                 
	"CCconfig_Rebulk_Volts                    ",                    
	"CCconfig_Float_Volts                     ",                     
	"CCconfig_Bulk_Current                    ",                    
	"CCconfig_EQ_Volts                        ",                        
	"CCconfig_EQ_Time_Hours                   ",                  
	"CCconfig_Auto_EQ_Days                    ",                    
	"CCconfig_MPPT_Mode                       ",                       
	"CCconfig_Sweep_Width                     ",									// 250                     
	"CCconfig_Sweep_Max_Percentage            ",            
	"CCconfig_U_Pick_PWM_Duty_Cycle           ",           
	"CCconfig_Grid_Tie_Mode                   ",                   
	"CCconfig_Temp_Comp_Mode                  ",                  
	"CCconfig_Temp_Comp_Lower_Limit_Volts     ",     
	"CCconfig_Temp_Comp_Upper_Limit_Volts     ",     
	"CCconfig_Temp_Comp_Slope                 ",              
	"CCconfig_Auto_Restart_Mode               ",               
	"CCconfig_Wakeup_VOC                      ",                      
	"CCconfig_Snooze_Mode_Amps                ",									// 260                
	"CCconfig_Wakeup_Interval                 ",                 
	"CCconfig_AUX_Mode                        ",                        
	"CCconfig_AUX_Control                     ",                       
	"CCconfig_AUX_State                       ",                       
	"CCconfig_AUX_Polarity                    ",                    
	"CCconfig_AUX_Low_Batt_Disconnect         ",         
	"CCconfig_AUX_Low_Batt_Reconnect          ",   
	"CCconfig_AUX_Low_Batt_Disconnect_Delay   ",   
	"CCconfig_AUX_Vent_Fan_Volts              ",              
	"CCconfig_AUX_PV_Limit_Volts              ",									// 270              
	"CCconfig_AUX_PV_Limit_Hold_Time          ",          
	"CCconfig_AUX_Night_Light_Thres_Volts     ",     
	"CCconfig_Night_Light_ON_Hours            ",            
	"CCconfig_Night_Light_ON_Hyst_Time        ",        
	"CCconfig_Night_Light_OFF_Hyst_Time       ",       
	"CCconfig_AUX_Error_Battery_Volts         ",         
	"CCconfig_AUX_Divert_Hold_Time            ",        
	"CCconfig_AUX_Divert_Delay_Time           ",           
	"CCconfig_AUX_Divert_Relative_Volts       ",       
	"CCconfig_AUX_Divert_Hyst_Volts           ",                                    // 280
	"CCconfig_Major_Firmware_Number           ",         
	"CCconfig_Mid_Firmware_Number             ",             
	"CCconfig_Minor_Firmware_Number           ",           
	"CCconfig_Set_Log_Day_Offset              ",              
	"CCconfig_Get_Current_Log_Day_Offset      ",      
	"CCconfig_Log_Daily_AH                    ",                    
	"CCconfig_Log_Daily_kWH                   ",                  
	"CCconfig_Log_Daily_Max_Output_Amps       ",       
	"CCconfig_Log_Daily_Max_Output_Watts      ",      
	"CCconfig_Log_Daily_Absorb_Time           ",                                    // 290
	"CCconfig_Log_Daily_Float_Time            ",           
	"CCconfig_Log_Daily_Min_Batt_Volts        ",        
	"CCconfig_Log_Daily_Max_Batt_Volts        ",        
	"CCconfig_Log_Daily_Max_Input_Volts       ",       
	"CCconfig_Clear_Log_Read                  ",                  
	"CCconfig_Clear_Log_Write_Complement      ",      
	"CCconfig_Stats_Maximum_Reset_Read        ",       
	"CCconfig_Stats_Maximum_Write_Complement  ",   
	"CCconfig_Stats_Totals_Reset_Read         ",          
	"CCconfig_Stats_Totals_Write_Complement   ",									// 300    
	"CCconfig_Battery_Voltage_Calibrate_Offset",
	"CCconfig_Serial_Number                   ",                   
	"CCconfig_Model_Number                    ",									// 303                     	 	
	"I_SunSpec_DID    ",															// 304
	"I_SunSpec_Length ",
	"I_AC_Current     ",
	"I_AC_CurrentA    ",
	"I_AC_CurrentB    ",
	"I_AC_CurrentC    ",
	"I_AC_Current_SF  ",															// 310
	"I_AC_VoltageAB   ",
	"I_AC_VoltageBC   ",
	"I_AC_VoltageCA   ",
	"I_AC_VoltageAN   ",
	"I_AC_VoltageBN   ",
	"I_AC_VoltageCN   ",
	"I_AC_Voltage_SF  ",
	"I_AC_Power       ",
	"I_AC_Power_SF    ",
	"I_AC_Frequency   ",                                                            // 320
	"I_AC_Frequency_SF",
	"I_AC_VA          ",
	"I_AC_VA_SF       ",
	"I_AC_VAR         ",
	"I_AC_VAR_SF      ",
	"I_AC_PF          ",
	"I_AC_PF_SF       ",
	"I_AC_Energy_WH   ",
	"I_AC_Energy_WH_SF",
	"I_DC_Current     ",															// 330
	"I_DC_Current_SF  ",
	"I_DC_Voltage     ",
	"I_DC_Voltage_SF  ",
	"I_DC_Power       ",
	"I_DC_Power_SF    ",
	"I_Temp_Cab       ",
	"I_Temp_Sink      ",
	"I_Temp_Trans     ",
	"I_Temp_Other     ",
	"I_Temp_SF        ",                                                            // 340
	"I_Status         ",
	"I_Status_Vendor  ",
	"I_Event_1        ",
	"I_Event_2        ",
	"I_Event_1_Vendor ",
	"I_Event_2_Vendor ",
	"I_Event_3_Vendor ",
	"I_Event_4_Vendor ",															// 348
	"GSconfig_SunSpec_DID                     ",									// 349
	"GSconfig_SunSpec_Length                  ",									// 350
	"GSconfig_Port_Number                     ",
	"GSconfig_DC_Voltage_SF                   ",
	"GSconfig_AC_Current_SF                   ",
	"GSconfig_AC_Voltage_SF                   ",
	"GSconfig_Time_SF                         ",
	"GSconfig_Major_firmware_number           ",
	"GSconfig_Mid_firmware_number             ",
	"GSconfig_Minor_firmware_number           ",
	"GSconfig_Absorb_Volts                    ",
	"GSconfig_Absorb_Time_Hours               ",									// 360
	"GSconfig_Float_Volts                     ",
	"GSconfig_Float_Time_Hours                ",
	"GSconfig_ReFloat_Volts                   ",
	"GSconfig_EQ_Volts                        ",
	"GSconfig_EQ_Time_Hours                   ",
	"GSconfig_Search_Sensitivity              ",
	"GSconfig_Search_Pulse_Length             ",
	"GSconfig_Search_Pulse_Spacing            ",
	"GSconfig_AC_Input_Select_Priority        ",
	"GSconfig_Grid_AC_Input_Current_Limit     ",									// 370
	"GSconfig_Gen_AC_Input_Current_Limit      ",
	"GSconfig_Charger_AC_Input_Current_Limit  ",
	"GSconfig_Charger_Operating_Mode          ",
	"GSconfig_AC_Coupled                      ",
	"GSconfig_Grid_Input_Mode                 ",
	"GSconfig_Grid_Lower_Input_Voltage_Limit  ",
	"GSconfig_Grid_Upper_Input_Voltage_Limit  ",
	"GSconfig_Grid_Transfer_Delay             ",
	"GSconfig_Grid_Connect_Delay              ",
	"GSconfig_Gen_Input_Mode                  ",									// 380
	"GSconfig_Gen_Lower_Input_Voltage_Limit   ",
	"GSconfig_Gen_Upper_Input_Voltage_Limit   ",
	"GSconfig_Gen_Transfer_Delay              ",
	"GSconfig_Gen_Connect_Delay               ",
	"GSconfig_AC_Output_Voltage               ",
	"GSconfig_Low_Battery_Cut_Out_Voltage     ",
	"GSconfig_Low_Battery_Cut_In_Voltage      ",
	"GSconfig_AUX_Mode                        ",
	"GSconfig_AUX_Control                     ",
	"GSconfig_AUX_ON_Battery_Voltage          ",									// 390
	"GSconfig_AUX_ON_Delay_Time               ",
	"GSconfig_AUX_OFF_Battery_Voltage         ",
	"GSconfig_AUX_OFF_Delay_Time              ",
	"GSconfig_AUX_Relay_Mode                  ",
	"GSconfig_AUX_Relay_Control               ",
	"GSconfig_AUX_Relay_ON_Battery_Voltage    ",
	"GSconfig_AUX_Relay_ON_Delay_Time         ",
	"GSconfig_AUX_Relay_OFF_Battery_Voltage   ",
	"GSconfig_AUX_Relay_OFF_Delay_Time        ",
	"GSconfig_Stacking_Mode                   ",									// 400
	"GSconfig_Master_Power_Save_Level         ",
	"GSconfig_Slave_Power_Save_Level          ",
	"GSconfig_Sell_Volts                      ",
	"GSconfig_Grid_Tie_Window                 ",
	"GSconfig_Grid_Tie_Enable                 ",
	"GSconfig_Grid_AC_Input_Voltage_Cal_Factor",
	"GSconfig_Gen_AC_Input_Voltage_Cal_Factor ",
	"GSconfig_AC_Output_Voltage_Cal_Factor    ",
	"GSconfig_Battery_Voltage_Cal_Factor      ",
    "GSconfig_ReBulk_Volts                    ",									// 410                      	
    "GSconfig_Mini_Grid_LBX_Volts             ",               	
    "GSconfig_Mini_Grid_LBX_Delay             ",               	
    "GSconfig_Grid_Zero_DoD_Volts             ",               	
    "GSconfig_Grid_Zero_DoD_Max_Offset_AC_Amps",  	
    "GSconfig_Serial_Number                   ",
	"GSconfig_Model_Number                    ",
	"GSconfig_Module_Control                  ",	
	"GSconfig_Model_Select                    ",									// 418
	
	"GSconfig_Low_Battery_Cut_Out_Delay       ",
	"GSconfig_High_Battery_Cut_Out_Voltage    ",
	"GSconfig_High_Battery_Cut_In_Voltage     ",
	"GSconfig_High_Battery_Cut_Out_Delay      ",									// 422
	  	
	"GS_Single_SunSpec_DID              ",											// 423
	"GS_Single_SunSpec_Length           ",											// 424
	"GS_Single_Port_Number              ",
	"GS_Single_DC_Voltage_SF            ",
	"GS_Single_AC_Current_SF            ",
	"GS_Single_AC_Voltage_SF            ",
	"GS_Single_Frequency_SF             ",
	"GS_Single_Inverter_Output_Current  ",
	"GS_Single_Inverter_Charge_Current  ",
	"GS_Single_Inverter_Buy_Current     ",
	"GS_Single_Inverter_Sell_Current    ",
	"GS_Single_Grid_Input_AC_Voltage    ",											// 434
	"GS_Single_Gen_Input_AC_Voltage     ",
	"GS_Single_AC_Output_Voltage        ",
	"GS_Single_Inverter_Operating_Mode  ",
	"GS_Single_Error_Flags              ",
	"GS_Single_Warning_Flags            ",
	"GS_Single_Battery_Voltage          ",
	"GS_Single_Temp_Comp_Target_Voltage ",
	"GS_Single_AUX_Output_State         ",
	"GS_Single_AUX_Relay_Output_State   ",
	"GS_Single_L_Module_Transformer_Temp",											// 444
	"GS_Single_L_Module_Capacitor_Temp  ",
	"GS_Single_L_Module_FET_Temp        ",
	"GS_Single_R_Module_Transformer_Temp",
	"GS_Single_R_Module_Capacitor_Temp  ",
	"GS_Single_R_Module_FET_Temp        ",
	"GS_Single_Battery_Temperature      ",
	"GS_Single_AC_Input_Selection       ",
	"GS_Single_AC_Input_Frequency       ",
	"GS_Single_AC_Input_Voltage         ",
	"GS_Single_AC_Input_State           ",											// 454
	"GS_Single_Minimum_AC_Input_Voltage ",
	"GS_Single_Maximum_AC_Input_Voltage ",
	"GS_Single_Sell_Status              ",
    "GS_Single_kWh_SF                   ",
	"GS_Single_AC1_Buy_kWh              ", 
	"GS_Single_AC2_Buy_kWh              ", 
	"GS_Single_AC1_Sell_kWh             ",
	"GS_Single_AC2_Sell_kWh             ",   	
    "GS_Single_Output_kWh               ",											// 463
	"GS_Single_Charger_kWh              ", 
	"GS_Single_Output_kW                ",   
	"GS_Single_Buy_kW                   ",      
	"GS_Single_Sell_kW                  ",     
	"GS_Single_Charge_kW                ",   
	"GS_Single_Load_kW                  ",     
	"GS_Single_AC_Couple_kW             ",
	"GS_Split_SunSpec_DID               ",											// 471
	"GS_Split_SunSpec_Length            ",
	"GS_Split_Port_Number               ",
	"GS_Split_DC_Voltage_SF             ",
	"GS_Split_AC_Current_SF             ",
	"GS_Split_AC_Voltage_SF             ",
	"GS_Split_Frequency_SF              ",
	"GS_Split_L1_Inverter_Output_Current",
	"GS_Split_L1_Inverter_Charge_Current",
	"GS_Split_L1_Inverter_Buy_Current   ",
	"GS_Split_L1_Inverter_Sell_Current  ",											// 481
	"GS_Split_L1_Grid_Input_AC_Voltage  ",
	"GS_Split_L1_Gen_Input_AC_Voltage   ",
	"GS_Split_L1_AC_Output_Voltage      ",
	"GS_Split_L2_Inverter_Output_Current",
	"GS_Split_L2_Inverter_Charge_Current",
	"GS_Split_L2_Inverter_Buy_Current   ",
	"GS_Split_L2_Inverter_Sell_Current  ",
	"GS_Split_L2_Grid_Input_AC_Voltage  ",
	"GS_Split_L2_Gen_Input_AC_Voltage   ",
	"GS_Split_L2_AC_Output_Voltage      ",											// 491
	"GS_Split_Inverter_Operating_Mode   ",
	"GS_Split_Error_Flags               ",
	"GS_Split_Warning_Flags             ",
	"GS_Split_Battery_Voltage           ",
	"GS_Split_Temp_Comp_Target_Voltage  ",
	"GS_Split_AUX_Output_State          ",
	"GS_Split_AUX_Relay_Output_State    ",
	"GS_Split_L_Module_Transformer_Temp ",
	"GS_Split_L_Module_Capacitor_Temp   ",
	"GS_Split_L_Module_FET_Temp         ",											// 501
	"GS_Split_R_Module_Transformer_Temp ",
	"GS_Split_R_Module_Capacitor_Temp   ",
	"GS_Split_R_Module_FET_Temp         ",
	"GS_Split_Battery_Temperature       ",
	"GS_Split_AC_Input_Selection        ",
	"GS_Split_AC_Input_Frequency        ",
	"GS_Split_AC_Input_Voltage          ",
	"GS_Split_AC_Input_State            ",
	"GS_Split_Minimum_AC_Input_Voltage  ",
	"GS_Split_Maximum_AC_Input_Voltage  ",											// 511
	"GS_Split_Sell_Status               ",
	"GS_Split_kWh_SF                    ",
	"GS_Split_AC1_L1_Buy_kWh            ", 
	"GS_Split_AC2_L1_Buy_kWh            ", 
	"GS_Split_AC1_L1_Sell_kWh           ",
	"GS_Split_AC2_L1_Sell_kWh           ",
	"GS_Split_L1_Output_kWh             ",  
	"GS_Split_AC1_L2_Buy_kWh            ", 
	"GS_Split_AC2_L2_Buy_kWh            ", 
	"GS_Split_AC1_L2_Sell_kWh           ",											// 521
	"GS_Split_AC2_L2_Sell_kWh           ",
	"GS_Split_L2_Output_kWh             ",											// 523
	"GS_Split_Charger_kWh               ", 
	"GS_Split_Output_kW                 ",   
	"GS_Split_Buy_kW                    ",      
	"GS_Split_Sell_kW                   ",     
	"GS_Split_Charge_kW                 ",   
	"GS_Split_Load_kW                   ",     
	"GS_Split_AC_Couple_kW              ",											// 530
	"FXconfig_SunSpec_DID                       ",									// 531									
	"FXconfig_SunSpec_Length                    ",
	"FXconfig_Port_Number                       ",
	"FXconfig_DC_Voltage_SF                     ",
	"FXconfig_AC_Current_SF                     ",
	"FXconfig_AC_Voltage_SF                     ",
	"FXconfig_Time_SF                           ",
	"FXconfig_Major_Firmware_Number             ",									// 538
	"FXconfig_Mid_Firmware_Number               ",
	"FXconfig_Minor_Firmware_Number             ",
	"FXconfig_Absorb_Volts                      ",
	"FXconfig_Absorb_Time_Hours                 ",
	"FXconfig_Float_Volts                       ",
	"FXconfig_Float_Time_Hours                  ",
	"FXconfig_ReFloat_Volts                     ",
	"FXconfig_EQ_Volts                          ",
	"FXconfig_EQ_Time_Hours                     ",
	"FXconfig_Search_Sensitivity                ",									// 548
	"FXconfig_Search_Pulse_Length               ",
	"FXconfig_Search_Pulse_Spacing              ",
	"FXconfig_AC_Input_Type                     ",
	"FXconfig_Input_Support                     ",
	"FXconfig_Grid_AC_Input_Current_Limit       ",
	"FXconfig_Gen_AC_Input_Current_Limit        ",
	"FXconfig_Charger_AC_Input_Current_Limit    ",
	"FXconfig_Charger_Operating_Mode            ",
	"FXconfig_Grid_Lower_Input_Voltage_Limit    ",
	"FXconfig_Grid_Upper_Input_Voltage_Limit    ",                                  // 558
	"FXconfig_Grid_Transfer_Delay               ",
	"FXconfig_Gen_Lower_Input_Voltage_Limit     ",
	"FXconfig_Gen_Upper_Input_Voltage_Limit     ",
	"FXconfig_Gen_Transfer_Delay                ",
	"FXconfig_Gen_Connect_Delay                 ",
	"FXconfig_AC_Output_Voltage                 ",
	"FXconfig_Low_Battery_Cut_Out_Voltage       ",
	"FXconfig_Low_Battery_Cut_In_Voltage        ",
	"FXconfig_AUX_Mode                          ",
	"FXconfig_AUX_Control                       ",									// 568
	"FXconfig_AUX_Load_Shed_Enable_Voltage      ",
	"FXconfig_AUX_Gen_Alert_On_Voltage          ",
	"FXconfig_AUX_Gen_Alert_On_Delay            ",
	"FXconfig_AUX_Gen_Alert_Off_Voltage         ",
	"FXconfig_AUX_Gen_Alert_Off_Delay           ",
	"FXconfig_AUX_Vent_Fan_Enable_Voltage       ",
	"FXconfig_AUX_Vent_Fan_Off_Period           ",
	"FXconfig_AUX_Divert_Enable_Voltage         ",
	"FXconfig_AUX_Divert_Off_Delay              ",
	"FXconfig_Stacking_Mode                     ",                                  // 578
	"FXconfig_Master_Power_Save_Level           ",
	"FXconfig_Slave_Power_Save_Level            ",
	"FXconfig_Sell_Volts                        ",
	"FXconfig_Grid_Tie_Window                   ",
	"FXconfig_Grid_Tie_Enable                   ",
	"FXconfig_AC_Input_Voltage_Calibrate_Factor ",
	"FXconfig_AC_Output_Voltage_Calibrate_Factor",
	"FXconfig_Battery_Voltage_Calibrate_Factor  ",
    "FXconfig_Serial_Number                     ", 	
    "FXconfig_Model_Number                      ",									// 588  		
	"FX_SunSpec_DID                    ",											// 589
	"FX_SunSpec_Length                 ",
	"FX_Port_Number                    ",
	"FX_AC_Current_SF                  ",
	"FX_DC_Voltage_SF                  ",
	"FX_AC_Voltage_SF                  ",
	"FX_AC_Frequency_SF                ",
	"FX_Inverter_Output_Current        ",
	"FX_Inverter_Charge_Current        ",
	"FX_Inverter_Buy_Current           ",                                           // 598
	"FX_Inverter_Sell_Current          ",
	"FX_Output_AC_Voltage              ",
	"FX_Inverter_Operating_Mode        ",
	"FX_Error_Flags                    ",
	"FX_Warning_Flags                  ",
	"FX_Battery_Voltage                ",
	"FX_Temp_Compensated_Target_Voltage",
	"FX_AUX_Output_State               ",
	"FX_Transformer_Temperature        ",
	"FX_Capacitor_Temperature          ",											// 608
	"FX_FET_Temperature                ",
	"FX_AC_Input_Frequency             ",
	"FX_AC_Input_Voltage               ",
	"FX_AC_Input_State                 ",
	"FX_Minimum_AC_Input_Voltage       ",
	"FX_Maximum_AC_Input_Voltage	   ",
	"FX_Sell_Status                    ",
	"FX_kWh_SF                         ",              	
	"FX_Buy_kWh                        ",             	
	"FX_Sell_kWh                       ",											// 618            	
	"FX_Output_kWh                     ",											// 619
	"FX_Charger_kWh                    ", 
	"FX_Output_kW                      ",   
	"FX_Buy_kW                         ",      
	"FX_Sell_kW                        ",     
	"FX_Charge_kW                      ",   
	"FX_Load_kW                        ",     
	"FX_AC_Couple_kW                   ",											// 626
  	"FNconfig_SunSpec_DID                    ",										// 627 
	"FNconfig_SunSpec_Length                 ", 
	"FNconfig_Port_Number                    ", 
	"FNconfig_DC_Voltage_SF                  ", 
	"FNconfig_DC_Current_SF                  ", 
	"FNconfig_kWh_SF                         ", 
	"FNconfig_Major_Firmware_Number          ", 
	"FNconfig_Mid_Firmware_Number            ", 
	"FNconfig_Minor_Firmware_Number          ",										// 635 
	"FNconfig_Battery_Capacity               ", 
	"FNconfig_Charged_Volts                  ", 
	"FNconfig_Charged_Time                   ", 
	"FNconfig_Battery_Charged_Amps           ", 
	"FNconfig_Charge_Factor                  ", 
	"FNconfig_Shunt_A_Enabled                ", 
	"FNconfig_Shunt_B_Enabled                ", 
	"FNconfig_Shunt_C_Enabled                ", 
	"FNconfig_Relay_Control                  ", 
	"FNconfig_Relay_Invert_Logic             ",										// 645 
	"FNconfig_Relay_High_Voltage             ", 
	"FNconfig_Relay_Low_Voltage              ", 
	"FNconfig_Relay_SOC_High                 ", 
	"FNconfig_Relay_SOC_Low                  ", 
	"FNconfig_Relay_High_Enable_Delay        ", 
	"FNconfig_Relay_Low_Enable_Delay         ", 
	"FNconfig_Set_Data_Log_Day_Offset        ", 
	"FNconfig_Get_Current_Data_Log_Day_Offset",
	"FNconfig_Datalog_Minimum_SOC            ", 
	"FNconfig_Datalog_Input_AH               ",										// 655 
	"FNconfig_Datalog_Input_kWh              ", 
	"FNconfig_Datalog_Output_AH              ", 
	"FNconfig_Datalog_Output_kWh             ", 
	"FNconfig_Datalog_NET_AH                 ", 
	"FNconfig_Datalog_NET_kWh                ", 
	"FNconfig_Clear_Data_Log_Read            ", 
	"FNconfig_Clear_Data_Log_Write_Complement", 
    "FNconfig_Serial_Number                  ", 	
    "FNconfig_Model_Number                   ",										// 664  		
	"FN_SunSpec_DID                                ",								// 665 
	"FN_SunSpec_Length                             ", 
	"FN_Port_Number                                ", 
	"FN_DC_Voltage_SF                              ", 
	"FN_DC_Current_SF                              ", 
	"FN_Time_SF                                    ", 
	"FN_kWh_SF                                     ",
	"FN_kW_SF                                      ", 
	"FN_Shunt_A_Current                            ", 
	"FN_Shunt_B_Current                            ", 
	"FN_Shunt_C_Current                            ",								// 675 
	"FN_Battery_Voltage                            ", 
	"FN_Battery_Current                            ", 
	"FN_Battery_Temperature                        ", 
	"FN_Status_Flags                               ", 
	"FN_Shunt_A_Accumulated_AH                     ", 
	"FN_Shunt_A_Accumulated_kWh                    ", 
	"FN_Shunt_B_Accumulated_AH                     ", 
	"FN_Shunt_B_Accumulated_kWh                    ", 
	"FN_Shunt_C_Accumulated_AH                     ", 
	"FN_Shunt_C_Accumulated_kWh                    ",								// 685
	"FN_Input_Current                              ",   
	"FN_Output_Current                             ",  
	"FN_Input_kW                                   ",        
	"FN_Output_kW                                  ",       
	"FN_Net_kW                                     ",           
	"FN_Days_Since_Charge_Parameters_Met           ", 
	"FN_State_Of_Charge                            ", 
	"FN_Todays_Minimum_SOC                         ", 
	"FN_Todays_Maximum_SOC                         ", 
	"FN_Todays_NET_Input_AH                        ",								// 695 
	"FN_Todays_NET_Input_kWh                       ", 
	"FN_Todays_NET_Output_AH                       ", 
	"FN_Todays_NET_Output_kWh                      ",
	"FN_Todays_NET_Battery_AH                      ",
	"FN_Todays_NET_Battery_kWh                     ",
	"FN_Charge_Factor_Corrected_NET_Battery_AH     ", 
	"FN_Charge_Factor_Corrected_NET_Battery_kWh    ", 
	"FN_Todays_Minimum_Battery_Voltage             ", 
	"FN_Todays_Minimum_Battery_Time                ", 
	"FN_Todays_Maximum_Battery_Voltage             ",								// 703
	"FN_Todays_Maximum_Battery_Time                ", 
	"FN_Cycle_Charge_Factor                        ", 
	"FN_Cycle_kWh_Charge_Efficiency                ", 
	"FN_Total_Days_At_100_Percent                  ", 
	"FN_Lifetime_kAH_Removed                       ", 
	"FN_Shunt_A_Historical_Returned_To_Battery_AH  ", 
	"FN_Shunt_A_Historical_Returned_To_Battery_kWh ", 
	"FN_Shunt_A_Historical_Removed_From_Battery_AH ", 
	"FN_Shunt_A_Historical_Removed_From_Battery_kWh", 
	"FN_Shunt_A_Maximum_Charge_Rate                ",								// 715 
	"FN_Shunt_A_Maximum_Charge_Rate_kW             ", 
	"FN_Shunt_A_Maximum_Discharge_Rate             ", 
	"FN_Shunt_A_Maximum_Discharge_Rate_kW          ", 
	"FN_Shunt_B_Historical_Returned_To_Battery_AH  ", 
	"FN_Shunt_B_Historical_Returned_To_Battery_kWh ", 
	"FN_Shunt_B_Historical_Removed_From_Battery_AH ", 
	"FN_Shunt_B_Historical_Removed_From_Battery_kWh", 
	"FN_Shunt_B_Maximum_Charge_Rate                ", 
	"FN_Shunt_B_Maximum_Charge_Rate_kW             ", 
	"FN_Shunt_B_Maximum_Discharge_Rate             ",								// 725 
	"FN_Shunt_B_Maximum_Discharge_Rate_kW          ", 
	"FN_Shunt_C_Historical_Returned_To_Battery_AH  ", 
	"FN_Shunt_C_Historical_Returned_To_Battery_kWh ", 
	"FN_Shunt_C_Historical_Removed_From_Battery_AH ", 
	"FN_Shunt_C_Historical_Removed_From_Battery_kWh", 
	"FN_Shunt_C_Maximum_Charge_Rate                ", 
	"FN_Shunt_C_Maximum_Charge_Rate_kW             ", 
	"FN_Shunt_C_Maximum_Discharge_Rate             ", 
	"FN_Shunt_C_Maximum_Discharge_Rate_kW          ", 
	"FN_Shunt_A_Reset_Maximum_Data                 ",								// 735 
	"FN_Shunt_A_Reset_Maximum_Data_Write_Complement", 
	"FN_Shunt_B_Reset_Maximum_Data                 ", 
	"FN_Shunt_B_Reset_Maximum_Data_Write_Complement", 
	"FN_Shunt_C_Reset_Maximum_Data                 ", 
	"FN_Shunt_C_Reset_Maximum_Data_Write_Complement",								// 740 
	
	"OP_stats_DID              ",													// 741        	
	"OP_stats_Length           ",             	
	"OP_stats_Bt_min           ",             	
	"OP_stats_Bt_max           ",             	
	"OP_stats_Bt_ave           ",             	
	"OP_stats_Bt_attempts      ",        	
	"OP_stats_Bt_errors        ",          	
	"OP_stats_Bt_timeouts      ",        	
	"OP_stats_Bt_packet_timeout",  	
	"OP_stats_Mp_min           ",             	
	"OP_stats_Mp_max           ",             	
	"OP_stats_Mp_ave           ",             	
	"OP_stats_Mp_attempts      ",        	
	"OP_stats_Mp_errors        ",          	
	"OP_stats_Mp_timeouts      ",        	
	"OP_stats_Mp_packet_timeout",  	
	"OP_stats_Cu_min           ",             	
	"OP_stats_Cu_max           ",             	
	"OP_stats_Cu_ave           ",             	
	"OP_stats_Cu_attempts      ",        	
	"OP_stats_Cu_errors        ",          	
	"OP_stats_Cu_timeouts      ",        	
	"OP_stats_Cu_packet_timeout",  	
	"OP_stats_Su_min           ",             	
	"OP_stats_Su_max           ",             	
	"OP_stats_Su_ave           ",             	
	"OP_stats_Su_attempts      ",        	
	"OP_stats_Su_errors        ",          	
	"OP_stats_Su_timeouts      ",        	
	"OP_stats_Su_packet_timeout",  	
	"OP_stats_Pg_min           ",             	
	"OP_stats_Pg_max           ",             	
	"OP_stats_Pg_ave           ",             	
	"OP_stats_Pg_attempts      ",        	
	"OP_stats_Pg_errors        ",          	
	"OP_stats_Pg_timeouts      ",        	
	"OP_stats_Pg_packet_timeout",  	
	"OP_stats_Mb_min           ",             	
	"OP_stats_Mb_max           ",             	
	"OP_stats_Mb_ave           ",             	
	"OP_stats_Mb_attempts      ",        	
	"OP_stats_Mb_errors        ",          	
	"OP_stats_Mb_timeouts      ",        	
	"OP_stats_Mb_packet_timeout",  	
	"OP_stats_Fu_min           ",             	
	"OP_stats_Fu_max           ",             	
	"OP_stats_Fu_ave           ",             	
	"OP_stats_Fu_attempts      ",        	
	"OP_stats_Fu_errors        ",          	
	"OP_stats_Fu_timeouts      ",        	
	"OP_stats_Fu_packet_timeout",  	
	"OP_stats_Ev_min           ",             	
	"OP_stats_Ev_max           ",             	
	"OP_stats_Ev_ave           ",             	
	"OP_stats_Ev_attempts      ",        	
	"OP_stats_Ev_errors        ",          	
	"OP_stats_Ev_timeouts      ",        	
	"OP_stats_Ev_packet_timeout", 													// 798  		
	
	"A_SunSpec_DID   ",																// 799
	"A_SunSpec_Length",
	"A_Devices       ",
	"A_Count         ",
	"A_Update_Number ",	
	"A_Status        ",
	"A_Status_Vendor ",
	"A_Event         ",
	"A_Event_Vendor  ",
	"A_Control       ",
	"A_Control_Vendor",
	"A_Control_Value ",
	"END_SunSpec_DID   ",
	"END_SunSpec_Length"															// 812 
};

modbus_t	*ctx;
uint16		deviceOffsetTable[MAX_DEVICES+1];	
uint16		deviceTable[MAX_DEVICES+1];
uint16		encKey;
uint8		encryptionEnabled;

uint32		getSunSpecID				(void);
uint16		getEncryptionKey			(void);
uint16		readSunSpecRegisterU16		(int, SunSpecField);
int16		readSunSpecRegisterS16		(int, SunSpecField);
uint32		readSunSpecRegisterU32		(int, SunSpecField);
int32		readSunSpecRegisterS32		(int, SunSpecField);
uint64		readSunSpecRegisterU64		(int, SunSpecField);
int64		readSunSpecRegisterS64		(int, SunSpecField);
int			readSunSpecString			(int, SunSpecField, char *);
uint32		readSunSpecAddress			(int, SunSpecField, char *);
int64		readSunSpecFormattedValue	(int, SunSpecField, char *);
int			writeSunSpecRegister		(int, SunSpecField, uint16);
int			writeSunSpecAddress			(int, SunSpecField, char *);
int			writeSunSpecString			(int, SunSpecField, char *);


int outbackInit(char *address, int port)
{
	int			rc;
	int			i = 0;
	int			flag = 0;
	int			addr;
	int			nb;
	int			device	= 0;
	int			offset	= 0;
	uint16		mb_data[MB_MAX_REGISTERS];
	
	memset(mb_data, 0, MB_MAX_REGISTERS * sizeof(uint16));
		
	/* TCP */
	ctx = modbus_new_tcp(address, port);
	//modbus_set_debug(ctx, TRUE);
	
	while(i < 4){
	
	if (-1 == modbus_connect(ctx))
	{	
		// Prafull
		// Added a maximum try of 4 that is a ait of 8 seconds with sleep for the code to give up
		// on reading through modbus. This generally gets successful after the first sleep at a wait
		// of 2 seconds. 
		if(flag ==  1 && i == 3)
		{	fprintf(stderr, "Connection failed: %s\n",
			modbus_strerror(errno));
			printf("modbus failed\n");
			return -1;
		}
		flag = 1;
		//Prafull added code as connection failed as soon as the next read was performed.
		sleep(1);
		//return -1;
	}
	else 
	    break;
	i++;
	}
	//Check if SunSpec
	encryptionEnabled = 0;

	if (SUNSPEC_MODBUS_MAP_ID == getSunSpecID())
	{
		if(pflag)
			printf("\nSunSpec Device Detected!\n");
	}
	else
	{
		//try decrypting
		encryptionEnabled = 1;
		encKey = getEncryptionKey();
		
		if (SUNSPEC_MODBUS_MAP_ID == getSunSpecID())
		{
			printf("\nEncrypted SunSpec Device Detected!\n");
		}
		else
		{
			printf("ERROR: SunSpec Device NOT Detected\n");
			encryptionEnabled = 0;
			
			return -2;
		}
	}
	
	//Get Device Table	
	addr = ADDR_START; // @@@ -1;
	nb = 2;

	do
	{
		if (device >= MAX_DEVICES)
		{
			break;
		}
		
		addr += (offset + 2);
		deviceOffsetTable[device] = addr;
		
//		printf("modbus_read_registers -> %d\n",addr);
		
		rc = modbus_read_registers(ctx, addr, nb, mb_data);
		
		if (rc != nb)
		{
//			printf("rc != nb, addr %d, device %d\n",addr,device);
		
			return -3;
		}
		else
		{
			if (0 != encryptionEnabled)
			{
				DECRYPT(mb_data[0]);
				DECRYPT(mb_data[1]);
			}
			
			offset = mb_data[1];
			deviceTable[device] = mb_data[0];

//			printf("%d: Addr %d DID %d Offset %d\n",device,addr,mb_data[0],mb_data[1]);
		}
	}
	while (deviceTable[device++] != SUNSPEC_END_BLOCK_DID);
	
	deviceOffsetTable[0] -= 2;					// fix common block offset
	deviceOffsetTable[device] = addr;
		
	return device;
}


void outbackClose(void)
{
	if (0 != ctx)
	{
		modbus_close(ctx);
		modbus_free(ctx);
	}
}


uint16 getBlockType(int block)
{
	return deviceTable[block];
}


uint16 getEncryptionKey(void)
{
	uint16	result;
	int		addr;
	int		nb = 1;
	
	addr = ADDR_START + COMMON_SIZE + md[OutBack_Encryption_Key].position + 1;
	modbus_read_registers(ctx, addr, nb, &result);

	return (result);
}


uint32 getSunSpecID(void)
{
	uint16	result[2];
	uint32	result32;
	int		addr;
	int		nb = 2;
	
	addr = ADDR_START + md[C_SunSpec_ID].position;
	modbus_read_registers(ctx, addr-1, nb, result);
	
	if (0 != encryptionEnabled)
	{
		DECRYPT(result[0]);
		DECRYPT(result[1]);
	}
	
	result32 = ((result[0] << 16) | result[1]);

	if(pflag)
		printf("getSunSpecID() Addr %d ID %0X\n",addr,result32);		
	
	return (result32);
}


uint16 readSunSpecRegisterU16(int block, SunSpecField field)
{
	uint16		result;
	int		addr;
	const int	nb = 1;
	
	addr = deviceOffsetTable[block] + md[field].position -1;
	modbus_read_registers(ctx, addr, nb, &result);
	
	if ((0 != encryptionEnabled) && (OutBack_Encryption_Key != field))
	{
		DECRYPT(result);
	}
	
	return (result);
}


int16 readSunSpecRegisterS16(int block, SunSpecField field)
{
	uint16		result;
	int			addr;
	const int	nb = 1;
	
	addr = deviceOffsetTable[block] + md[field].position -1;
	modbus_read_registers(ctx, addr, nb, &result);
	
	if (0 != encryptionEnabled)
	{
		DECRYPT(result);
	}
	
	return ((int16)result);
}

uint32 readSunSpecRegisterU32(int block, SunSpecField field)
{
	uint16		result[2];
	uint32		result32;
	int			addr;
	const int	nb = 2;
	
	addr = deviceOffsetTable[block] + md[field].position -1;
	modbus_read_registers(ctx, addr, nb, result);
	
	if (0 != encryptionEnabled)
	{
		DECRYPT(result[0]);
		DECRYPT(result[1]);
	}
	
	memcpy(&result32, result, nb*2);

	return (result32);
}


int32 readSunSpecRegisterS32(int block, SunSpecField field)
{
	uint16		result[2];
	int32		result32;
	int			addr;
	const int	nb = 2;
	
	addr = deviceOffsetTable[block] + md[field].position -1;
	modbus_read_registers(ctx, addr, nb, result);
	
	if (0 != encryptionEnabled)
	{
		DECRYPT(result[0]);
		DECRYPT(result[1]);
	}
	
	memcpy(&result32, result, nb*2);
	
	return (result32);
}


uint64 readSunSpecRegisterU64(int block, SunSpecField field)
{
	uint16		result[4];
	uint64		result64;
	int			addr;
	const int	nb = 4;
	
	addr = deviceOffsetTable[block] + md[field].position -1;
	modbus_read_registers(ctx, addr, nb, result);
	
	if (0 != encryptionEnabled)
	{
		DECRYPT(result[0]);
		DECRYPT(result[1]);
		DECRYPT(result[2]);
		DECRYPT(result[3]);
	}
	
	memcpy(&result64, result, nb*2);
	
	return (result64);
}


int64 readSunSpecRegisterS64(int block, SunSpecField field)
{
	uint16		result[4];
	int64		result64;
	int			addr;
	const int	nb = 4;
	
	addr = deviceOffsetTable[block] + md[field].position -1;
	modbus_read_registers(ctx, addr, nb, result);
	
	if (0 != encryptionEnabled)
	{
		DECRYPT(result[0]);
		DECRYPT(result[1]);
		DECRYPT(result[2]);
		DECRYPT(result[3]);
	}
	
	memcpy(&result64, result, nb*2);
	
	return (result64);
}


uint32 readSunSpecAddress(int block, SunSpecField field, char buff[16])
{
	uint16		result[2];
	uint32		result32;
	int			addr;
	const int	nb = 2;
	
	addr = deviceOffsetTable[block] + md[field].position -1;
	modbus_read_registers(ctx, addr, nb, result);
	
	if (0 != encryptionEnabled)
	{
		DECRYPT(result[0]);
		DECRYPT(result[1]);
	}
	
	sprintf(buff, "%u.%u.%u.%u",
			result[0] >> 8, result[0] & 0xff,
			result[1] >> 8, result[1] & 0xff);
	
	memcpy(&result32, result, nb*2);
	
	return result32;
}


int readSunSpecString(int block, SunSpecField field, char buff[64])
{
	uint16	result[32];
	int	rc, addr, nb, i;
	char	*buff_ptr, *result_ptr;
	
	addr = deviceOffsetTable[block] + md[field].position -1;
	nb = md[field].size;
	rc = modbus_read_registers(ctx, addr, nb, result);
	
	if ( (0 != encryptionEnabled) && (result[0] != 0 ))
	{
		for (i = 0; i < nb; i++)
		{
			DECRYPT(result[i]);
		}
	}

	buff_ptr = buff;
	result_ptr = (char *) result;

	for (i = nb; 0 != i; --i)
	{
		*buff_ptr++ = *(result_ptr+1);
		*buff_ptr++ = *(result_ptr+0);
		result_ptr += 2;
	}
		
	return rc * 2;
}


int64 readSunSpecFormattedValue(int block, SunSpecField field, char buff[16])
{
	int64	result;
	uint16	sf;
	char	units[8];
	//printf("reading value sunspec formatted \n");
	//get value
	switch (md[field].type)
	{
		//printf("Prafull md field type a");
		case ACC16_T:
			result = readSunSpecRegisterU16(block, field);
			//printf("Prafull md field type ACC16_T");
			
			if (NI_ACC16 == (result & 0xFFFF))
			{
				sprintf(buff, "Not implemented");
				return NI_INT64;
			}
			break;
			
		case INT16_T:
			//printf("Prafull md field type INT16_T");
			result = (int64) readSunSpecRegisterS16(block, field);
			
			if (NI_INT16 == (result & 0xFFFF))
			{
				sprintf(buff, "Not implemented");
				return NI_INT64;
			}
			break;
			
		case UINT16_T:
			//printf("Prafull md field type UINT16_T");
			result = readSunSpecRegisterU16(block, field);
			
			if ((NI_UINT16 == (result & 0xFFFF)) && (END_SunSpec_DID != field))
			{
				sprintf(buff, "Not implemented");
				return NI_INT64;
			}
			
			break;
			
		case ACC32_T:
			//printf("Prafull md field type ACC32_T");
			result = readSunSpecRegisterU32(block, field);
			
			if (NI_ACC32 == (result & 0xFFFFFFFF))
			{
				sprintf(buff, "Not implemented");
				return NI_INT64;
			}
			break;
			
		case INT32_T:
			//printf("Prafull md field type INT32_T");
			result = readSunSpecRegisterS32(block, field);
			
			if (NI_INT32 == (result & 0xFFFFFFFF))
			{
				sprintf(buff, "Not implemented");
				return NI_INT64;
			}
			break;
			
		case UINT32_T:
			//printf("Prafull md field type UINT32_T");
			result = readSunSpecRegisterU32(block, field);
			
			if (NI_UINT32 == (result & 0xFFFFFFFF))
			{
				sprintf(buff, "Not implemented");
				return NI_INT64;
			}
			break;
			
		case ACC64_T:
			//printf("Prafull md field type ACC64_T");
			result = readSunSpecRegisterU64(block, field);
			
			if (NI_ACC64 == result)
			{
				sprintf(buff, "Not implemented");
				return NI_INT64;
			}
			break;
			
		case INT64_T:
			//printf("Prafull md field type INT64_T");
			result = readSunSpecRegisterS64(block, field);
			
			if (NI_INT64 == result)
			{
				sprintf(buff, "Not implemented");
				return NI_INT64;
			}
			break;
			
		case UINT64_T:
			//printf("Prafull md field type UINT64_T");
			result = readSunSpecRegisterU64(block, field);
			
			if (NI_UINT64 == result)
			{
				sprintf(buff, "Not implemented");
				return NI_INT64;
			}
			break;
			
		default:
			sprintf(buff, "Unknown Type");
			return NI_INT64;
			break;
	}
	
	//get units
	memset(units, 0, sizeof(units));
	
	switch (md[field].units)
	{
		case BITFIELD_U:
			strcpy(units, "b");
			break;
			
		case PERCENTAGE_U:
			strcpy(units, "%");
			break;
			
		case VOLTS_U:
			strcpy(units, "V");
			break;
			
		case AMPS_U:
			strcpy(units, "A");
			break;
			
		case WATTS_U:
			strcpy(units, "W");
			break;
			
		case AH_U:
			strcpy(units, "Ah");
			break;

		case KAH_U:
			strcpy(units, "kAh");
			break;
			
		case WH_U:
			strcpy(units, "Wh");
			break;
			
		case KWH_U:
			strcpy(units, "kWh");
			break;

		case VA_U:
		case VAR_U:			
		case KW_U:
			strcpy(units, "kW");
			break;
			
		case DEGREES_C_U:
			strcpy(units, "deg C");
			break;
			
		case HERTZ_U:
			strcpy(units, "Hz");
			break;
			
		case SECS_U:
			strcpy(units, "secs");
			break;
			
		case MINS_U:
			strcpy(units, "mins");
			break;
			
		case HOURS_U:
			strcpy(units, "hours");
			break;
			
		case DAYS_U:
			strcpy(units, "days");
			break;
			
		case CYCLES_U:
			strcpy(units, "cycles");
			break;
			
		case TIME_U:
			break;
			
		default:
			strcpy(units, " ");
			break;
	}
	
	
	if (NI_SF == (uint16) md[field].scaleFactor)
	{
		sf = NI_SF;
	}
	else
	{
		sf = readSunSpecRegisterS16(block, md[field].scaleFactor);
	}
	
	switch (sf)
	{
		case 0xfffd: // -3
			if (result >= 0)
			{
				sprintf(buff, "%lld.%03lld", result / 1000, result % 1000);
			}
			else if ((result >= -999) && (result <= -1))
			{
				sprintf(buff, "-0.%03d", abs((int) result) % 1000);
 			}
			else
			{
				sprintf(buff, "%lld.%03d", result / 1000, abs((int) result) % 1000);
			}			
			break;
			
		case 0xfffe: // -2
			if (result >= 0)
			{
				sprintf(buff, "%lld.%02lld", result / 100, result % 100);
			}
			else if ((result >= -99) && (result <= -1))
			{
				sprintf(buff, "-0.%02d", abs((int) result) % 100);
			}
			else
			{
				sprintf(buff, "%lld.%02d", result / 100, abs((int) result) % 100);
			}			
			break;
			
		case 0xffff: // -1
			if (result >= 0)
			{
				sprintf(buff, "%lld.%lld", result / 10, result % 10);
			}
			else if ((result >= -9) && (result <= -1))
			{
				sprintf(buff, "-0.%d", abs((int) result) % 10);
			}
			else
			{
				sprintf(buff, "%lld.%d", result / 10, abs((int) result) % 10);
			}
			break;
			
		case 0:
			sprintf(buff, "%lld", result);
			break;
			
		case 1:
			sprintf(buff, "%lld", result * 10);
			break;
			
		case 2:
			sprintf(buff, "%lld", result * 100);
			break;
			
		case 3:
			sprintf(buff, "%lld", result * 1000);
			break;
			
		case NI_SF:
			if (BITFIELD_U == md[field].units)
			{
				if (UINT16_T == md[field].type)
				     sprintf(buff, "0x%04llX", result);
				else sprintf(buff, "0x%08llX", result);
			}
			else
			{
				sprintf(buff, "%lld", result);
			}
			
			break;
			
		default:
			sprintf(buff, "%lld", result);
			break;
	}

	/*
	switch (sf)
	{
		case 0xfffd: // -3
			if (result >= 0)
			{
				sprintf(buff, "%lld.%03lld %s", result / 1000, result % 1000, units);
			}
			else if ((result >= -999) && (result <= -1))
			{
				sprintf(buff, "-0.%03d %s", abs((int) result) % 1000, units);
 			}
			else
			{
				sprintf(buff, "%lld.%03d %s", result / 1000, abs((int) result) % 1000, units);
			}			
			break;
			
		case 0xfffe: // -2
			if (result >= 0)
			{
				sprintf(buff, "%lld.%02lld %s", result / 100, result % 100, units);
			}
			else if ((result >= -99) && (result <= -1))
			{
				sprintf(buff, "-0.%02d %s", abs((int) result) % 100, units);
			}
			else
			{
				sprintf(buff, "%lld.%02d %s", result / 100, abs((int) result) % 100, units);
			}			
			break;
			
		case 0xffff: // -1
			if (result >= 0)
			{
				sprintf(buff, "%lld.%lld %s", result / 10, result % 10, units);
			}
			else if ((result >= -9) && (result <= -1))
			{
				sprintf(buff, "-0.%d %s", abs((int) result) % 10, units);
			}
			else
			{
				sprintf(buff, "%lld.%d %s", result / 10, abs((int) result) % 10, units);
			}
			break;
			
		case 0:
			sprintf(buff, "%lld %s", result, units);
			break;
			
		case 1:
			sprintf(buff, "%lld %s", result * 10, units);
			break;
			
		case 2:
			sprintf(buff, "%lld %s", result * 100, units);
			break;
			
		case 3:
			sprintf(buff, "%lld %s", result * 1000, units);
			break;
			
		case NI_SF:
			if (BITFIELD_U == md[field].units)
			{
				if (UINT16_T == md[field].type)
				     sprintf(buff, "0x%04llX %s", result, units);
				else sprintf(buff, "0x%08llX %s", result, units);
			}
			else
			{
				sprintf(buff, "%lld %s", result, units);
			}
			
			break;
			
		default:
			sprintf(buff, "%lld", result);
			break;
	}
	*/
	
	return result;
}


int writeSunSpecRegister(int block, SunSpecField field, uint16 value)
{
	uint16		result;
	int			rc;
	int			addr;
	const int	nb = 1;
	
	addr = deviceOffsetTable[block] + md[field].position -1;
		
	if (0 != encryptionEnabled)
	{
		ENCRYPT(value);
	}
	
	rc = modbus_write_register(ctx, addr, value);
	
	if (-1 == rc)
	{
		fprintf(stderr, "Write Register failed: %s\n", modbus_strerror(errno));
    }
	else if (rc == nb)
	{
		if (md[field].read)			// if not read only
		{
			rc = modbus_read_registers(ctx, addr, nb, &result);
			
			if (result != value)
			{
				rc = -1;
			}
		}
//		else
//		{
//			rc = -2;			// read only field
//		}
	}
	
	return rc;
}


int writeSunSpecAddress(int block, SunSpecField field, char *buff)
{
	uint32	address;
	uint16	value[2];
	int	rc;
	int addr;
	const int nb = 2;
	
	address = inet_addr(buff);
	
	if (INADDR_NONE != address)
	{
    	address = BSwap32(address);
				
		value[0] = address >> 16;
		value[1] = address & 0xffff;

		if (0 != encryptionEnabled)
		{
			ENCRYPT(value[0]);
			ENCRYPT(value[1]);
		}
		
		addr = deviceOffsetTable[block] + md[field].position -1;
		
		rc = modbus_write_registers(ctx, addr, nb, value);

		if (-1 == rc)
		{
			fprintf(stderr, "[writeSunSpecAddress] Write Registers failed: %s\n", modbus_strerror(errno));
    	}
		else if (rc == nb)
		{
			uint16 check[2];
			
			rc = modbus_read_registers(ctx, addr, nb, check);			
				
			if ((nb != rc) || (memcmp(&check, &value, 2 * nb) != 0))
			{
				rc = -1;
			}
		}
	}
	else
	{
		rc = -1;
	}
	
	return rc;
}


int writeSunSpecString(int block, SunSpecField field, char *buff)
{
	uint16	value[32];
	char *value_ptr;
	int	rc, addr, nb, i;
	int clear = 0;
	
	if (STRING_T == md[field].type)
	{
		char *buff_ptr;

		addr = deviceOffsetTable[block] + md[field].position -1;
		nb = md[field].size;
		
		memset(value, 0, 64);
		
		if (0 != strcmp("NULL", buff))
		{
			buff_ptr = buff;
			value_ptr = (char *) value;

			for (i = nb; 0 != i; --i)
			{
				*value_ptr++ = *(buff_ptr+1);
				*value_ptr++ = *(buff_ptr+0);
				buff_ptr += 2;
			}
		}
		else clear = 1;
		
		if (0 != encryptionEnabled)
		{
			for (i = 0; i < nb; i++)
			{
				ENCRYPT(value[i]);
			}
		}
		
		rc = modbus_write_registers(ctx, addr, nb, value);

		if (-1 == rc)
		{
			fprintf(stderr, "[writeSunSpecString] Write Registers failed: %s\n", modbus_strerror(errno));
    	}
	}
	else
	{
		nb = 0;
		rc = -1;
	}
	
	if (rc == nb)
	{
		uint16 check_buff[32];
		char *check_ptr;
		
		if (md[field].read && (0 == clear))			// if not read only and NOT NULL string
		{
			rc = modbus_read_registers(ctx, addr, nb, value);

			check_ptr = (char *) check_buff;
			value_ptr = (char *) value;

			for (i = rc; 0 != i; --i)
			{
				*check_ptr++ = *(value_ptr+1);
				*check_ptr++ = *(value_ptr+0);
				value_ptr += 2;
			}

			if ((rc != nb) || (memcmp(check_buff, buff, 2 * nb) != 0))
			{
				rc = -1;
			}
		}
	}
	
	return (rc * 2);
}


int fieldIsInBlock(SunSpecField field, int block)
{	
	if (field >= firstField(block) && field <= lastField(block))
	{
		return 1;
	}
	else
	{
		return 0;
	}
}


SunSpecField firstField(int block)
{
	SunSpecField field;
	
	switch (deviceTable[block])
	{
		case SUNSPEC_COMMON_MODEL_BLOCK_DID:
			field = COMMON_MDOFFSET;
			break;
			
		case SUNSPEC_OUTBACK_DID:
			field = OUTBACK_MDOFFSET;
			break;
			
		case SUNSPEC_OUTBACK_SYS_CONTROL_DID:
			field = OB_SYS_CTRL_MDOFFSET;
			break;		
			
		case SUNSPEC_BASIC_CC_DID:
			field = CC_MDOFFSET;
			break;
			
		case SUNSPEC_OUTBACK_FM_CC_DID:
			field = CC_CONFIG_MDOFFSET;
			break;
			
		case SUNSPEC_INVERTER_SINGLE_DID:
		case SUNSPEC_INVERTER_SPLIT_DID:
		case SUNSPEC_INVERTER_3PHASE_DID:
			field = I_MDOFFSET;
			break;

		case SUNSPEC_OUTBACK_GS_CONFIG_DID:
			field = GS_CONFIG_MDOFFSET;
			break;
		
		case SUNSPEC_OUTBACK_GS_SINGLE_DID:
			field = GS_SINGLE_MDOFFSET;
			break;
		
		case SUNSPEC_OUTBACK_GS_SPLIT_DID:
			field = GS_SPLIT_MDOFFSET;
			break;
			
		case SUNSPEC_OUTBACK_FX_CONFIG_DID:
 			field = FX_CONFIG_MDOFFSET;
			break;
			
		case SUNSPEC_OUTBACK_FX_DID:
 			field = FX_MDOFFSET;
			break;
			
		case SUNSPEC_OUTBACK_FNDC_DID:
			field = FNDC_MDOFFSET;
			break;
			
		case SUNSPEC_OUTBACK_FNDC_CONFIG_DID:
			field = FNDC_CONFIG_MDOFFSET;
			break;

		case SUNSPEC_OUTBACK_STATISTICS_DID:
			field = OB_STATS_MDOFFSET;
			break;
			
		case SUNSPEC_END_BLOCK_DID:
			field = END_MDOFFSET;
			break;
			
		default:
			field = 0;
			break;
	}
	
	return field;
}


SunSpecField lastField(int block)
{
	SunSpecField field;
	
	switch (deviceTable[block])
	{
		case SUNSPEC_COMMON_MODEL_BLOCK_DID:
			field = OUTBACK_MDOFFSET - 1;
			break;
			
		case SUNSPEC_OUTBACK_DID:
			field = OB_SYS_CTRL_MDOFFSET - 1;
			break;
			
		case SUNSPEC_OUTBACK_SYS_CONTROL_DID:
			field = CC_MDOFFSET - 1;
			break;
					
		case SUNSPEC_BASIC_CC_DID:
			field = CC_CONFIG_MDOFFSET - 1;
			break;
			
		case SUNSPEC_OUTBACK_FM_CC_DID:
			field = I_MDOFFSET - 1;
			break;
			
		case SUNSPEC_INVERTER_SINGLE_DID:
		case SUNSPEC_INVERTER_SPLIT_DID:
		case SUNSPEC_INVERTER_3PHASE_DID:
			field = GS_CONFIG_MDOFFSET - 1;
			break;

		case SUNSPEC_OUTBACK_GS_CONFIG_DID:
			field = GS_SINGLE_MDOFFSET - 1;
			break;
		
		case SUNSPEC_OUTBACK_GS_SINGLE_DID:
			field = GS_SPLIT_MDOFFSET - 1;
			break;
		
		case SUNSPEC_OUTBACK_GS_SPLIT_DID:
			field = FX_CONFIG_MDOFFSET - 1;
			break;
			
		case SUNSPEC_OUTBACK_FX_CONFIG_DID:
 			field = FX_MDOFFSET - 1;
			break;
			
		case SUNSPEC_OUTBACK_FX_DID:
 			field = FNDC_CONFIG_MDOFFSET - 1;
			break;
			
		case SUNSPEC_OUTBACK_FNDC_CONFIG_DID:
 			field = FNDC_MDOFFSET - 1;
			break;
			
		case SUNSPEC_OUTBACK_FNDC_DID:
		 	field = OB_STATS_MDOFFSET - 1;
			break;

		case SUNSPEC_OUTBACK_STATISTICS_DID:
			field = AGGREGATOR_MDOFFSET - 1;
			break;
			
		case SUNSPEC_END_BLOCK_DID:
			field = END_MDOFFSET + 1;
			break;
			
		default:
			field = 0;
			break;
	}
	
	return field;
}


uint64 getField(int block, SunSpecField field, char *buff)
{
	uint64 value = 0;
	//printf("Prafull start getField\n");
	switch (md[field].type)
	{
		case UINT16_T:
		{	
			//printf(" Prafull UINT16_T\n");
			if (ENUMERATED_U == md[field].units)
			{
				value = readSunSpecRegisterU16(block, field);
                                printf("value is %04x", value);
				
				switch (field)
				{
					case FNconfig_Shunt_A_Enabled:
					case FNconfig_Shunt_B_Enabled:
					case FNconfig_Shunt_C_Enabled:
					case FNconfig_Relay_Invert_Logic:
						value = (1 == value) ? 0 : 1;

					case OutBack_Enable_DHCP:
					case OutBack_Alarm_Email_Enable:
					case OutBack_Enable_Time_Server:
					case OutBack_Enable_Float_Coordination:
					case OutBack_Enable_FNDC_Charge_Termination:
					case OutBack_Enable_FNDC_Grid_Tie_Control:
					case CCconfig_Grid_Tie_Mode:
					case GSconfig_Grid_Tie_Enable:
					case GSconfig_AC_Coupled:
					case GS_Single_AUX_Output_State:
					case GS_Single_AUX_Relay_Output_State:
					case GS_Split_AUX_Output_State:
					case GS_Split_AUX_Relay_Output_State:
					case FX_AUX_Output_State:
					case FXconfig_Input_Support:
					case FXconfig_Grid_Tie_Enable:
					case OutBack_AGS_Mode:
					case OutBack_Generator_Exercise_Mode:
					case OutBack_AGS_Sell_Mode:
					case OutBack_AGS_2_Min_Start_Mode:
					case OutBack_AGS_2_Hour_Start_Mode:
					case OutBack_AGS_24_Hour_Start_Mode:
					case OutBack_AGS_Enable_Full_Charge_Mode:
					case OutBack_AGS_SOC_Start_Mode:
					case OutBack_AGS_Load_Start_Mode:
					case OutBack_AGS_Must_Run_Mode:
					case OutBack_AGS_Quiet_Time_Mode:
					case OutBack_Grid_Use_Interval_1_Mode:
					case OutBack_Grid_Use_Interval_2_Mode:
					case OutBack_Grid_Use_Interval_3_Mode:
					case OutBack_Load_Grid_Transfer_Mode:
					case OutBack_Global_Charger_Control_Mode:
					case OutBack_Radian_AC_Coupled_Mode:
					{
						if (NI_UINT16 == (value & 0xFFFF))
						{
							printf("1\n");
							sprintf(buff, "Not Implemented");
						}
						else if (2 <= value)
						{
							sprintf(buff, "!%u", (uint16) value);
						}
						else
						{
							sprintf(buff, "%s", disabled_enabled_text[value]);
						}
						
						break;
					}
					
					case OutBack_SD_Card_Logging_Mode:
					{
						if (0 == value)
						{
							sprintf(buff, "%s", "Disabled");
						}
						else if (1 == value)
						{
							sprintf(buff, "%s", "Excel Format");
						}
						else if (2 == value)
						{
							sprintf(buff, "%s", "Compact Format");
						}
						else
						{
							sprintf(buff, "!%u", (uint16)value);
						}

						break;
					}
					
					case OutBack_HUB_Type:
					{
						if (NI_UINT16 == (value & 0xFFFF))
						{
							//printf("2 Prafull\n");
							sprintf(buff, "Not Implemented");
						}
                        else if (0 == value)
						{
							sprintf(buff, "%s", "Legacy Hub");
						}
						else if (4 == value)
						{
							sprintf(buff, "%s", "HUB4");
						}
						else if (10 == value)
						{
							sprintf(buff, "%s", "HUB10.3");
						}
						else if (11 == value)
						{
							sprintf(buff, "%s", "HUB3PH");
						}
						else
						{
							sprintf(buff, "!%u", (uint16) value);
						}
						
						break;
					}
					
					case OutBack_SD_Card_Log_Write_Interval:
					{
						if (value >= 11)
						{
							sprintf(buff, "!%u", (uint16) value);
						}
						else
						{
							sprintf(buff, "%u secs",SD_card_logging_interval_table[value]);
						}
						
						break;
					}
 					
					case OutBack_Gateway_Type:
					{
						if (1 == value)
						{
							sprintf(buff, "%s", "AXS Port");
						}
						else if (2 == value)
						{
							sprintf(buff, "%s", "MATE3");
						}
						else
						{
							sprintf(buff, "!%u", (uint16)value);
						}
					
						break;
					}
					
					case OutBack_AGS_Port_Type:
					{
						if (0 == value)
						{
							sprintf(buff, "%s", "AUX Output");
						}
						else if (1 == value)
						{
							sprintf(buff, "%s", "AUX Relay");
						}
						else
						{
							sprintf(buff, "!%u", (uint16)value);
						}

						break;
					}
										
					case OutBack_Generator_Type:
					{
						if (0 == value)
						{
							sprintf(buff, "%s", "AC Gen");
						}
						else if (1 == value)
						{
							sprintf(buff, "%s", "DC Gen");
						}
						else if (2 == value)
						{
							sprintf(buff, "%s", "No Gen");
						}
						else
						{
							sprintf(buff, "!%u", (uint16)value);
						}

						break;
					}
										
					case  OB_Set_AGS_OP_Mode:
					{					
						if (3 <= value)
						{
							sprintf(buff, "!%u", (uint16) value);
						}
						else
						{
							strcpy(buff, CC_AUX_op_Modes[value]);
						}
					
						break;
					}
					
					case OB_AGS_Operational_State:
					{
						if (6 <= value)
						{
							sprintf(buff, "!%u", (uint16)value);
						}
						else
						{
							strcpy(buff, AGS_op_states[value]);
						}

						break;
					}
					
					case OutBack_Exercise_Day:
					{					
						if (7 <= value)
						{
							sprintf(buff, "!%u", (uint16)value);
						}
						else
						{
							strcpy(buff, Day_of_week_text[value]);
						}

						break;
					}
					
					case OutBack_HBX_Mode:
					{
						if (4 <= value)
						{
							sprintf(buff, "!%u", (uint16)value);
						}
						else
						{
							strcpy(buff, HBX_op_modes[value]);
						}
										
						break;
					}
											
					case CC_Charger_State:
					{
						if (5 <= value)
						{
							sprintf(buff, "!%u", (uint16)value);
						}
						else
						{
							strcpy(buff, CC_charge_mode[value]);
						}
						
						break;
					}
						
					case CCconfig_MPPT_Mode:
					{
						if (3 <= value)
						{
							sprintf(buff, "!%u", (uint16)value);
						}
						else
						{
							strcpy(buff, CC_mppt_mode[value]);
						}
						
						break;
					}
						
					case CCconfig_Sweep_Width:
					{
						if (2 <= value)
						{
							sprintf(buff, "!%u", (uint16)value);
						}
						else
						{
							strcpy(buff, CC_sweep_width_modes[value]);
						}
						
						break;
					}
						
					case CCconfig_Sweep_Max_Percentage:
					{
						if (4 <= value)
						{
							sprintf(buff, "!%u", (uint16) value);
						}
						else
						{
							sprintf(buff, "%u %%", CC_sweep_max[value]);
						}
						
						break;
					}
						
					case CCconfig_Temp_Comp_Mode:
					{
						if (2 <= value)
						{
							sprintf(buff, "!%u", (uint16)value);
						}
						else
						{
							strcpy(buff, CC_temp_comp_modes[value]);
						}
						
						break;
					}
						
					case CCconfig_Auto_Restart_Mode:
					{
						if (3 <= value)
						{
							sprintf(buff, "!%u", (uint16)value);
						}
						else
						{
							strcpy(buff, CC_auto_restart_modes[value]);
						}
						
						break;
					}
						
					case CCconfig_AUX_Mode:
					{
						if (9 <= value)
						{
							sprintf(buff, "!%u", (uint16)value);
						}
						else
						{
							strcpy(buff, CC_aux_modes[value]);
						}
						
						break;
					}
						
					case CCconfig_AUX_Control:
					case GSconfig_AUX_Control:
					case GSconfig_AUX_Relay_Control:
					case FXconfig_AUX_Control:
					case FNconfig_Relay_Control:
					{
						if (NI_UINT16 == (value & 0xFFFF))
						{
							//printf("3 Prafull\n");
							sprintf(buff, "Not Implemented");
						}
                        else if (3 <= value)
						{
							sprintf(buff, "!%u", (uint16) value);
						}
						else
						{
							strcpy(buff, CC_aux_cntl[value]);
						}
						
						break;
					}
						
					case CCconfig_AUX_Polarity:
					{
						if (2 <= value)
						{
							sprintf(buff, "!%u", (uint16) value);
						}
						else
						{
							strcpy(buff, CC_aux_polarity[value]);
						}
						
						break;
					}
						
					case I_Status:
					{
						if (8 <= value)
						{
							sprintf(buff, "!%u", (uint16)value);
						}
						else
						{
							strcpy(buff, I_status_mode[value]);
						}
						
						break;
					}
						
					case I_Status_Vendor:
					{
						if (16 <= value)
						{
							sprintf(buff, "!%u", (uint16) value);
						}
						else
						{
							strcpy(buff, I_status_vendor[value]);
						}
						
						break;
					}
																	
					case GSconfig_AC_Input_Select_Priority:
					case GS_Split_AC_Input_Selection:
					case GS_Single_AC_Input_Selection:
					case FXconfig_AC_Input_Type:
					{
						if (2 <= value)
						{
							sprintf(buff, "!%u", (uint16) value);
						}
						else
						{
							strcpy(buff, AC_input_select_text[value]);
						}
						
						break;
					}
						
					case GSconfig_Charger_Operating_Mode:
					{
						if (2 <= value)
						{
							sprintf(buff, "!%u", (uint16) value);
						}
						else
						{
							strcpy(buff, Inv_charger_op_mode_text[value]);
						}
						
						break;
					}

					case FXconfig_Charger_Operating_Mode:
					{
						if (3 <= value)
						{
							sprintf(buff, "!%u", (uint16) value);
						}
						else
						{
							strcpy(buff, FX_charger_ctrl_mode_text[value]);
						}
						
						break;
					}
						
					case FX_Inverter_Operating_Mode:
					case GS_Single_Inverter_Operating_Mode:
					case GS_Split_Inverter_Operating_Mode:
					{
						if (16 <= value)
						{
							sprintf(buff, "!%u", (uint16) value);
						}
						else
						{
							strcpy(buff, FX_mode[value]);
						}
						
						break;
					}					
												
					case GSconfig_Grid_Input_Mode:
					case GSconfig_Gen_Input_Mode:
					{
						if (6 <= value)
						{
							sprintf(buff, "!%u", (uint16) value);
						}
						else
						{
							strcpy(buff, GS_AC_in_mode_text[value]);
						}
						
						break;
					}
						
					case GSconfig_AUX_Mode:
					case GSconfig_AUX_Relay_Mode:
					{
						if (NI_UINT16 == (value & 0xFFFF))
						{
							//printf("4 Prafull\n");
							sprintf(buff, "Not Implemented");
						}
						else if ((0 == value) || (11 <= value) )
						{
							sprintf(buff, "!%u", (uint16) value);
						}
						else
						{
							strcpy(buff, GS_AUX_Relay_Modes_text[value-1]);
						}
						
						break;
					}
						
					case FXconfig_AUX_Mode:
					{
						if (value > 8)
						{
							sprintf(buff, "!%u", (uint16) value);
						}
						else
						{
							strcpy(buff, FX_AUX_Modes[value]);
						}

						break;
					}										
						
					case GSconfig_Stacking_Mode:
					{
						if ( (value < 10) || (18 < value) )
						{
							sprintf(buff, "!%u", (uint16) value);
						}
						else
						{
							strcpy(buff, GS_stacking_modes_text[value-10]);
						}
						
						break;
					}
						
					case FXconfig_Stacking_Mode:
					{
						if (value > 19)
						{
							sprintf(buff, "!%u", (uint16) value);
						}
						else
						{
							strcpy(buff, FX_stacking_modes[value]);
						}

						break;
					}						
						
					case GSconfig_Grid_Tie_Window:
					case FXconfig_Grid_Tie_Window:
					{
						if (2 <= value)
						{
							if (NI_UINT16 == (value & 0xFFFF))
							{
								//printf("5 Prafull\n");
								sprintf(buff, "Not Implemented");
							}
							else
							{
								sprintf(buff, "!%u", (uint16) value);
							}
						}
						else
						{
							strcpy(buff, Grid_tie_window_text[value]);
						}
						
						break;
					}

					case GS_Single_AC_Input_State:
					case GS_Split_AC_Input_State:
					case FX_AC_Input_State:
					{
						if (2 <= value)
						{
							sprintf(buff, "!%u", (uint16) value);
						}
						else
						{
							strcpy(buff, AC_input_mode_text[value]);
						}
						
						break;
					}
					
					case GSconfig_Module_Control:
					{
						if (NI_UINT16 == (value & 0xFFFF))
						{
					
							//printf("6 Prafull\n");
							sprintf(buff, "Not Implemented");
						}
						else
						{
							strcpy(buff, GS_module_text[value]);
						}
						
						break;
					}
					
					case GSconfig_Model_Select:
					{
						if (NI_UINT16 == (value & 0xFFFF))
						{
							//printf("7 Prafull\n");
							sprintf(buff, "Not Implemented");
						}
						else
						{
							uint32 fxr_inv = (NI_UINT16 == readSunSpecRegisterU16(block, GSconfig_AUX_Relay_Mode)) ? 1 : 0;

							if (0 != fxr_inv)
							{						
								strcpy(buff, FXR_sealed_vented_text[value]);
							}
							else
							{
								strcpy(buff, GS_dual_single_module_text[value]);
							}
						}

						break;
					}
																
 					default:
					{
						if (NI_UINT16 == (value & 0xFFFF))
						{
							//printf("8 Prafull\n");
							sprintf(buff, "Not Implemented");
						}
						else
						{
							sprintf(buff, "%u",(uint16) value);
						}
						
						break;
					}
				}				
			}
			else
			{
				value = readSunSpecFormattedValue(block, field, buff);
			}
			
			break;
		}
		
		case INT16_T:
		{	
			
			//printf(" Prafull INT16_T\n");
			if (ENUMERATED_U == md[field].units)
			{
				value = readSunSpecRegisterU16(block, field);
				 //printf("value is %04x", value);
				switch (field)
				{		
					case OutBack_Sched_1_AC_Mode:
					case OutBack_Sched_2_AC_Mode:
					case OutBack_Sched_3_AC_Mode:
					{
						if (NI_INT16 == (value & 0xFFFF))
						{
							//printf("9 Prafull\n");
							strcpy(buff, "Not Implemented");
						}
						else
						{
							strcpy(buff, GS_AC_in_mode[((int16) value) + 1]);
						}
						
						break;
					}
					
					default:
						break;
				}
			}
			else
			{
				value = readSunSpecFormattedValue(block, field, buff);
			}

			break;
		}
			
		case UINT32_T:
		{
			
			//printf(" Prafull UINT32_T\n");
			if (ADDR_U == md[field].units)
			{
				value = readSunSpecAddress(block, field, buff);
				 //printf("value is %04x", value);
			}
			else if (TIME_U == md[field].units)
			{
				time_t unix_time;
				struct tm clock_tm;
				int32 time_zone;
				char tbuff[64];
				
				*tbuff = 0;
				
				unix_time = (time_t) readSunSpecRegisterU32(block, field);

				time_zone = (int32) readSunSpecRegisterS16(1,OutBack_Set_Time_Zone) * 3600;
				unix_time += time_zone;

				if (0 != daylight)
				{
					unix_time -= 3600;
				}
#ifdef WIN32
				temp_clock = &clock_tm;
				_localtime32_s(temp_clock,&unix_time);
				strftime(tbuff, sizeof(tbuff), "%a, %d %b %Y %X", temp_clock);
#else
				memcpy(&clock_tm, localtime((const time_t *) &unix_time), sizeof(struct tm));
				strftime(tbuff, sizeof(tbuff), "%a, %d %b %Y %X", &clock_tm);				
#endif
				strcpy(buff,tbuff);
			}
			else
			{
				value = readSunSpecFormattedValue(block, field, buff);
			}
			break;
		}
			
		case STRING_T:

		//printf(" Prafull STRING_T\n");
		{
			value = readSunSpecString(block, field, buff);
			 //printf("value is %04x", value);
			break;
		}
			
		default:
		{

			//printf(" Prafull Default\n");
			value = readSunSpecFormattedValue(block, field, buff);
			break;
		}
	}
	
	return value;
}


int setFieldWithString(int block, SunSpecField field, char *buff)
{
	int value = 0;
	
	switch (md[field].type)
	{
		case UINT32_T:
			if (ADDR_U == md[field].units)
			{
				printf("field is address \n");
				value = writeSunSpecAddress(block, field, buff);
			}
			else
			{
				printf("field is not address write sunspec address \n");
				value = writeSunSpecRegister(block, field, atoi(buff));
			}
			break;
			
		case STRING_T:
			printf("String type  \n");
			value = writeSunSpecString(block, field, buff);
			break;
			
		default:
			value = writeSunSpecRegister(block, field, atoi(buff));
			break;
	}
	
	return value;
}


int setFieldsWithString(int block, SunSpecField field, char *buff)
{
	char *b_ptr, *comma_ptr;
	int num_regs, rc;
	int addr;

	uint16 value[MODBUS_MAX_WRITE_REGISTERS];
	uint16 check_value[MODBUS_MAX_WRITE_REGISTERS];
	
	comma_ptr = strchr(buff, ',');
	b_ptr = buff;
	num_regs = 0;
	
	addr = deviceOffsetTable[block] + md[field].position -1;

	for (; ; ++field)
	{
    	if (NULL != comma_ptr)
			*comma_ptr = 0;
		
		switch (md[field].type)
		{
			case UINT32_T:
				if (ADDR_U == md[field].units)
				{
					uint32	address;
					
					address = inet_addr(b_ptr);
					
					if ( (INADDR_NONE != address) && ( (num_regs+2) < MODBUS_MAX_WRITE_REGISTERS) )
					{
    					address = BSwap32(address);
								
						value[num_regs+0] = address >> 16;
						value[num_regs+1] = address & 0xffff;

						if (0 != encryptionEnabled)
						{
							ENCRYPT(value[num_regs+0]);
							ENCRYPT(value[num_regs+1]);
						}
						
						num_regs += 2;
					}						
				}
				else if (num_regs < MODBUS_MAX_WRITE_REGISTERS)
				{
					value[num_regs+0] = atoi(b_ptr);
				
					if (0 != encryptionEnabled)
					{
						ENCRYPT(value[num_regs+0]);
					}
					
					++num_regs;
				}
				
				break;
				
			case STRING_T:
			{
				uint16 field_size;
				uint16 i;
				char *value_ptr, *buff_ptr;
			
				field_size = md[field].size;

				if ((num_regs+field_size) < MODBUS_MAX_WRITE_REGISTERS)
				{
					value_ptr = (char *) &value[num_regs];				
					memset(value_ptr, 0, field_size*2);
					
					if (0 != strcmp("NULL", buff))
					{
						buff_ptr = b_ptr;

						for (i = field_size; 0 != i; --i)
						{
							*value_ptr++ = *(buff_ptr+1);
							*value_ptr++ = *(buff_ptr+0);
							buff_ptr += 2;
						}
					}
					
					if (0 != encryptionEnabled)
					{
						for (i = 0; i < field_size; i++)
						{
							ENCRYPT(value[num_regs+i]);
						}
					}
					
					num_regs += field_size;
				}
						
				break;
			}
				
			default:
				if (num_regs < MODBUS_MAX_WRITE_REGISTERS)
				{
					value[num_regs+0] = atoi(b_ptr);
				
					if (0 != encryptionEnabled)
					{
						ENCRYPT(value[num_regs+0]);
					}
					
					++num_regs;
				}
				
				break;
		}
		
		if (NULL == comma_ptr)
		{
			break;
		}
		else
		{
			b_ptr = ++comma_ptr;
			comma_ptr = strchr(b_ptr, ',');
		}
	}
	
	rc = modbus_write_registers(ctx, addr, num_regs, value);
	
	if (-1 == rc)
	{
		fprintf(stderr, "[setFieldsWithString] Write Registers failed: %s\n", modbus_strerror(errno));
   	}
			
	if (rc == num_regs)
	{
		rc = modbus_read_registers(ctx, addr, num_regs, check_value);
		
		if ((rc != num_regs) || (memcmp(value, check_value, 2 * num_regs) != 0))
		{
			rc = -1;
		}
	}
		
	return (rc);
}


int Get_ModBus_Register_address(const int block, const int field)
{
	return (deviceOffsetTable[block] + md[field].position -1);
}




