/*	outback.h	version 0.92	Mar. 1, 2012
 *
 *	Written by Daniel Lloyd. 2011-2012.
 *	Property of OutBack Power Systems Inc.
 */


#ifndef _outback_h
#define _outback_h

/***********************************************************************/
/*
 * basic data types
 */
typedef	unsigned char			uint8;		/*  8 bits */
typedef unsigned short int		uint16;		/* 16 bits */
typedef unsigned int			uint32;		/* 32 bits */
typedef unsigned long long int	uint64;		/* 64 bits */

typedef char					int8;		/*  8 bits */
typedef short int				int16;		/* 16 bits */
typedef int						int32;		/* 32 bits */
typedef long long int			int64;		/* 64 bits */

typedef volatile int8			vint8;		/*  8 bits */
typedef volatile int16			vint16;		/* 16 bits */
typedef volatile int32			vint32;		/* 32 bits */
typedef volatile int64			vint64;		/* 64 bits */

typedef volatile uint8			vuint8;		/*  8 bits */
typedef volatile uint16			vuint16;	/* 16 bits */
typedef volatile uint32			vuint32;	/* 32 bits */
typedef volatile uint64			vuint64;	/* 64 bits */

/***********************************************************************/


#include "modbus.h"

#define MAX_DEVICES						(28)

#define SUNSPEC_COMMON_MODEL_BLOCK_DID		(0x0001)
#define SUNSPEC_AGGREGATOR_BLOCK_DID		(0x0002)
#define SUNSPEC_END_BLOCK_DID				(0xffff)
#define SUNSPEC_INVERTER_SINGLE_DID			(101)
#define SUNSPEC_INVERTER_SPLIT_DID			(102)
#define SUNSPEC_INVERTER_3PHASE_DID			(103)
#define SUNSPEC_BASIC_CC_DID				(64111)
#define SUNSPEC_OUTBACK_DID					(64110)
#define SUNSPEC_OUTBACK_FM_CC_DID			(64112)
#define SUNSPEC_OUTBACK_FX_DID				(64113)
#define SUNSPEC_OUTBACK_FX_CONFIG_DID		(64114)
#define SUNSPEC_OUTBACK_GS_SPLIT_DID		(64115)
#define SUNSPEC_OUTBACK_GS_CONFIG_DID		(64116)
#define SUNSPEC_OUTBACK_GS_SINGLE_DID		(64117)
#define SUNSPEC_OUTBACK_FNDC_DID			(64118)
#define SUNSPEC_OUTBACK_FNDC_CONFIG_DID		(64119)
#define SUNSPEC_OUTBACK_SYS_CONTROL_DID		(64120)
#define SUNSPEC_OUTBACK_STATISTICS_DID		(64255)


#define COMMON_MDOFFSET					(0)
#define	COMMON_FIELDS					(9)
#define	COMMON_SIZE						(69)
#define OUTBACK_MDOFFSET				(COMMON_MDOFFSET + COMMON_FIELDS)
#define	OUTBACK_FIELDS					(166)
#define	OUTBACK_SIZE					(422)
#define OB_SYS_CTRL_MDOFFSET			(OUTBACK_MDOFFSET + OUTBACK_FIELDS)
#define OB_SYS_CTRL_FIELDS				(27)
#define OB_SYS_CTRL_SIZE				(29)
#define CC_MDOFFSET						(OB_SYS_CTRL_MDOFFSET + OB_SYS_CTRL_FIELDS)
#define	CC_FIELDS						(28)
#define	CC_SIZE							(28)
#define CC_CONFIG_MDOFFSET				(CC_MDOFFSET + CC_FIELDS)

#define	CC_CONFIG_FIELDS				(74)
#define	CC_CONFIG_SIZE					(90)

#define I_MDOFFSET						(CC_CONFIG_MDOFFSET + CC_CONFIG_FIELDS)
#define I_FIELDS						(45)
#define I_SIZE							(52)
#define GS_CONFIG_MDOFFSET				(I_MDOFFSET + I_FIELDS)

#define GS_CONFIG_FIELDS				(74)
#define GS_CONFIG_SIZE					(90)

#define GS_SINGLE_MDOFFSET				(GS_CONFIG_MDOFFSET + GS_CONFIG_FIELDS)
#define GS_SINGLE_FIELDS				(48)
#define GS_SINGLE_SIZE					(48)
#define GS_SPLIT_MDOFFSET				(GS_SINGLE_MDOFFSET + GS_SINGLE_FIELDS)
#define GS_SPLIT_FIELDS					(60)
#define GS_SPLIT_SIZE					(60)
#define FX_CONFIG_MDOFFSET				(GS_SPLIT_MDOFFSET + GS_SPLIT_FIELDS)

#define FX_CONFIG_FIELDS				(58)
#define FX_CONFIG_SIZE					(74)

#define FX_MDOFFSET						(FX_CONFIG_MDOFFSET + FX_CONFIG_FIELDS)
#define FX_FIELDS						(38)
#define FX_SIZE							(38)
#define FNDC_CONFIG_MDOFFSET			(FX_MDOFFSET + FX_FIELDS)

#define FNDC_CONFIG_FIELDS				(38)
#define FNDC_CONFIG_SIZE				(54)

#define FNDC_MDOFFSET					(FNDC_CONFIG_MDOFFSET + FNDC_CONFIG_FIELDS)
#define FNDC_FIELDS						(76)
#define FNDC_SIZE						(78)

#define OB_STATS_MDOFFSET				(FNDC_MDOFFSET + FNDC_FIELDS)
#define OB_STATS_FIELDS					(58)
#define OB_STATS_SIZE					(58)

#define	AGGREGATOR_MDOFFSET				(OB_STATS_MDOFFSET + OB_STATS_FIELDS)
#define	AGGREGATOR_FIELDS				(12)
#define	AGGREGATOR_SIZE					(16)

#define END_MDOFFSET					(AGGREGATOR_MDOFFSET + AGGREGATOR_FIELDS)
#define	END_FIELDS						(2)
#define	END_SIZE						(2)
#define NULL_MDOFFSET					(0)
#define	NULL_FIELDS						(0)
#define	NULL_SIZE						(0)
#define	MAX_SIZE						(COMMON_SIZE + OUTBACK_SIZE + OB_SYS_CTRL_SIZE + I_SIZE + MAX_HUB_DEVICES * (GS_SPLIT_SIZE + GS_CONFIG_SIZE) + END_SIZE)

#define C_STATUS_NORMAL					(0x00000000)	//Operating Normally
#define C_STATUS_ERROR					(0xFFFFFFFE)	//Generic Failure
#define C_STATUS_UNK					(0xFFFFFFFF)	//Status Unknown

// for OB_Control_Status

#define STATUS_ABSORB_ENABLED			0x0001
#define STATUS_FLOAT_ENABLED			0x0002
#define STATUS_AC_USE					0x0004
#define STATUS_INVERTER_OFF				0x0008
#define STATUS_INVERTER_SEARCH			0x0010
#define STATUS_INVERTER_ON				0x0020
#define STATUS_SELL_ENABLED				0x0040
#define STATUS_CHARGER_DISABLED			0x0080
#define STATUS_CHARGER_AUTO				0x0100
#define STATUS_CHARGER_BULK_AND_FLOAT	0x0200

