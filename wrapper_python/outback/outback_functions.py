__author__ = 'Prafull'
# # Time -
# OutBack_Year        #Clock year (4 digit)
# Outback_Month       #Clock Month (1 - 12)
# OutBack_Day         #Clock Day (1 - 31)
# OutBack_Hour        #Clock Hour (0 - 23)
# Outback_Minute      #Clock Minute (0 - 59)
# OutBack_Second      #Clock Second (0 - 59)

## System setpoints
# OB_Bulk_Charge_Enable_Disable = 0   #0=Not used 1=Start Bulk, 2=Stop Bulk, 3=Start EQ Charge, 4= Stop EQ Charge
# OB_Inverter_AC_Drop_Use = 0         # 0=Not used 1=Use, 2=Drop
# OB_Set_Inverter_Mode = 2            #1=Off, 2=Search, 3=On
# OB_Grid_Tie_Mode  = 1              #1=Enable, 2=Disable
# OB_Set_Inverter_Charger_Mode  = 2   #Auto set to 2 when grid tie mode is enabled  #1=Off, 2=Auto, 3=On
# #TODO: look into registers to specify PV array wattage, Generator kW rating and type, Maximume inverter kW rating,

functions = {
    "time":{
        "hour": "OutBack_Hour",
        "min": "OutBack_Minute",
        "sec": "OutBack_Second",
    },
    "date": {
        "year": "OutBack_Year",
        "mon": "OutBack_Month",
        "day": "OutBack_Day",
    },

    "system_setpoints":{
        ""
    }


}