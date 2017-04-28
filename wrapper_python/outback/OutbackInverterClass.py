# coding=utf-8
__author__ = 'Prafull'

import register_bank
import blocks
import inverter
from outback_register import register
from __utils__ import logger, json


class OutbackInverterClass(inverter.Inverter):

    def __init__(self, inv_json=""):
        inverter.Inverter.__init__(self, inv_json)
        try:
            logger.info("inv json {} ".format(inv_json))
            self.type = inv_json["type"]
            self.ip = inv_json["ip"]
        except KeyError:
            logger.error("inv_json {} is not valid".format(inv_json))
            return
        self.block = self.get_register_bank()
        self.registers = self.get_register_bank()
        self.blocks = self.get_blocks()
        self.blocks_names = self.get_block_names()

    def get_blocks(self):
        return blocks.blocks

    def get_register(self, name):
        """
        Get register from the register bank which has all its details.
        :param name:
        :return:
        """
        if name in self.registers:
            return self.registers[name]
        logger.error("Invalid register name {}")

    def get_block_names(self):
        return blocks.blocks_name

    def get_block(self, index):
        """
        Function to return block registers by first getting index, its name and then its registers.
        :param index:
        :return:
        """
        name = ""
        try:
            index = int(index)
        except ValueError:
            return blocks.blocks[index]

        if index in blocks.blocks_name:
            name = blocks.blocks_name[index]
        else:
            logger.error("No index {} in blocks_name".format(index))

        if name in blocks.blocks:
            return blocks.blocks[name]
        logger.error("No {} in blocks.blocks".format(name))

    def read(self, name):
        """
        Read register of Outback
        :param name:
        :return:
        """
        reg = self.get_register(name)
        if reg is not None:
            value = reg.read_reg(self.ip)
            if value is not None:
                return value
            logger.error("Read register {} failed".format(name))
        else:
            logger.error("Invalid Register {}".format(name))

    def write_multiple(self, reg_names, values):
        """
        Read multiple register of Outback
        :param name:
        :return:
        """
	fields = []
	blocks = []
	for name in reg_names:
	        reg = self.get_register(name)
		if reg is not None:
	       	 	fields.append(reg.field)
			blocks.append(reg.block)
		else:
			logger.error(name + " not present")
	return register.write_multiple_reg(self.ip, fields, blocks, values)

    def read_multiple(self, reg_names):
        """
        Read multiple register of Outback
        :param name:
        :return:
        """
	fields = []
	blocks = []
	for name in reg_names:
	        reg = self.get_register(name)
		if reg is not None:
	       	 	fields.append(reg.field)
			blocks.append(reg.block)
		else:
			logger.error(name + " not present")
	return register.read_multiple_reg(self.ip, fields, blocks)

    def read_single(self, field, block):
        """
        Read register of Outback
        :param name:
        :return:
        """
        name = "custom_read"
        reg = register(name, field, block)
        if reg is not None:
            value = reg.read_reg(self.ip)
            if value is not None:
                return value
            logger.error("Read register {} failed".format(name))
        else:
            logger.error("Invalid Register {}".format(name))

    def write_single(self, field, block, value):
        """
        Read register of Outback
        :param name:
        :return:
        """
        name = "custom_read"
        reg = register(name, field, block)
        if reg is not None:
            value = reg.write_reg(value, self.ip)
            if value is not None:
                return value
            logger.error("write register {} failed".format(name))
        else:
            logger.error("Invalid Register {}".format(name))


    def get_register_bank(self):
        """
        Returns register bank for Outback
        :return:
        """
        return register_bank.register_bank

    def write(self, name, value):
        """
        Write to given register
        :param name:
        :param value:
        :return:
        """
        reg = self.get_register(name)
        if reg is not None:
            if reg.write_reg(self.ip, value) is not None:
                 return True
            logger.error("Write register {} failed".format(name))
        else:
            logger.error("Invalid Register {}".format(name))
        return False

    def set_setpoints(self, arg_dict):
        """

        :param arg_dict:
        :return:
        """
        flag = True
        for reg_name in arg_dict:
            if not self.write(reg_name, arg_dict.get(reg_name)):
                logger.error("{} register write failed".format(reg_name))
                flag = False
        return flag

    def set_date(self, dd=None, mm=None, yy =None):
        """
        Set date on Outback Inverter. Write registers
            OutBack_Year        #Clock year (4 digit)
            Outback_Month       #Clock Month (1 - 12)
            OutBack_Day         #Clock Day (1 - 31)
        :param dd:
        :param mm:
        :param yy:
        :return:
        """
        date_reg = {
            "OutBack_Year":yy,
            "Outback_Month":mm,
            "Outback_Day":dd
        }
        return self.set_setpoints(date_reg)

    def set_time(self, h=None, min=None, sec =None):
        """
        Set date on Outback Inverter. Write registers
            OutBack_Hour        #Clock Hour (0 - 23)
            Outback_Minute      #Clock Minute (0 - 59)
            OutBack_Second      #Clock Second (0 - 59)
        :param h:
        :param min:
        :param sec:
        :return:
        """
        time_reg = {
            "OutBack_Hour":h,
            "Outback_Minute":min,
            "Outback_Second":sec
        }
        return self.set_setpoints(time_reg)

    def set_system_setpoints(self):
        """
        Write the following registers as follows -

        OB_Bulk_Charge_Enable_Disable = 0   #0=Not used 1=Start Bulk, 2=Stop Bulk, 3=Start EQ Charge, 4= Stop EQ Charge
        OB_Inverter_AC_Drop_Use = 0         # 0=Not used 1=Use, 2=Drop
        OB_Set_Inverter_Mode = 2            #1=Off, 2=Search, 3=On
        OB_Grid_Tie_Mode  = 1              #1=Enable, 2=Disable
        OB_Set_Inverter_Charger_Mode  = 2   #Auto set to 2 when grid tie mode is enabled  #1=Off, 2=Auto, 3=On
        :return:
        """
        system_regs = {
            "OB_Bulk_Charge_Enable_Disable": 0,
            "OB_Inverter_AC_Drop_Use": 0,
            "OB_Set_Inverter_Mode": 2,
            "OB_Grid_Tie_Mode": 1,
            "OB_Set_Inverter_Charger_Mode": 2,
        }
        return self.set_setpoints(system_regs)

    def set_battery_setpoints(self):
        """
        Write the registers to initialize battery settings -
            FNconfig_Battery_Capacity = 120     #AH- set this value to the capacity of the battery bank
            FNconfig_Charged_Volts    = 55.2    #V -dependent on the specifc battery bank used
            FNconfig_Battery_Charged_Amps = 0.02*FNconfig_Battery_Capacity   #Can vary between 1.5% and 2% of battery capacity depending on life and condition of bank
            FNconfig_Charged_Time = 1       #minutes- time voltage and amps must be met to consider batteries charged
            #TODO: Find register to set nominal voltage of battery bank
        :return:
        """
        FNconfig_Battery_Capacity = 120
        battery_regs = {
           "FNconfig_Battery_Capacity" : 120,
            "FNconfig_Charged_Volts": 55.2,
            "FNconfig_Battery_Charged_Amps" : 0.02*FNconfig_Battery_Capacity,
            "FNconfig_Charged_Time": 1,
        }

        return self.set_setpoints(battery_regs)

    def set_chargecontroller_setpoints(self):
        """
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
        """
        chargecontroller_regs = {
            "CCconfig_MPPT_Mode" : 0,
            "CCconfig_Grid_Tie_Mode": 1,
            #TODO remaining registers values to set is not available right now.
        }

        return self.set_setpoints(chargecontroller_regs)

    def set_batterymonitor_setpoints(self):
        """
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

        :return:
        """
        batterymonitor_setpoints = {
            "FNconfig_Battery_Capacity" :100,
            "FNconfig_Charged_Volts" : 55.2,
            "FNconfig_Charged_Time" : 3,
            "FNconfig_Battery_Charged_Amps":2.0,
            "FNconfig_Charge_Factor" :94,
            "FNconfig_Shunt_A_Enabled" : 0,
            "FNconfig_Shunt_B_Enabled" : 0,
            "FNconfig_Shunt_C_Enabled" : 1,
            "FNconfig_Relay_Control" : 0,

        }
        return self.set_setpoints(batterymonitor_setpoints)

    def set_mate3_settings(self):
        """
        #TODO: Advacned gnerator start (AGS) Setup
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

        :return:
        """
        batterymonitor_setpoints = {
            "OutBack_Enable_FNDC_Grid_Tie_Control" :1,
            "OutBack_Enable_FNDC_Charge_Termination" : 55.2,
            "OutBack_Enable_Float_Coordination" : 0,
            "OutBack_Global_Charger_Control_Output_Limit":80,
            "OutBack_Global_Charger_Control_Mode" :0,
            "OutBack_Grid_Use_Interval_3_Weekday_Stop_Minute" : 0,
            "OutBack_Grid_Use_Interval_3_Weekday_Stop_Hour" : 0,
            "OutBack_Grid_Use_Interval_3_Weekday_Start_Minute" : 1,
            "OutBack_Grid_Use_Interval_3_Weekday_Start_Hour" : 0,
            "OutBack_Grid_Use_Interval_3_Mode" : 0,
            "OutBack_Grid_Use_Interval_2_Weekday_Stop_Minute" : 0,
            "OutBack_Grid_Use_Interval_2_Weekday_Stop_Hour" : 0,
            "OutBack_Grid_Use_Interval_2_Weekday_Start_Minute" : 0,
            "OutBack_Grid_Use_Interval_2_Weekday_Start_Hour" : 0,
            "OutBack_Grid_Use_Interval_2_Mode" : 0,
            "OutBack_Grid_Use_Interval_1_Mode ": 1,        #0=Disabled, 1=Enabled
            "OutBack_Grid_Use_Interval_1_Weekday_Start_Hour": 0,
            "OutBack_Grid_Use_Interval_1_Weekday_Start_Minute" : 0,
            "OutBack_Grid_Use_Interval_1_Weekday_Stop_Hour": 13,
            "OutBack_Grid_Use_Interval_1_Weekday_Stop_Minute" : 0,
            "OutBack_Grid_Use_Interval_1_Weekend_Start_Hour" : 0,
            "OutBack_Grid_Use_Interval_1_Weekend_Start_Minute" : 0,
            "OutBack_Grid_Use_Interval_1_Weekend_Stop_Hour" : 0,
        }
        return self.set_setpoints(batterymonitor_setpoints)

    def record_data(self, regs_list):
        """
        Utility function to read a list of register at once.
        :param regs_list:
        :return:
        """
        read_values = {}
        for reg_name in regs_list:
            read_values[reg_name] = self.read(reg_name)
        return json.dumps(read_values)

    def record_suspec_data(self):
        """

        :return:
        """
        sunspec_data_regs = [
                "I_DC_Current",
                "I_DC_Voltage",
                "I_DC_Power",
                "I_AC_Power",
                "I_AC_Voltage_AN",
                "I_AC_Current",
                "I_AC_CurrentA",
                "I_AC_Frequency",
                "I_AC_VAR",
                "I_AC_VA",
                "I_AC_Energy_WH" ]

        return self.record_data(sunspec_data_regs)

    def record_system_data(self):
        """
        read the registers specific to system data
        :return:
        """
        system_regs = [
            "OutBack_Error",
            "OutBack_Status",
            "OutBack_System_Voltage",
            "OutBack_Measured_System_Voltage"]

        return self.record_data(system_regs)

    def read_single_reg(self, dict_fields):

        if "field" in dict_fields:
            field = dict_fields["field"]
        else:
            logger.error("invalid register fields")
            return -1
        if "block" in dict_fields:
            block= dict_fields["block"]
        else:
            logger.error("invalid register fields")
            return -1

        return self.read_single(field, block)


    def write_single_reg(self, dict_fields):

        if "field" in dict_fields:
            field = dict_fields["field"]
        else:
            logger.error("invalid register fields")
            return -1
        if "block" in dict_fields:
            block= dict_fields["block"]
        else:
            logger.error("invalid register fields")
            return -1

        if "value" in dict_fields:
            value = dict_fields["value"]
        else:
            logger.error("invalid register fields")
            return -1

        return self.write_single(field, block, value)
