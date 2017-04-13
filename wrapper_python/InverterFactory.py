import json

__author__ = 'Prafull'

from inverter import Inverter
from outback import OutbackInverter

INV_TYPES = {
    # add the types of inverter
    "outback": OutbackInverter,
}

class InverterFactory(object):
    # Create based on class name:
    def factory():
        """

        :return:
        """
        with open("inverter.json", 'r') as f:
            inv_type_json = json.load(f)
        if "type" not in inv_type_json:
            assert 0, "Problem with inverter_type file: " + type
        type = inv_type_json["type"]

        if type not in INV_TYPES:
            assert 0, "Bad inverter type creation: " + type
        inv_instance = INV_TYPES.get(type)
        return inv_instance(inv_type_json)

    factory = staticmethod(factory)
