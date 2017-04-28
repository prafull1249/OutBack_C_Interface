import InverterFactory
import InverterFactory as ifact
ifac = ifact.InverterFactory()
inv = ifac.factory()

reg_names = ["I_AC_Power", "I_AC_Current", "I_AC_VoltageAN","CC_Charger_State", "CC_Batt_Current", "CC_Watts", "I_AC_Frequency", "I_AC_VAR"]
#OutBack_Hour        #Clock Hour (0 - 23)
#Outback_Minute      #Clock Minute (0 - 59)
#OutBack_Second      #Clock Second (0 - 59)

#inv.read_multiple(reg_names)


def write_reg(field, block, value):

	print "*" * 80
	inv.read_single(field, block)
	inv.write_single(field, block, value)
	print "*" * 80
	inv.read_single(field, block)

write_reg = ["OutBack_Grid_Use_Interval_1_Mode","CCconfig_MPPT_Mode","GSconfig_Grid_Tie_Enable","GSconfig_AC_Output_Voltage"]

values = [1, 0, 0, 117]
flip_values = [0,1,1,120]

inv.read_multiple(write_reg)
inv.write_multiple(write_reg, values)
inv.read_multiple(write_reg)
inv.write_multiple(write_reg, flip_values)
inv.read_multiple(write_reg)


#write_reg(366, 6 , 30)
