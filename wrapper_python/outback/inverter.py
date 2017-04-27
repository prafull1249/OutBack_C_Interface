class Inverter:

    class __Inverter:
        def __init__(self, inv_type_json):
            self.type = "outback"

        def __str__(self):
            return repr(self) + "Inverter - " + self.type

    instance = None

    def __init__(self, arg):
        if not Inverter.instance:
            Inverter.instance = Inverter.__Inverter(arg)
        else:
            Inverter.instance.val = arg

    def __getattr__(self, name):
        return getattr(self.instance, name)

    def set_time(self, dd=None,mm=None, yy=None):
        pass

    def set_date(self, dd=None,mm=None, yy=None):
        pass

    def set_system_setpoints(self):
        pass

    def set_battery_setpoints(self):
        pass

    def set_inverter_setpoints(self):
        pass

    def set_chargecontroller_setpoints(self):
        pass

    def set_mate3_settings(self):
        pass

    def set_batterymonitor_setpoints(self):
        pass

    def record_inverter_data(self):
        pass

    def record_system_data(self):
        pass

    def record_suspec_data(self):
        pass

    def record_battery_data(self):
        pass

    def read_single_reg(self, dict_fields):
        pass
