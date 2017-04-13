
 #TODO: Find registers to set installer information/ settings and user access rights
set_outback_time()

OutBack_Year        #Clock year (4 digit)
Outback_Month       #Clock Month (1 - 12)
OutBack_Day         #Clock Day (1 - 31)
OutBack_Hour        #Clock Hour (0 - 23)
Outback_Minute      #Clock Minute (0 - 59)
OutBack_Second      #Clock Second (0 - 59)

set_system_setpoints()

OB_Bulk_Charge_Enable_Disable = 0   #0=Not used 1=Start Bulk, 2=Stop Bulk, 3=Start EQ Charge, 4= Stop EQ Charge
OB_Inverter_AC_Drop_Use = 0         # 0=Not used 1=Use, 2=Drop
OB_Set_Inverter_Mode = 2            #1=Off, 2=Search, 3=On
OB_Grid_Tie_Mode  = 1              #1=Enable, 2=Disable
OB_Set_Inverter_Charger_Mode  = 2   #Auto set to 2 when grid tie mode is enabled  #1=Off, 2=Auto, 3=On
#TODO: look into registers to specify PV array wattage, Generator kW rating and type, Maximume inverter kW rating,

set_battery_setpoints():

FNconfig_Battery_Capacity = 120     #AH- set this value to the capacity of the battery bank
FNconfig_Charged_Volts    = 55.2    #V -dependent on the specifc battery bank used
FNconfig_Battery_Charged_Amps = 0.02*FNconfig_Battery_Capacity   #Can vary between 1.5% and 2% of battery capacity depending on life and condition of bank
FNconfig_Charged_Time = 1       #minutes- time voltage and amps must be met to consider batteries charged
#TODO: Find register to set nominal voltage of battery bank
#If needed outback has the implimentation of scaling factors to adjust these nominal values

set_inverter_setpoints():

#search circuit for detecting loads
GSconfig_Search_Sensitivity = 30    #setting to 0 will disable search
GSconfig_Search_Pulse_Length = 8
GSconfig_Search_Pulse_Spacing = 60
#larger length or spacing will meet loads faster but consume more energy

#AC Input
GSconfig_AC_Input_Select_Priority = 0   #0=Grid, 1=Gen
GSconfig_Grid_AC_Input_Current_Limit = 60       #Can be modified/ reduced to minimize kW demand from the grid
GSconfig_Gen_AC_Input_Current_Limit = 43        #Adjust inverter draw size to size of gen circuit
GSconfig_AC_Coupled = 0                     #0=No, 1=Yes (not implemented)


#Grid Input
GSconfig_Grid_Input_Mode = 2                #Grid Input Mode: 0=Generator, 1=Support, 2=Grid Tied, 3=UPS, 4=Backup, 5=Mini Grid, 6=Grid Zero
GSconfig_Grid_Lower_Input_Voltage_Limit = 108       #V- Grid Input AC voltage lower limit
GSconfig_Grid_Upper_Input_Voltage_Limit = 132       #V- Grid Input AC voltage upper limit
GSconfig_Grid_Transfer_Delay = 1000          #msecs - limit on duration that input AC voltage or frequency may exceed limits before disconnect
GSconfig_Grid_Connect_Delay = 0.2            #min - time before inverter begins accepting power from source

#Specific operating mode set points
GSconfig_Mini_Grid_LBX_Volts = 48.0     #V-Mini Grid LBX reconnect to Grid Battery Voltage
GSconfig_Mini_Grid_LBX_Delay = 1.0      #hours- Mini Grid LBX reconnect to Grid Delay Time
GSconfig_Grid_Zero_DoD_Volts = 50.0     #V- Grid Zero DoD Voltage
GSconfig_Grid_Zero_DoD_Max_Offset_AC_Amps = 12.0  #amps- Grid Zero Maximum Offset AC Amps
GSconfig_Grid_Tie_Window = 0        #0=IEEE, 1=User (GS8048 Only)
GSconfig_Grid_Tie_Enable = 1        #1=Yes, 0=No
#GSconfig_Stacking_Mode = 10         #READ ONLY!!!- 10=Master, 12=Slave, 17=B Phase Master, 18=C Phase Master
GSconfig_Master_Power_Save_Level = 0        #Master inverter power save level
GSconfig_Slave_Power_Save_Level = 1         #Slave inverter power save level