#define OUTBACK_PASSWORD_SIZE			(8)
#define SMTP_SERVER_NAME_SIZE			(20)
#define SMTP_ACCOUNT_NAME_SIZE			(16)
#define SMTP_EMAIL_PASSWORD_SIZE		(8)
#define SMTP_EMAIL_USER_NAME_SIZE		(20)
#define STATUS_EMAIL_SUBJECT_SIZE		(25)
#define STATUS_EMAIL_TO_ADDR1_SIZE		(20)
#define STATUS_EMAIL_TO_ADDR2_SIZE		(20)
#define ALARM_EMAIL_SUBJECT_SIZE		(25)
#define ALARM_EMAIL_TO_ADDR1_SIZE		(20)
#define ALARM_EMAIL_TO_ADDR2_SIZE		(20)
#define FTP_PASSWORD_SIZE				(8)
#define TELNET_PASSWORD_SIZE			(8)
#define TIME_SERVER_NAME_SIZE			(20)

#define ERR_LAST_WRITE_LIMIT_HIGH			(0)
#define ERR_LAST_WRITE_LIMIT_LOW			(1)
#define ERR_LAST_WRITE_INVALID				(2)
#define ERR_DHCP_FAILED						(3)
#define ERR_DNS_RESOLVE_FAILED				(4)
#define ERR_SMTP_AUTH_FAILED				(5)
#define ERR_SMTP_SEND_FAILED				(6)
#define ERR_FTP_ERROR						(7)
#define ERR_SD_CARD_ERROR					(8)
#define ERR_SNTP_FAILED						(9)
#define ERR_WRITE_WHILE_LOCKED				(10)
#define	ERR_DEVICE_FW_UPDATE_INVALID		(11) 
#define ERR_DEVICE_FW_UPDATE_FILE_NOT_FOUND	(12) 
#define ERR_DEVICE_FW_UPDATE_FILE_INVALID	(13) 
#define ERR_DEVICE_FW_UPDATE_FAILURE		(14) 
#define ERR_OPERATION_NOT_SUPPORTED			(15) 


#define OUTBACK_READ_ERROR(x,y)			(((x) & (0x0001 << (y))) >> (y))
#define OUTBACK_DEVICE_ERROR(x)			(((x) & 0xF000) >> 12)


