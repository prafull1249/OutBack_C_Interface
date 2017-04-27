__author__ = 'Prafull'

from __utils__ import *
class register:
    """
    "Usage: outback [rwa] -c ip [-p port] -f field -b block -v value\n"

    Register class which has all the fields and block related to a
    """

    def __init__(self, name, field, block, unit=""):
        self.field = field
        self.block = block
        self.name = name
        self.unit = unit

    def read_reg(self, ip, port=""):
        result = subprocess.Popen(["outback", "-r", "-c", ip, "-f", self.field, "-b", self.block]
                                  , stdout=subprocess.PIPE)
        out, err = result.communicate()
        logger.info("Output " + out +  " error " + err)
        #TODO check error handling to check if register read fails.
        return out

    def write_reg(self, value, ip, port=""):
        result = subprocess.Popen(["outback", "-w", "-c", ip, "-f", self.field, "-b", self.block, "-v", value]
                                  , stdout=subprocess.PIPE)
        out, err = result.communicate()
        logger.info("Output " + out +  " error " + err)
        #TODO check error handling to check if register write fails.
        return out

    @staticmethod
    def read_multiple_reg(self, ip, fields, blocks):
        """
        Reads multiple registers.
        :param fields: a comma separated string of register field values
        :param blocks: a comma separated string of register block values
        :return:
        """
        fields_el = fields.split(',')
        blocks_el = blocks.split(',')

        if fields_el != blocks_el:
            logger.debug("{} fields".format(fields))
            logger.debug("{} blocks".format(blocks))
            logger.error("fields and blocks are of different lengths")
            return None

        result = subprocess.Popen(["outback", "-m", "-c", ip, "-f", fields, "-b", blocks]
                                  , stdout=subprocess.PIPE)
        out, err = result.communicate()
        logger.info("Output " + out +  " error " + err)
        #TODO check error handling to check if register write fails.
        return out