#Generator Input
GSconfig_Gen_Input_Mode = 0             #Grid Input Mode: 0=Generator, 1=Support, 2=Grid Tied, 3=UPS, 4=Backup, 5=Mini Grid, 6=Grid Zero
GSconfig_Gen_Lower_Input_Voltage_Limit = 108            #V- Gen Input AC voltage lower limit
GSconfig_Gen_Upper_Input_Voltage_Limit = 140            #V- Gen Input AC voltage upper limit
GSconfig_Gen_Transfer_Delay = 1000          #msecs - limit on duration that input AC voltage or frequency may exceed limits before disconnect
GSconfig_Gen_Connect_Delay = 0.2            #min - time before inverter begins accepting power from source

#AC Output
GSconfig_AC_Output_Voltage = 120        #V

#Battery/ Charging
GSconfig_Charger_AC_Input_Current_Limit = 16    #Used when inverter charging is enabled
GSconfig_Charger_Operating_Mode = 0         #0=All Inverter Charging Disabled, 1=Bulk and Float Charging Enabled
GSconfig_Absorb_Volts = 57.6            #V- Absorb and Bulk Voltage Target
GSconfig_Absorb_Time_Hours = 1.1        #hours- Absorb Time Hours
GSconfig_Float_Volts = 54.4             #V- Float Voltage Target
GSconfig_Float_Time_Hours = 1.0         #hours- Float Time Hours
GSconfig_ReFloat_Volts = 50.0           #V- ReFloat Voltage Target
GSconfig_EQ_Volts = 44.0                #V- EQ Voltage Target
GSconfig_EQ_Time_Hours= 3.0             #hours- EQ Time Hours
GSconfig_Low_Battery_Cut_Out_Voltage = 47.6     #V- Low Battery Voltage Cut Out
GSconfig_Low_Battery_Cut_In_Voltage = 48.8      #V- Low Battery Voltage Cut In
GSconfig_Sell_Volts = 48                        #V - Sell Voltage Target
GSconfig_ReBulk_Volts = 54.4                    #V- ReBulk Voltage Target
GSconfig_Low_Battery_Cut_Out_Delay = 6411.8     #secs- Seconds delay before inverter shutdown upon low battery voltage
GSconfig_High_Battery_Cut_Out_Voltage = 7.6     #V- High Battery Voltage Cut Out
GSconfig_High_Battery_Cut_In_Voltage = 0.3      #V- High Battery Voltage Cut In
GSconfig_High_Battery_Cut_Out_Delay = 0         #Seconds delay before inverter shutdown upon high battery voltage

#AUX
#TODO

set_chargecontroller_setpoints():
#Charging set points (also set in inverter setpoints)
#Since the inverter sells power to maintain its own Float, Absorption, or Sell settings (all of which should be lower than those of the controller), this mode makes it easier for the inverter to sell power.
CCconfig_Absorb_Volts           #Absorb Voltage Target
CCconfig_Absorb_Time_Hours      #Absorb Time Hours
CCconfig_Absorb_End_Amps        #Amperage to end Absorbing
CCconfig_Rebulk_Volts           #Voltage to re-initiate Bulk charge
CCconfig_Float_Volts            #Float Voltage Target
CCconfig_Bulk_Current           #Max Output Current Limit
CCconfig_EQ_Volts               #Target Voltage for Equalize
CCconfig_EQ_Time_Hours          #EQ Time Hours
CCconfig_Auto_EQ_Days           #Auto EQ Interval Days