typedef enum {
	C_SunSpec_ID,											//	0
	C_SunSpec_DID,											//	1
	C_SunSpec_Length,										//	2
	C_Manufacturer,											//	3
	C_Model,												//	4
	C_Options,												//	5
	C_Version,												//	6
	C_SerialNumber,											//	7
	C_DeviceAddress,										//	8
	
	OutBack_SunSpec_DID,									//	9
	OutBack_SunSpec_Length,									//	10
	OutBack_Major_Firmware_Number,							//	11
	OutBack_Mid_Firmware_Number,							//	12
	OutBack_Minor_Firmware_Number,							//	13
	OutBack_Encryption_Key,									//	14
	OutBack_MAC_Address,									//  15
	OutBack_Write_Password,									//	16
	OutBack_Enable_DHCP,									//	17
	OutBack_TCPIP_Address,									//	18
	OutBack_TCPIP_Gateway,									//	19
	OutBack_TCPIP_Netmask,									//	20
	OutBack_TCPIP_DNS_1,									//	21
	OutBack_TCPIP_DNS_2,									//	22
	OutBack_Modbus_Port,									//	23
	OutBack_SMTP_Server_Name,								//	24
	OutBack_SMTP_Account_Name,								//	25
	OutBack_SMTP_SSL_Enable,								//	26
	OutBack_SMTP_Email_Password,							//	27
	OutBack_SMTP_Email_User_Name,							//	28
	OutBack_Status_Email_Interval,							//	29
	OutBack_Status_Email_Status_Time,						//	30
	OutBack_Status_Email_Subject_Line,						//	31
	OutBack_Status_Email_To_Address_1,						//	32
	OutBack_Status_Email_To_Address_2,						//	33
	OutBack_Alarm_Email_Enable,								//	34
	OutBack_Alarm_Email_Subject_Line,						//	35
	OutBack_Alarm_Email_To_Address_1,						//	36
	OutBack_Alarm_Email_To_Address_2,						//	37
	OutBack_FTP_Password,									//	38
	OutBack_Telnet_Password,								//	39
	OutBack_SD_Card_Log_Write_Interval,						//	40
	OutBack_SD_Card_Log_Retain_Days,						//	41
	OutBack_SD_Card_Logging_Mode,							//	42
	OutBack_Time_Server_Name,								//	43
	OutBack_Enable_Time_Server,								//	44
	OutBack_Set_Time_Zone,									//	45
	OutBack_Enable_Float_Coordination,						//	46
	OutBack_Enable_FNDC_Charge_Termination,					//	47
	OutBack_Enable_FNDC_Grid_Tie_Control,					//	48
	OutBack_Voltage_SF,										//  49
	OutBack_Hour_SF,										//  50
	OutBack_AGS_Mode,										//  51
	OutBack_AGS_Port,										//  52
	OutBack_AGS_Port_Type,									//  53
	OutBack_Generator_Type,									//  54
	OutBack_AGS_DC_Gen_Absorb_Voltage,						//  55 
	OutBack_AGS_DC_Gen_Absorb_Time,                         //  56  
	OutBack_AGS_Fault_Time,                                 //  57  
	OutBack_AGS_Gen_Cool_Down_Time,                         //  58  
	OutBack_AGS_Gen_Warm_Up_Time,                           //  59  
	OutBack_Generator_Exercise_Mode,                        //  60  
	OutBack_Exercise_Start_Hour,                            //  61  
	OutBack_Exercise_Start_Minute,                          //  62  
	OutBack_Exercise_Day,                                   //  63  
	OutBack_Exercise_Period,                                //  64  
	OutBack_Exercise_Interval,                              //  65  
	OutBack_AGS_Sell_Mode,                                  //  66  
	OutBack_AGS_2_Min_Start_Mode,                           //  67  
	OutBack_AGS_2_Min_Start_Voltage,                        //  68  
	OutBack_AGS_2_Hour_Start_Mode,                          //  69  
	OutBack_AGS_2_Hour_Start_Voltage,                       //  70  
	OutBack_AGS_24_Hour_Start_Mode,                         //  71  
	OutBack_AGS_24_Hour_Start_Voltage,                      //  72  
	OutBack_AGS_Load_Start_Mode,                            //  73  
	OutBack_AGS_Load_Start_kW,                              //  74  
	OutBack_AGS_Load_Start_Delay,                           //  75  
	OutBack_AGS_Load_Stop_kW,                               //  76  
	OutBack_AGS_Load_Stop_Delay,                            //  77  
	OutBack_AGS_SOC_Start_Mode,                             //  78  
	OutBack_AGS_SOC_Start_Percentage,                       //  79  
	OutBack_AGS_SOC_Stop_Percentage,                        //  80  
	OutBack_AGS_Enable_Full_Charge_Mode,                    //  81  
	OutBack_AGS_Full_Charge_Interval,                       //  82  
	OutBack_AGS_Must_Run_Mode,                              //  83  
	OutBack_AGS_Must_Run_Weekday_Start_Hour,                //  84  
	OutBack_AGS_Must_Run_Weekday_Start_Minute,              //  85  
	OutBack_AGS_Must_Run_Weekday_Stop_Hour,                 //  86  
	OutBack_AGS_Must_Run_Weekday_Stop_Minute,               //  87  
	OutBack_AGS_Must_Run_Weekend_Start_Hour,                //  88  
	OutBack_AGS_Must_Run_Weekend_Start_Minute,              //  89  
	OutBack_AGS_Must_Run_Weekend_Stop_Hour,                 //  90  
	OutBack_AGS_Must_Run_Weekend_Stop_Minute,               //  91  
	OutBack_AGS_Quiet_Time_Mode,                            //  92  
	OutBack_AGS_Quiet_Time_Weekday_Start_Hour,              //  93  
	OutBack_AGS_Quiet_Time_Weekday_Start_Minute,            //  94  
	OutBack_AGS_Quiet_Time_Weekday_Stop_Hour,               //  95  
	OutBack_AGS_Quiet_Time_Weekday_Stop_Minute,             //  96  
	OutBack_AGS_Quiet_Time_Weekend_Start_Hour,              //  97  
	OutBack_AGS_Quiet_Time_Weekend_Start_Minute,            //  98  
	OutBack_AGS_Quiet_Time_Weekend_Stop_Hour,               //  99  
	OutBack_AGS_Quiet_Time_Weekend_Stop_Minute,             // 100  
	OutBack_AGS_Total_Generator_Run_Time,                   // 101  
	OutBack_HBX_Mode,                                       // 102  
	OutBack_HBX_Grid_Connect_Voltage,                       // 103  
	OutBack_HBX_Grid_Connect_Delay,                         // 104  
	OutBack_HBX_Grid_Disconnect_Voltage,                    // 105  
	OutBack_HBX_Grid_Disconnect_Delay,                      // 106  
	OutBack_HBX_Grid_Connect_SOC,                           // 107  
	OutBack_HBX_Grid_Disconnect_SOC,                        // 108  
	OutBack_Grid_Use_Interval_1_Mode,                       // 109  
	OutBack_Grid_Use_Interval_1_Weekday_Start_Hour,         // 110  
	OutBack_Grid_Use_Interval_1_Weekday_Start_Minute,       // 111  
	OutBack_Grid_Use_Interval_1_Weekday_Stop_Hour,          // 112  
	OutBack_Grid_Use_Interval_1_Weekday_Stop_Minute,        // 113  
	OutBack_Grid_Use_Interval_1_Weekend_Start_Hour,         // 114  
	OutBack_Grid_Use_Interval_1_Weekend_Start_Minute,       // 115  
	OutBack_Grid_Use_Interval_1_Weekend_Stop_Hour,          // 116  
	OutBack_Grid_Use_Interval_1_Weekend_Stop_Minute,        // 117  
	OutBack_Grid_Use_Interval_2_Mode,                       // 118  
	OutBack_Grid_Use_Interval_2_Weekday_Start_Hour,         // 119  
	OutBack_Grid_Use_Interval_2_Weekday_Start_Minute,       // 120  
	OutBack_Grid_Use_Interval_2_Weekday_Stop_Hour,          // 121  
	OutBack_Grid_Use_Interval_2_Weekday_Stop_Minute,        // 122  
	OutBack_Grid_Use_Interval_3_Mode,                       // 123  
	OutBack_Grid_Use_Interval_3_Weekday_Start_Hour,         // 124  
	OutBack_Grid_Use_Interval_3_Weekday_Start_Minute,       // 125  
	OutBack_Grid_Use_Interval_3_Weekday_Stop_Hour,          // 126  
	OutBack_Grid_Use_Interval_3_Weekday_Stop_Minute,        // 127  
	OutBack_Load_Grid_Transfer_Mode,                        // 128  
	OutBack_Load_Grid_Transfer_Threshold,                   // 129  
	OutBack_Load_Grid_Transfer_Connect_Delay,               // 130  
	OutBack_Load_Grid_Transfer_Disconnect_Delay,            // 131  
	OutBack_Load_Grid_Transfer_Connect_Battery_Voltage,     // 132  
	OutBack_Load_Grid_Transfer_Re_Connect_Battery_Voltage,  // 133
	OutBack_Global_Charger_Control_Mode,                    // 134  
	OutBack_Global_Charger_Output_Limit,                    // 135  
	OutBack_Radian_AC_Coupled_Mode,                         // 136  
	OutBack_Radian_AC_Coupled_AUX_Port,                     // 137  
	OutBack_URL_Lock,                                       // 138  
	OutBack_Web_Reporting_Base_URL,                         // 139  
	OutBack_Web_User_Logged_In_Status,						// 140  
	OutBack_HUB_Type,										// 141  
	OutBack_HUB_Major_Firmware_Number,                      // 142  
	OutBack_HUB_Mid_Firmware_Number,                        // 143  
	OutBack_HUB_Minor_Firmware_Number,	                    // 144  
	OutBack_Year,										    // 145
	OutBack_Month,										    // 146
	OutBack_Day,										    // 147
	OutBack_Hour,										    // 148
	OutBack_Minute,										    // 149
	OutBack_Second,										    // 150
	OutBack_Temp_Batt,									    // 151
	OutBack_Temp_Ambient,								    // 152
	OutBack_Temp_SF,									    // 153
	OutBack_Error,										    // 154
	OutBack_Status,										    // 155
	OutBack_Update_Device_Firmware_Port,				    // 156
	OutBack_Gateway_Type,									// 157
	OutBack_System_Voltage,									// 158
	OutBack_Measured_System_Voltage,						// 159
	OutBack_AGS_AC_Reconnect_Delay,							// 160
	OutBack_Multi_Phase_Coordination,						// 161
	OutBack_Sched_1_AC_Mode,								// 162
	OutBack_Sched_1_AC_Mode_Hour,							// 163
	OutBack_Sched_1_AC_Mode_Min,							// 164
	OutBack_Sched_2_AC_Mode,								// 165
	OutBack_Sched_2_AC_Mode_Hour,							// 166
	OutBack_Sched_2_AC_Mode_Min,							// 167
	OutBack_Sched_3_AC_Mode,								// 168
	OutBack_Sched_3_AC_Mode_Hour,							// 169
	OutBack_Sched_3_AC_Mode_Min,							// 170
	OutBack_Auto_reboot,									// 171
	OutBack_Spare_Reg_2,									// 172
	OutBack_Spare_Reg_3,									// 173
	OutBack_Spare_Reg_4,									// 174
	
	OB_SunSpec_DID,										    // 175
	OB_SunSpec_Length,									    // 176
	OB_DC_Voltage_SF,									    // 177
	OB_AC_Current_SF,									    // 178
	OB_Time_SF,											    // 179
	OB_Bulk_Charge_Enable_Disable,						    // 180
	OB_Inverter_AC_Drop_Use,							    // 181
	OB_Set_Inverter_Mode,								    // 182
	OB_Grid_Tie_Mode,									    // 183
	OB_Set_Inverter_Charger_Mode,						    // 184
	OB_Control_Status,									    // 185
	OB_Set_Sell_Voltage,								    // 186
	OB_Set_Radian_Inverter_Sell_Current_Limit,			    // 187
	OB_Set_Absorb_Voltage,								    // 188
	OB_Set_Absorb_Time,									    // 189
	OB_Set_Float_Voltage,								    // 190
	OB_Set_Float_Time,									    // 191
	OB_Set_Inverter_Charger_Current_Limit,				    // 192
	OB_Set_Inverter_AC1_Current_Limit,					    // 193
	OB_Set_Inverter_AC2_Current_Limit,					    // 194
	OB_Set_AGS_OP_Mode,										// 195       
	OB_AGS_Operational_State,								// 196
	OB_AGS_Operational_State_Timer,							// 197
	OB_Gen_Last_Run_Start_Time_GMT,							// 198
	OB_Gen_Last_Run_Duration,								// 199
	OB_Set_AC_Output_Freq_Offline_Mode,						// 200
	OB_Set_AC_Output_Offline_Freq,							// 201
	
	CC_SunSpec_DID,											// 202  
	CC_SunSpec_Length,										// 203 
	CC_port_number,											// 204  
	CC_Voltage_SF,											// 205  
	CC_Current_SF,											// 206  
	CC_Power_SF,											// 207  
	CC_AH_SF,												// 208  
	CC_KWH_SF,												// 209  
	CC_Batt_Voltage,										// 210  
	CC_Array_Voltage,										// 211  
	CC_Batt_Current,										// 212  
	CC_Array_Current,										// 213  
	CC_Charger_State,										// 214  
	CC_Watts,												// 215  
	CC_Todays_Min_Battery_Volts,							// 216  
	CC_Todays_Max_Battery_Volts,							// 217  
	CC_VOC,													// 218  
	CC_Todays_Peak_VOC,										// 219  
	CC_Todays_kWH,											// 220  
	CC_Todays_AH,											// 221  
	CC_Lifetime_kWH_Hours,									// 222  
	CC_Lifetime_kAmp_Hours,									// 223  
	CC_Lifetime_Max_Watts,									// 224  
	CC_Lifetime_Max_Battery_Volts,							// 225  
	CC_Lifetime_Max_VOC,									// 226  
	CC_Temp_SF,												// 227  
	CC_Temp_Output_FETs,									// 228  
	CC_Temp_Enclosure,										// 229
	  
	CCconfig_SunSpec_DID,								    // 230
	CCconfig_SunSpec_Length,							    // 231
	CCconfig_port_number,								    // 232
	CCconfig_Voltage_SF,								    // 233
	CCconfig_Current_SF,								    // 234
	CCconfig_Hours_SF,									    // 235
	CCconfig_Power_SF,									    // 236
	CCconfig_AH_SF,										    // 237
	CCconfig_KWH_SF,									    // 238
	CCconfig_Faults,									    // 239
	CCconfig_Absorb_Volts,								    // 240
	CCconfig_Absorb_Time_Hours,							    // 241
	CCconfig_Absorb_End_Amps,							    // 242
	CCconfig_Rebulk_Volts,								    // 243 
	CCconfig_Float_Volts,								    // 244
	CCconfig_Bulk_Current,								    // 245
	CCconfig_EQ_Volts,									    // 246
	CCconfig_EQ_Time_Hours,								    // 247
	CCconfig_Auto_EQ_Days,								    // 248
	CCconfig_MPPT_Mode,									    // 249
	CCconfig_Sweep_Width,								    // 250
	CCconfig_Sweep_Max_Percentage,						    // 251
	CCconfig_U_Pick_PWM_Duty_Cycle,						    // 252
	CCconfig_Grid_Tie_Mode,								    // 253
	CCconfig_Temp_Comp_Mode,							    // 254
	CCconfig_Temp_Comp_Lower_Limit_Volts,				    // 255
	CCconfig_Temp_Comp_Upper_Limit_Volts,				    // 256
	CCconfig_Temp_Comp_Slope,							    // 257
	CCconfig_Auto_Restart_Mode,							    // 258
	CCconfig_Wakeup_VOC,								    // 259
	CCconfig_Snooze_Mode_Amps,							    // 260
	CCconfig_Wakeup_Interval,							    // 261
	CCconfig_AUX_Mode,									    // 262
	CCconfig_AUX_Control,								    // 263
	CCconfig_AUX_State,									    // 264
	CCconfig_AUX_Polarity,								    // 265
	CCconfig_AUX_Low_Batt_Disconnect,					    // 266
	CCconfig_AUX_Low_Batt_Reconnect,					    // 267
	CCconfig_AUX_Low_Batt_Disconnect_Delay,				    // 268
	CCconfig_AUX_Vent_Fan_Volts,						    // 269
	CCconfig_AUX_PV_Limit_Volts,						    // 270
	CCconfig_AUX_PV_Limit_Hold_Time,					    // 271
	CCconfig_AUX_Night_Light_Thres_Volts,				    // 272 
	CCconfig_Night_Light_ON_Hours,						    // 273
	CCconfig_Night_Light_ON_Hyst_Time,					    // 274
	CCconfig_Night_Light_OFF_Hyst_Time,					    // 275
	CCconfig_AUX_Error_Battery_Volts,					    // 276
	CCconfig_AUX_Divert_Hold_Time,						    // 277
	CCconfig_AUX_Divert_Delay_Time,						    // 278
	CCconfig_AUX_Divert_Relative_Volts,					    // 279
	CCconfig_AUX_Divert_Hyst_Volts,						    // 280
	CCconfig_Major_Firmware_Number,						    // 281
	CCconfig_Mid_Firmware_Number,						    // 282
	CCconfig_Minor_Firmware_Number,						    // 283
	CCconfig_Set_Log_Day_Offset,						    // 284
	CCconfig_Get_Current_Log_Day_Offset,				    // 285
	CCconfig_Log_Daily_AH,								    // 286
	CCconfig_Log_Daily_kWH,								    // 287
	CCconfig_Log_Daily_Max_Output_Amps,					    // 288
	CCconfig_Log_Daily_Max_Output_Watts,				    // 289
	CCconfig_Log_Daily_Absorb_Time,						    // 290
	CCconfig_Log_Daily_Float_Time,						    // 291
	CCconfig_Log_Daily_Min_Batt_Volts,					    // 292
	CCconfig_Log_Daily_Max_Batt_Volts,					    // 293
	CCconfig_Log_Daily_Max_Input_Volts,					    // 294
	CCconfig_Clear_Log_Read,							    // 295
	CCconfig_Clear_Log_Write_Complement,				    // 296
	CCconfig_Stats_Maximum_Reset_Read,					    // 297
	CCconfig_Stats_Maximum_Write_Complement,			    // 298
	CCconfig_Stats_Totals_Reset_Read,					    // 299
	CCconfig_Stats_Totals_Write_Complement,				    // 300
	CCconfig_Battery_Voltage_Calibrate_Offset,				// 301
	CCconfig_Serial_Number,								    // 302
	CCconfig_Model_Number,								    // 303
	
	I_SunSpec_DID,										    // 304
	I_SunSpec_Length,									    // 305
	I_AC_Current,										    // 306
	I_AC_CurrentA,										    // 307
	I_AC_CurrentB,										    // 308
	I_AC_CurrentC,										    // 309
	I_AC_Current_SF,									    // 310
	I_AC_VoltageAB,										    // 311
	I_AC_VoltageBC,										    // 312
	I_AC_VoltageCA,										    // 313
	I_AC_VoltageAN,										    // 314
	I_AC_VoltageBN,										    // 315
	I_AC_VoltageCN,										    // 316
	I_AC_Voltage_SF,									    // 317
	I_AC_Power,											    // 318
	I_AC_Power_SF,										    // 319
	I_AC_Frequency,										    // 320
	I_AC_Frequency_SF,									    // 321
	I_AC_VA,											    // 322
	I_AC_VA_SF,											    // 323
	I_AC_VAR,											    // 324
	I_AC_VAR_SF,										    // 325
	I_AC_PF,											    // 326
	I_AC_PF_SF,											    // 327
	I_AC_Energy_WH,										    // 328
	I_AC_Energy_WH_SF,									    // 329
	I_DC_Current,										    // 330
	I_DC_Current_SF,									    // 331
	I_DC_Voltage,										    // 332
	I_DC_Voltage_SF,									    // 333
	I_DC_Power,											    // 334
	I_DC_Power_SF,										    // 335
	I_Temp_Cab,											    // 336
	I_Temp_Sink,										    // 337
	I_Temp_Trans,										    // 338
	I_Temp_Other,										    // 339
	I_Temp_SF,											    // 340
	I_Status,											    // 341
	I_Status_Vendor,									    // 342
	I_Event_1,											    // 343
	I_Event_2,											    // 344
	I_Event_1_Vendor,									    // 345
	I_Event_2_Vendor,									    // 346
	I_Event_3_Vendor,									    // 347
	I_Event_4_Vendor,									    // 348
	
	GSconfig_SunSpec_DID,								    // 349
	GSconfig_SunSpec_Length,							    // 350
	GSconfig_Port_Number,								    // 351
	GSconfig_DC_Voltage_SF,								    // 352
	GSconfig_AC_Current_SF,								    // 353
	GSconfig_AC_Voltage_SF,								    // 354
	GSconfig_Time_SF,									    // 355
	GSconfig_Major_Firmware_Number,						    // 356
	GSconfig_Mid_Firmware_Number,						    // 357
	GSconfig_Minor_Firmware_Number,						    // 358
	GSconfig_Absorb_Volts,								    // 359
	GSconfig_Absorb_Time_Hours,							    // 360
	GSconfig_Float_Volts,								    // 361
	GSconfig_Float_Time_Hours,							    // 362
	GSconfig_ReFloat_Volts,								    // 363
	GSconfig_EQ_Volts,									    // 364
	GSconfig_EQ_Time_Hours,								    // 365
	GSconfig_Search_Sensitivity,						    // 366
	GSconfig_Search_Pulse_Length,						    // 367
	GSconfig_Search_Pulse_Spacing,						    // 368
	GSconfig_AC_Input_Select_Priority,					    // 369
	GSconfig_Grid_AC_Input_Current_Limit,				    // 370
	GSconfig_Gen_AC_Input_Current_Limit,				    // 371
	GSconfig_Charger_AC_Input_Current_Limit,			    // 372
	GSconfig_Charger_Operating_Mode,					    // 373
	GSconfig_AC_Coupled,								    // 374
	GSconfig_Grid_Input_Mode,							    // 375
	GSconfig_Grid_Lower_Input_Voltage_Limit,			    // 376
	GSconfig_Grid_Upper_Input_Voltage_Limit,			    // 377
	GSconfig_Grid_Transfer_Delay,						    // 378
	GSconfig_Grid_Connect_Delay,						    // 379
	GSconfig_Gen_Input_Mode,							    // 380
	GSconfig_Gen_Lower_Input_Voltage_Limit,				    // 381
	GSconfig_Gen_Upper_Input_Voltage_Limit,				    // 382
	GSconfig_Gen_Transfer_Delay,						    // 383
	GSconfig_Gen_Connect_Delay,							    // 384
	GSconfig_AC_Output_Voltage,							    // 385
	GSconfig_Low_Battery_Cut_Out_Voltage,				    // 386
	GSconfig_Low_Battery_Cut_In_Voltage,				    // 387
	GSconfig_AUX_Mode,									    // 388
	GSconfig_AUX_Control,								    // 389
	GSconfig_AUX_ON_Battery_Voltage,					    // 390
	GSconfig_AUX_ON_Delay_Time,							    // 391
	GSconfig_AUX_OFF_Battery_Voltage,					    // 392
	GSconfig_AUX_OFF_Delay_Time,						    // 393
	GSconfig_AUX_Relay_Mode,							    // 394
	GSconfig_AUX_Relay_Control,							    // 395
	GSconfig_AUX_Relay_ON_Battery_Voltage,				    // 396
	GSconfig_AUX_Relay_ON_Delay_Time,					    // 397
	GSconfig_AUX_Relay_OFF_Battery_Voltage,				    // 398
	GSconfig_AUX_Relay_OFF_Delay_Time,					    // 399
	GSconfig_Stacking_Mode,								    // 400
	GSconfig_Master_Power_Save_Level,					    // 401
	GSconfig_Slave_Power_Save_Level,					    // 402
	GSconfig_Sell_Volts,								    // 403
	GSconfig_Grid_Tie_Window,							    // 404
	GSconfig_Grid_Tie_Enable,							    // 405
	GSconfig_Grid_AC_Input_Voltage_Cal_Factor,				// 406
	GSconfig_Gen_AC_Input_Voltage_Cal_Factor,				// 407
	GSconfig_AC_Output_Voltage_Cal_Factor,				    // 408
	GSconfig_Battery_Voltage_Cal_Factor,				    // 409	
	GSconfig_ReBulk_Volts,								    // 410
	GSconfig_Mini_Grid_LBX_Volts,						    // 411
	GSconfig_Mini_Grid_LBX_Delay,						    // 412
	GSconfig_Grid_Zero_DoD_Volts,						    // 413
	GSconfig_Grid_Zero_DoD_Max_Offset_AC_Amps,			    // 414			  
	GSconfig_Serial_Number,								    // 415
	GSconfig_Model_Number,								    // 416
	GSconfig_Module_Control,								// 417
	GSconfig_Model_Select,									// 418
	GSconfig_Low_Battery_Cut_Out_Delay,						// 419
	GSconfig_High_Battery_Cut_Out_Voltage,					// 420
	GSconfig_High_Battery_Cut_In_Voltage,					// 421
	GSconfig_High_Battery_Cut_Out_Delay,					// 422

	GS_Single_SunSpec_DID,								    // 423
	GS_Single_SunSpec_Length,							    // 424
	GS_Single_Port_Number,								    // 425
	GS_Single_DC_Voltage_SF,							    // 426
	GS_Single_AC_Current_SF,							    // 427
	GS_Single_AC_Voltage_SF,							    // 428
	GS_Single_Frequency_SF,								    // 429
	GS_Single_Inverter_Output_Current,					    // 430
	GS_Single_Inverter_Charge_Current,					    // 431
	GS_Single_Inverter_Buy_Current,						    // 432
	GS_Single_Inverter_Sell_Current,					    // 433
	GS_Single_Grid_Input_AC_Voltage,					    // 434
	GS_Single_Gen_Input_AC_Voltage,						    // 435
	GS_Single_AC_Output_Voltage,						    // 436
	GS_Single_Inverter_Operating_Mode,					    // 437
	GS_Single_Error_Flags,								    // 438
	GS_Single_Warning_Flags,							    // 439
	GS_Single_Battery_Voltage,							    // 440
	GS_Single_Temp_Comp_Target_Voltage,					    // 441
	GS_Single_AUX_Output_State,							    // 442
	GS_Single_AUX_Relay_Output_State,					    // 443
	GS_Single_L_Module_Transformer_Temp,				    // 444
	GS_Single_L_Module_Capacitor_Temp,					    // 445
	GS_Single_L_Module_FET_Temp,						    // 446
	GS_Single_R_Module_Transformer_Temp,				    // 447
	GS_Single_R_Module_Capacitor_Temp,					    // 448
	GS_Single_R_Module_FET_Temp,						    // 449
	GS_Single_Battery_Temperature,						    // 450
	GS_Single_AC_Input_Selection,						    // 451
	GS_Single_AC_Input_Frequency,						    // 452
	GS_Single_AC_Input_Voltage,							    // 453
	GS_Single_AC_Input_State,							    // 454
	GS_Single_Minimum_AC_Input_Voltage,					    // 455
	GS_Single_Maximum_AC_Input_Voltage,					    // 456
	GS_Single_Sell_Status,								    // 457
	GS_Single_kWh_SF,									    // 458	
	GS_Single_AC1_Buy_kWh,									// 459
	GS_Single_AC2_Buy_kWh,									// 460
	GS_Single_AC1_Sell_kWh,									// 461
	GS_Single_AC2_Sell_kWh,									// 462
	GS_Single_Output_kWh,								    // 463
	GS_Single_Charger_kWh,                                  // 464
	GS_Single_Output_kW,                                    // 465
	GS_Single_Buy_kW,                                       // 466
	GS_Single_Sell_kW,                                      // 467
	GS_Single_Charge_kW,                                    // 468
	GS_Single_Load_kW,                                      // 469
	GS_Single_AC_Couple_kW,                                 // 470
	
	GS_Split_SunSpec_DID,								    // 471
	GS_Split_SunSpec_Length,							    // 472
	GS_Split_Port_Number,								    // 473
	GS_Split_DC_Voltage_SF,								    // 474
	GS_Split_AC_Current_SF,								    // 475
	GS_Split_AC_Voltage_SF,								    // 476
	GS_Split_Frequency_SF,								    // 477
	GS_Split_L1_Inverter_Output_Current,				    // 478
	GS_Split_L1_Inverter_Charge_Current,				    // 479
	GS_Split_L1_Inverter_Buy_Current,					    // 480
	GS_Split_L1_Inverter_Sell_Current,					    // 481
	GS_Split_L1_Grid_Input_AC_Voltage,					    // 482
	GS_Split_L1_Gen_Input_AC_Voltage,					    // 483
	GS_Split_L1_AC_Output_Voltage,						    // 484
	GS_Split_L2_Inverter_Output_Current,				    // 485
	GS_Split_L2_Inverter_Charge_Current,				    // 486
	GS_Split_L2_Inverter_Buy_Current,					    // 487
	GS_Split_L2_Inverter_Sell_Current,					    // 488
	GS_Split_L2_Grid_Input_AC_Voltage,					    // 489
	GS_Split_L2_Gen_Input_AC_Voltage,					    // 490
	GS_Split_L2_AC_Output_Voltage,						    // 491
	GS_Split_Inverter_Operating_Mode,					    // 492
	GS_Split_Error_Flags,								    // 493
	GS_Split_Warning_Flags,								    // 494
	GS_Split_Battery_Voltage,							    // 495
	GS_Split_Temp_Comp_Target_Voltage,					    // 496
	GS_Split_AUX_Output_State,							    // 497
	GS_Split_AUX_Relay_Output_State,					    // 498
	GS_Split_L_Module_Transformer_Temp,					    // 499
	GS_Split_L_Module_Capacitor_Temp,					    // 500
	GS_Split_L_Module_FET_Temp,							    // 501
	GS_Split_R_Module_Transformer_Temp,					    // 502
	GS_Split_R_Module_Capacitor_Temp,					    // 503
	GS_Split_R_Module_FET_Temp,							    // 504
	GS_Split_Battery_Temperature,						    // 505
	GS_Split_AC_Input_Selection,						    // 506
	GS_Split_AC_Input_Frequency,						    // 507
	GS_Split_AC_Input_Voltage,							    // 508
	GS_Split_AC_Input_State,							    // 509
	GS_Split_Minimum_AC_Input_Voltage,					    // 510
	GS_Split_Maximum_AC_Input_Voltage,					    // 511
	GS_Split_Sell_Status,								    // 512
	GS_Split_kWh_SF,									    // 513
	GS_Split_AC1_L1_Buy_kWh,								// 514
	GS_Split_AC2_L1_Buy_kWh,								// 515
	GS_Split_AC1_L1_Sell_kWh,								// 516
	GS_Split_AC2_L1_Sell_kWh,								// 517
	GS_Split_L1_Output_kWh,									// 518
	GS_Split_AC1_L2_Buy_kWh,								// 519
	GS_Split_AC2_L2_Buy_kWh,								// 520
	GS_Split_AC1_L2_Sell_kWh,								// 521
	GS_Split_AC2_L2_Sell_kWh,								// 522
	GS_Split_L2_Output_kWh,									// 523
	GS_Split_Charger_kWh,                                   // 524
	GS_Split_Output_kW,                                     // 525
	GS_Split_Buy_kW,                                        // 526
	GS_Split_Sell_kW,                                       // 527
	GS_Split_Charge_kW,                                     // 528
	GS_Split_Load_kW,                                       // 529
	GS_Split_AC_Couple_kW,                                  // 530
	
	FXconfig_SunSpec_DID,								    // 531
	FXconfig_SunSpec_Length,							    // 532
	FXconfig_Port_Number,								    // 533
	FXconfig_DC_Voltage_SF,								    // 534
	FXconfig_AC_Current_SF,								    // 535
	FXconfig_AC_Voltage_SF,								    // 536
	FXconfig_Time_SF,									    // 537 
	FXconfig_Major_Firmware_Number,						    // 538
	FXconfig_Mid_Firmware_Number,						    // 539
	FXconfig_Minor_Firmware_Number,						    // 540
	FXconfig_Absorb_Volts,								    // 541
	FXconfig_Absorb_Time_Hours,							    // 542
	FXconfig_Float_Volts,								    // 543
	FXconfig_Float_Time_Hours,							    // 544
	FXconfig_ReFloat_Volts,								    // 545
	FXconfig_EQ_Volts,									    // 546
	FXconfig_EQ_Time_Hours,								    // 547
	FXconfig_Search_Sensitivity,						    // 548
	FXconfig_Search_Pulse_Length,						    // 549 
	FXconfig_Search_Pulse_Spacing,						    // 550
	FXconfig_AC_Input_Type,								    // 551
	FXconfig_Input_Support,								    // 552
	FXconfig_Grid_AC_Input_Current_Limit,				    // 553
	FXconfig_Gen_AC_Input_Current_Limit,				    // 554
	FXconfig_Charger_AC_Input_Current_Limit,			    // 555
	FXconfig_Charger_Operating_Mode,					    // 556
	FXconfig_Grid_Lower_Input_Voltage_Limit,			    // 557
	FXconfig_Grid_Upper_Input_Voltage_Limit,			    // 558
	FXconfig_Grid_Transfer_Delay,						    // 559
	FXconfig_Gen_Lower_Input_Voltage_Limit,				    // 560
	FXconfig_Gen_Upper_Input_Voltage_Limit,				    // 561
	FXconfig_Gen_Transfer_Delay,						    // 562
	FXconfig_Gen_Connect_Delay,							    // 563
	FXconfig_AC_Output_Voltage,							    // 564
	FXconfig_Low_Battery_Cut_Out_Voltage,				    // 565 
	FXconfig_Low_Battery_Cut_In_Voltage,				    // 566
	FXconfig_AUX_Mode,									    // 567
	FXconfig_AUX_Control,								    // 568
	FXconfig_AUX_Load_Shed_Enable_Voltage,				    // 569
	FXconfig_AUX_Gen_Alert_On_Voltage,					    // 570
	FXconfig_AUX_Gen_Alert_On_Delay,					    // 571
	FXconfig_AUX_Gen_Alert_Off_Voltage,					    // 572
	FXconfig_AUX_Gen_Alert_Off_Delay,					    // 573
	FXconfig_AUX_Vent_Fan_Enable_Voltage,				    // 574
	FXconfig_AUX_Vent_Fan_Off_Period,					    // 575
	FXconfig_AUX_Divert_Enable_Voltage,					    // 576
	FXconfig_AUX_Divert_Off_Delay,						    // 577
	FXconfig_Stacking_Mode,								    // 578
	FXconfig_Master_Power_Save_Level,					    // 579
	FXconfig_Slave_Power_Save_Level,					    // 580
	FXconfig_Sell_Volts,								    // 581
	FXconfig_Grid_Tie_Window,							    // 582
	FXconfig_Grid_Tie_Enable,							    // 583
	FXconfig_AC_Input_Voltage_Calibrate_Factor,			    // 584
	FXconfig_AC_Output_Voltage_Calibrate_Factor,		    // 585
	FXconfig_Battery_Voltage_Calibrate_Factor,			    // 586
	FXconfig_Serial_Number,								    // 587
	FXconfig_Model_Number,								    // 588
	
	FX_SunSpec_DID,										    // 589
	FX_SunSpec_Length,									    // 590
	FX_Port_Number,										    // 591
	FX_DC_Voltage_SF,									    // 592
	FX_AC_Current_SF,									    // 593
	FX_AC_Voltage_SF,									    // 594
	FX_AC_Frequency_SF,									    // 595
	FX_Inverter_Output_Current,							    // 596
	FX_Inverter_Charge_Current,							    // 597
	FX_Inverter_Buy_Current,							    // 598
	FX_Inverter_Sell_Current,							    // 599
	FX_Output_AC_Voltage,								    // 600
	FX_Inverter_Operating_Mode,							    // 601 
	FX_Error_Flags,										    // 602
	FX_Warning_Flags,									    // 603
	FX_Battery_Voltage,									    // 604
	FX_Temp_Compensated_Target_Voltage,					    // 605
	FX_AUX_Output_State,								    // 606
	FX_Transformer_Temperature,							    // 607
	FX_Capacitor_Temperature,							    // 608
	FX_FET_Temperature,									    // 609
	FX_AC_Input_Frequency,								    // 610
	FX_AC_Input_Voltage,								    // 611
	FX_AC_Input_State,									    // 612
	FX_Minimum_AC_Input_Voltage,						    // 613
	FX_Maximum_AC_Input_Voltage,						    // 614
	FX_Sell_Status,										    // 615
	FX_kWh_SF,											    // 616
	FX_Buy_kWh,											    // 617
	FX_Sell_kWh,										    // 618
	FX_Output_kWh,										    // 619
	FX_Charger_kWh,                                         // 620
	FX_Output_kW,                                           // 621
	FX_Buy_kW,                                              // 622
	FX_Sell_kW,                                             // 623
	FX_Charge_kW,                                           // 624
	FX_Load_kW,                                             // 625
	FX_AC_Couple_kW,                                        // 626
	
	FNconfig_SunSpec_DID,								    // 627
	FNconfig_SunSpec_Length,							    // 628 
	FNconfig_Port_Number,								    // 629
	FNconfig_DC_Voltage_SF,								    // 630
	FNconfig_DC_Current_SF,								    // 631
	FNconfig_kWh_SF,									    // 632
	FNconfig_Major_Firmware_Number,						    // 633
	FNconfig_Mid_Firmware_Number,						    // 634
	FNconfig_Minor_Firmware_Number,						    // 635
	FNconfig_Battery_Capacity,							    // 636
	FNconfig_Charged_Volts,								    // 637
	FNconfig_Charged_Time,								    // 638
	FNconfig_Battery_Charged_Amps,						    // 639
	FNconfig_Charge_Factor,								    // 640
	FNconfig_Shunt_A_Enabled,							    // 641
	FNconfig_Shunt_B_Enabled,							    // 642
	FNconfig_Shunt_C_Enabled,							    // 643
	FNconfig_Relay_Control,								    // 644
	FNconfig_Relay_Invert_Logic,						    // 645
	FNconfig_Relay_High_Voltage,						    // 646
	FNconfig_Relay_Low_Voltage,							    // 647
	FNconfig_Relay_SOC_High,							    // 648
	FNconfig_Relay_SOC_Low,								    // 649
	FNconfig_Relay_High_Enable_Delay,					    // 650
	FNconfig_Relay_Low_Enable_Delay,					    // 651
	FNconfig_Set_Data_Log_Day_Offset,					    // 652
	FNconfig_Get_Current_Data_Log_Day_Offset,			    // 653
	FNconfig_Datalog_Minimum_SOC,						    // 654
	FNconfig_Datalog_Input_AH,							    // 655
	FNconfig_Datalog_Input_kWh,							    // 656
	FNconfig_Datalog_Output_AH,							    // 657
	FNconfig_Datalog_Output_kWh,						    // 658
	FNconfig_Datalog_NET_AH,							    // 659
	FNconfig_Datalog_NET_kWh,							    // 660
	FNconfig_Clear_Data_Log_Read,						    // 661
	FNconfig_Clear_Data_Log_Write_Complement,			    // 662
	FNconfig_Serial_Number,								    // 663
	FNconfig_Model_Number,								    // 664
	
	FN_SunSpec_DID,										    // 665
	FN_SunSpec_Length,									    // 666
	FN_Port_Number,										    // 667
	FN_DC_Voltage_SF,									    // 668
	FN_DC_Current_SF,									    // 669
	FN_Time_SF,											    // 670
	FN_kWh_SF,											    // 671	
	FN_kW_SF,												// 672 
	FN_Shunt_A_Current,									    // 673
	FN_Shunt_B_Current,									    // 674
	FN_Shunt_C_Current,									    // 675
	FN_Battery_Voltage,									    // 676
	FN_Battery_Current,									    // 677
	FN_Battery_Temperature,								    // 678
	FN_Status_Flags,									    // 679
	FN_Shunt_A_Accumulated_AH,							    // 680
	FN_Shunt_A_Accumulated_kWh,							    // 681
	FN_Shunt_B_Accumulated_AH,							    // 682
	FN_Shunt_B_Accumulated_kWh,							    // 682
	FN_Shunt_C_Accumulated_AH,							    // 684
	FN_Shunt_C_Accumulated_kWh,							    // 685	
	FN_Input_Current,										// 686
	FN_Output_Current,										// 687
	FN_Input_kW,											// 688
	FN_Output_kW,											// 689
	FN_Net_kW,												// 690	
 	FN_Days_Since_Charge_Parameters_Met,				    // 691
	FN_State_Of_Charge,									    // 692
	FN_Todays_Minimum_SOC,								    // 693
	FN_Todays_Maximum_SOC,								    // 694
	FN_Todays_NET_Input_AH,								    // 695
	FN_Todays_NET_Input_kWh,							    // 696
	FN_Todays_NET_Output_AH,							    // 697
	FN_Todays_NET_Output_kWh,							    // 698	
	FN_Todays_NET_Battery_AH,								// 699
	FN_Todays_NET_Battery_kWh,								// 700	
	FN_Charge_Factor_Corrected_NET_Battery_AH,			    // 701
	FN_Charge_Factor_Corrected_NET_Battery_kWh,			    // 702
	FN_Todays_Minimum_Battery_Voltage,					    // 703
	FN_Todays_Minimum_Battery_Time,						    // 704
	FN_Todays_Maximum_Battery_Voltage,					    // 705
	FN_Todays_Maximum_Battery_Time,						    // 706
	FN_Cycle_Charge_Factor,								    // 707
	FN_Cycle_kWh_Charge_Efficiency,						    // 708
	FN_Total_Days_At_100_Percent,						    // 709
	FN_Lifetime_kAH_Removed,							    // 710
	FN_Shunt_A_Historical_Returned_To_Battery_AH,		    // 711
	FN_Shunt_A_Historical_Returned_To_Battery_kWh,		    // 712
	FN_Shunt_A_Historical_Removed_From_Battery_AH,		    // 713
	FN_Shunt_A_Historical_Removed_From_Battery_kWh,		    // 714
	FN_Shunt_A_Maximum_Charge_Rate,						    // 715
	FN_Shunt_A_Maximum_Charge_Rate_kW,					    // 716
	FN_Shunt_A_Maximum_Discharge_Rate,					    // 717
	FN_Shunt_A_Maximum_Discharge_Rate_kW,				    // 718
	FN_Shunt_B_Historical_Returned_To_Battery_AH,		    // 719
	FN_Shunt_B_Historical_Returned_To_Battery_kWh,		    // 720
	FN_Shunt_B_Historical_Removed_From_Battery_AH,		    // 721
	FN_Shunt_B_Historical_Removed_From_Battery_kWh,		    // 722
	FN_Shunt_B_Maximum_Charge_Rate,						    // 723
	FN_Shunt_B_Maximum_Charge_Rate_kW,					    // 724
	FN_Shunt_B_Maximum_Discharge_Rate,					    // 725
	FN_Shunt_B_Maximum_Discharge_Rate_kW,				    // 726
	FN_Shunt_C_Historical_Returned_To_Battery_AH,		    // 727
	FN_Shunt_C_Historical_Returned_To_Battery_kWh,		    // 728
	FN_Shunt_C_Historical_Removed_From_Battery_AH,		    // 729
	FN_Shunt_C_Historical_Removed_From_Battery_kWh,		    // 730
	FN_Shunt_C_Maximum_Charge_Rate,						    // 731
	FN_Shunt_C_Maximum_Charge_Rate_kW,					    // 732
	FN_Shunt_C_Maximum_Discharge_Rate,					    // 733 
	FN_Shunt_C_Maximum_Discharge_Rate_kW,				    // 734
	FN_Shunt_A_Reset_Maximum_Data,						    // 735
	FN_Shunt_A_Reset_Maximum_Data_Write_Complement,		    // 736
	FN_Shunt_B_Reset_Maximum_Data,						    // 737
	FN_Shunt_B_Reset_Maximum_Data_Write_Complement,		    // 738
	FN_Shunt_C_Reset_Maximum_Data,						    // 739
	FN_Shunt_C_Reset_Maximum_Data_Write_Complement,		    // 740

	OP_stats_DID,											// 741        
 	OP_stats_Length,                                        // 742
 	OP_stats_Bt_min,                                        // 743
 	OP_stats_Bt_max,                                        // 744
 	OP_stats_Bt_ave,                                        // 745
 	OP_stats_Bt_attempts,                                   // 746
 	OP_stats_Bt_errors,                                     // 747
	OP_stats_Bt_timeouts,                                   // 748
 	OP_stats_Bt_packet_timeout,                             // 749
 	OP_stats_Mp_min,                                        // 750
 	OP_stats_Mp_max,                                        // 751
 	OP_stats_Mp_ave,                                        // 752
 	OP_stats_Mp_attempts,                                   // 753
	OP_stats_Mp_errors,                                     // 754
	OP_stats_Mp_timeouts,                                   // 755
	OP_stats_Mp_packet_timeout,                             // 756
	OP_stats_Cu_min,                                        // 757
	OP_stats_Cu_max,                                        // 758
	OP_stats_Cu_ave,                                        // 759
	OP_stats_Cu_attempts,                                   // 760
	OP_stats_Cu_errors,                                     // 761
	OP_stats_Cu_timeouts,                                   // 762
	OP_stats_Cu_packet_timeout,                             // 763
	OP_stats_Su_min,                                        // 764
	OP_stats_Su_max,                                        // 765
	OP_stats_Su_ave,                                        // 766
	OP_stats_Su_attempts,                                   // 767
	OP_stats_Su_errors,                                     // 768
	OP_stats_Su_timeouts,                                   // 769
	OP_stats_Su_packet_timeout,                             // 770
	OP_stats_Pg_min,                                        // 771
	OP_stats_Pg_max,                                        // 772
	OP_stats_Pg_ave,                                        // 773
	OP_stats_Pg_attempts,                                   // 774
	OP_stats_Pg_errors,                                     // 775
	OP_stats_Pg_timeouts,                                   // 776
	OP_stats_Pg_packet_timeout,                             // 777
	OP_stats_Mb_min,                                        // 778
	OP_stats_Mb_max,                                        // 779
	OP_stats_Mb_ave,                                        // 780
	OP_stats_Mb_attempts,                                   // 781
	OP_stats_Mb_errors,                                     // 782
	OP_stats_Mb_timeouts,                                   // 783
	OP_stats_Mb_packet_timeout,                             // 784
	OP_stats_Fu_min,                                        // 785
	OP_stats_Fu_max,                                        // 786
	OP_stats_Fu_ave,                                        // 787
	OP_stats_Fu_attempts,                                   // 788
	OP_stats_Fu_errors,                                     // 789
	OP_stats_Fu_timeouts,                                   // 790
	OP_stats_Fu_packet_timeout,                             // 791
	OP_stats_Ev_min,                                        // 792
	OP_stats_Ev_max,                                        // 793
	OP_stats_Ev_ave,                                        // 794
	OP_stats_Ev_attempts,                                   // 795
	OP_stats_Ev_errors,                                     // 796
	OP_stats_Ev_timeouts,                                   // 797
	OP_stats_Ev_packet_timeout,                             // 798
	
	A_SunSpec_DID,										    // 799 
	A_SunSpec_Length,									    // 800 
	A_Devices,											    // 801
	A_Count,											    // 802 
	A_Update_Number,									    // 803 
	A_Status,											    // 804 
	A_Status_Vendor,									    // 805 
	A_Event,											    // 806 
	A_Event_Vendor,										    // 807 
	A_Control,											    // 808 
	A_Control_Vendor,									    // 809 
	A_Control_Value,									    // 810 
	END_SunSpec_DID,									    // 811 
	END_SunSpec_Length									    // 812 
} SunSpecField;


extern const char* fnames[];

int				outbackInit					(char *, int);
void			outbackClose				(void);
uint16			getBlockType				(int block);
uint64			getField					(int, SunSpecField, char *);
int				setFieldWithString			(int, SunSpecField, char *);
int				setFieldsWithString			(int, SunSpecField, char *);
int				fieldIsInBlock				(SunSpecField, int);
SunSpecField	firstField					(int block);
SunSpecField	lastField					(int block);
int				Get_ModBus_Register_address	(const int block, const int field);


#endif
