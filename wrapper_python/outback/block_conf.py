__author__ = 'Prafull'


blocks_ind = [0,1,2,3,4,5,6,7,8,9]

COMMON_MODEL_BLOCK = 0
OUTBACK_BLOCK = 1
SYSTEM_CONTROL_BLOCK = 2
SINGLE_PHASE_INV_BLOCK = 3
PACKET_STATISTICS_BLOCK = 4
RADIAN_SINGLE_PHASE_INV = 5
RADIAN_INV_CONF_BLOCK = 6
BATTERY_MONITOR_BLOCK = 9
BATTERY_MONITOR_CONF_BLOCK = 10
FMCHARGE_CONTROLLER_BLOCK = 8
BASIC_CHARGE_CONTROLLER_BLOCK = 7
END_BLOCK = 11

blocks_name = {

    COMMON_MODEL_BLOCK : "SunSpec Common Model Block",
    OUTBACK_BLOCK : "OutBack OutBack Block",
    SYSTEM_CONTROL_BLOCK: "OutBack System Control",
    SINGLE_PHASE_INV_BLOCK: "Single Phase Inverter",
    PACKET_STATISTICS_BLOCK: "OutBack OPTICS Packet Statistics ",
    RADIAN_SINGLE_PHASE_INV : "OutBack Radian Single Phase Inverter",
    RADIAN_INV_CONF_BLOCK : "OutBack Radian Inverter Configuration",
    BATTERY_MONITOR_BLOCK : "OutBack FLEXnet-DC Battery Monitor",
    FMCHARGE_CONTROLLER_BLOCK:"OutBack FM Charge Controller",
    BASIC_CHARGE_CONTROLLER_BLOCK :"Basic Charge Controller ",
    BATTERY_MONITOR_CONF_BLOCK : "OutBack FLEXnet-DC Battery Monitor Configuration",
    END_BLOCK: "SunSpec End Block"
}