#MPPT
CCconfig_MPPT_Mode = 0          #0 = Auto; 1 = U-Pick
CCconfig_Wakeup_VOC
CCconfig_Wakeup_Interval
CCconfig_Snooze_Mode_Amps
#TODO: Need the active registers (Registers in AllOutput.txt) to find rest of set points
CCconfig_Grid_Tie_Mode = 1      #0 = Grid Tie Mode disabled; 1 = Grid Tie Mode enabled
#TODO: Auxillary Output of CC

set_batterymonitor_setpoints():
#when parameters are met the SOC will be consider 100%
FNconfig_Battery_Capacity = 100         #Ah- Battery AH capacity
FNconfig_Charged_Volts = 55.2           #V-  Sets the minimum voltage the three-stage charger must reach during the Bulk or Absorption stages for the battery monitor to consider the batteries fully charged.
FNconfig_Charged_Time = 3               #min -Sets the duration the Charged Voltage and Charged Return Amps must be maintained before the charging cycle is considered finished.
FNconfig_Battery_Charged_Amps = 2.0     #A- Sets the limit to which the charging current must “trickle down” or decrease before the batteries are considered charged.
FNconfig_Charge_Factor = 94             #%- Adjusts the anticipated charging efficiency of the batteries.  Because the batteries cannot be 100% efficient, the battery monitor discounts a certain percentage of the energy used to charge them.
FNconfig_Shunt_A_Enabled = 0            #0=Enabled, 1=Disabled
FNconfig_Shunt_B_Enabled = 0            #0=Enabled, 1=Disabled
FNconfig_Shunt_C_Enabled = 1            #0=Enabled, 1=Disabled
FNconfig_Relay_Control = 0              #0 = Off; 1 = Auto; 2 = On
#TODO: Relay Mode to switch or turn other devices on or off

set_mate3_settings():
#TODO: Advacned gnerator start (AGS) Setup
#TODO: Data logging functionality (possible recording onto external memory device)
#TODO: Possible set up of HBX mode if trying to minimize grid usage
#Grid Use Time
#Will not work with HBX or Mini Grid modes, If the battery voltage falls below the inverter’s Low Battery Cut-Out voltage, the inverter will automatically connect to the AC input source regardless of the time-of-day setting)
#Only one Grid Use Time may be programmed on a weekend.  Three Grid Use Time periods may be programmed on weekdays.
#Min = 0-59, Hour = 0-23
OutBack_Grid_Use_Interval_1_Mode = 1        #0=Disabled, 1=Enabled
OutBack_Grid_Use_Interval_1_Weekday_Start_Hour = 0
OutBack_Grid_Use_Interval_1_Weekday_Start_Minute = 0
OutBack_Grid_Use_Interval_1_Weekday_Stop_Hour = 13
OutBack_Grid_Use_Interval_1_Weekday_Stop_Minute = 0
OutBack_Grid_Use_Interval_1_Weekend_Start_Hour = 0
OutBack_Grid_Use_Interval_1_Weekend_Start_Minute = 0
OutBack_Grid_Use_Interval_1_Weekend_Stop_Hour = 0
OutBack_Grid_Use_Interval_1_Weekend_Stop_Minute = 0
OutBack_Grid_Use_Interval_2_Mode = 0        #0=Disabled, 1=Enabled
OutBack_Grid_Use_Interval_2_Weekday_Start_Hour = 0
OutBack_Grid_Use_Interval_2_Weekday_Start_Minute = 0
OutBack_Grid_Use_Interval_2_Weekday_Stop_Hour = 0
OutBack_Grid_Use_Interval_2_Weekday_Stop_Minute = 0
OutBack_Grid_Use_Interval_3_Mode = 0        #0=Disabled, 1=Enabled
OutBack_Grid_Use_Interval_3_Weekday_Start_Hour = 0
OutBack_Grid_Use_Interval_3_Weekday_Start_Minute = 0
OutBack_Grid_Use_Interval_3_Weekday_Stop_Hour = 0
OutBack_Grid_Use_Interval_3_Weekday_Stop_Minute = 0

