import json

__author__ = 'Prafull'

from outback import OutbackInverterClass
from outback.__utils__ import  logger, json

INV_TYPES = {
    # add the types of inverter
    "outback": OutbackInverterClass.OutbackInverterClass,
}


class InverterFactory(object):
    # Create based on class name:
    @staticmethod
    def factory():
        """
        :return:
        """
        with open("inverter.json", 'r') as f:
            inv_type_json = json.load(f)
            logger.info("inverte r type string " + str(inv_type_json))
        if "type" not in inv_type_json:
            assert 0, "Problem with inverter_type file: inverter.json"
        type_inv = inv_type_json["type"]

        if type_inv not in INV_TYPES:
            assert 0, "Bad inverter type creation: " + type_inv
        inv_instance = INV_TYPES[type_inv]
        print type(inv_instance)
        return inv_instance(inv_type_json)