#Global Charege Control
OutBack_Global_Charger_Control_Mode = 0     #0=Disabled, 1=Enabled
OutBack_Global_Charger_Control_Output_Limit = 80        #Amps - 10 to 800 amps
OutBack_Enable_Float_Coordination = 0       #0=Disabled, 1=Enabled

#TODO: Flexnet DC Advanced Control (Can not find registers to recod warnings for SOC)
OutBack_Enable_FNDC_Charge_Termination = 1      #0=Disabled, 1=Enabled
OutBack_Enable_FNDC_Grid_Tie_Control = 1        #0=Disabled, 1=Enabled


record_sunspec_data():
#Sunspec registers for inverter
#DC (In)
I_DC_Current
I_DC_Voltage
I_DC_Power
#AC(Out)
I_AC_Power
I_AC_Voltage_AN
I_AC_Current
I_AC_CurrentA
I_AC_Frequency
I_AC_VAR
I_AC_VA
I_AC_Energy_WH

record_system_data():
#System
OutBack_Error
OutBack_Status
OutBack_System_Voltage              #12, 24, 26, 48 or 60 VDC
OutBack_Measured_System_Voltage     #Current system battery voltage computed by gateway

record_battery_data():
#Battery/ Charge Controller
CC_Batt_Voltage
CC_Array_Voltage
CC_Batt_Current
CC_Array_Current
CC_Charger_State        #If possible
CC_Watts
CC_Todays_Min_Battery_Volts
CC_Todays_Max_Battery_Volts
CC_VOC
CC_Todays_Peak_VOC
CC_Todays_kWH
CC_Todays_AH
CC_Lifetime_kWH_Hours
CC_Lifetime_kAmp_Hours
CC_Lifetime_Max_Watts
CC_Lifetime_Max_Battery_Volts
CC_Lifetime_Max_VOC

# FM Charge controller
CCconfig_Get_Current_Data_Log_Day_Offset
CCconfig_Data_Log_Daily_AH
CCconfig_Data_Log_Daily_kWH
CCconfig_Data_Log_Daily_Max_Output_Amps
CCconfig_Data_Log_Daily_Max_Output_Watts
CCconfig_Data_Log_Daily_Absorb_Time
CCconfig_Data_Log_Daily_Float_Time
CCconfig_Data_Log_Daily_Min_Batt_Volts
CCconfig_Data_Log_Daily_Max_Batt_Volts
CCconfig_Data_Log_Daily_Max_Input_Volts

#FlexNet Battery Monitor
FNconfig_Datalog_Minimum_SOC
FNconfig_Datalog_Input_AH
FNconfig_Datalog_Input_kWh
FNconfig_Datalog_Output_AH
FNconfig_Datalog_Output_kWh
FNconfig_Datalog_NET_AH
FNconfig_Datalog_NET_kWh

FN_Shunt_A_Current
FN_Shunt_B_Current
FN_Shunt_C_Current
FN_Battery_Voltage
FN_Battery_Current
FN_Battery_Temperature
FN_Status_Flags
FN_Shunt_A_Accumulated_AH
FN_Shunt_A_Accumulated_kWh
FN_Shunt_B_Accumulated_AH
FN_Shunt_B_Accumulated_kWh
FN_Shunt_C_Accumulated_AH
FN_Shunt_C_Accumulated_kWh
FN_Input_Current
FN_Output_Current
FN_Input_kW
FN_Output_kW
FN_Net_kW
FN_Days_Since_Charge_Parameters_Met
FN_State_Of_Charge
FN_Todays_Minimum_SOC
FN_Todays_Maximum_SOC
FN_Todays_NET_Input_AH
FN_Todays_NET_Input_kWh
FN_Todays_NET_Output_AH
FN_Todays_NET_Output_kWh
FN_Todays_NET_Battery_AH
FN_Todays_NET_Battery_kWh
FN_Charge_Factor_Corrected_NET_Battery_AH
FN_Charge_Factor_Corrected_NET_Battery_kWh
FN_Todays_Minimum_Battery_Voltage
FN_Todays_Minimum_Battery_Time
FN_Todays_Maximum_Battery_Voltage
FN_Todays_Maximum_Battery_Time
FN_Cycle_Charge_Factor
FN_Cycle_kWh_Charge_Efficiency
FN_Total_Days_At_100_Percent
FN_Lifetime_kAH_Removed
FN_Shunt_A_Historical_Returned_To_Battery_AH
FN_Shunt_A_Historical_Returned_To_Battery_kWh
FN_Shunt_A_Historical_Removed_From_Battery_AH
FN_Shunt_A_Historical_Removed_From_Battery_kWh
FN_Shunt_A_Maximum_Charge_Rate
FN_Shunt_A_Maximum_Charge_Rate_kW
FN_Shunt_A_Maximum_Discharge_Rate
FN_Shunt_A_Maximum_Discharge_Rate_kW
FN_Shunt_B_Historical_Returned_To_Battery_AH
FN_Shunt_B_Historical_Returned_To_Battery_kWh
FN_Shunt_B_Historical_Removed_From_Battery_AH
FN_Shunt_B_Historical_Removed_From_Battery_kWh
FN_Shunt_B_Maximum_Charge_Rate
FN_Shunt_B_Maximum_Charge_Rate_kW
FN_Shunt_B_Maximum_Discharge_Rate
FN_Shunt_B_Maximum_Discharge_Rate_kW
FN_Shunt_C_Historical_Returned_To_Battery_AH
FN_Shunt_C_Historical_Returned_To_Battery_kWh
FN_Shunt_C_Historical_Removed_From_Battery_AH
FN_Shunt_C_Historical_Removed_From_Battery_kWh
FN_Shunt_C_Maximum_Charge_Rate
FN_Shunt_C_Maximum_Charge_Rate_kW
FN_Shunt_C_Maximum_Discharge_Rate
FN_Shunt_C_Maximum_Discharge_Rate_kW

record_inverter_data():
#Inverter
GSconfig_Stacking_Mode
GS_Single_Inverter_Output_Current
GS_Single_Inverter_Charge_Current
GS_Single_Inverter_Buy_Current
GS_Single_Inverter_Sell_Current
GS_Single_Grid_Input_AC_Voltage
GS_Single_Gen_Input_AC_Voltage
GS_Single_Output_AC_Voltage
GS_Single_Inverter_Operating_mode
GS_Single_Error_Flags
GS_Single_Warning_Flags
GS_Single_Battery_Voltage
GS_Single_Temp_Compensated_Target_Voltage
GS_Single_AUX_Output_State
GS_Single_AUX_Relay_Output_State
GS_Single_L_Module_Transformer_Temperature
GS_Single_L_Module_Capacitor_Temperature
GS_Single_L_Module_FET_Temperature
GS_Single_R_Module_Transformer_Temperature
GS_Single_R_Module_Capacitor_Temperature
GS_Single_R_Module_FET_Temperature
GS_Single_Battery_Temperature
GS_Single_AC_Input_Selection
GS_Single_AC_Input_Frequency
GS_Single_AC_Input_Voltage
GS_Single_AC_Input_State
GS_Single_Minimum_AC_Input_Voltage
GS_Single_Maximum_AC_Input_Voltage
GS_Single_Sell_Status
GS_Single_kWh_SF
GS_Single_AC1_Buy_kWh
GS_Single_AC2_Buy_kWh
GS_Single_AC1_Sell_kWh
GS_Single_AC2_Sell_kWh
GS_Single_Output_kWh
GS_Single_Charger_kWh
GS_Single_Output_kW
GS_Single_Buy_kW
GS_Single_Sell_kW
GS_Single_Charge_kW
GS_Single_Load_kW
GS_Single_AC_Couple_kW



