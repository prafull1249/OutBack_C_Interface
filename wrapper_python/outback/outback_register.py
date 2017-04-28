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
        result = subprocess.Popen(["./program", "-r", "-c", str(ip), "-f", str(self.field), "-b", str(self.block)],stdout=subprocess.PIPE)
        out, err = result.communicate()
        logger.info("Output " + str(out) +  " error " + str(err))
        #TODO check error handling to check if register read fails.
        return out

    def write_reg(self, value, ip, port=""):
        result = subprocess.Popen(["./program", "-w", "-c", str(ip), "-f", str(self.field), "-b", str(self.block), "-v", str(value)], stdout=subprocess.PIPE)
        out, err = result.communicate()
        logger.info("Output " + str(out) +  " error " + str(err))
        #TODO check error handling to check if register write fails.
        return out


    @staticmethod
    def read_multiple_reg( ip, fields, blocks):
        """
        Reads multiple registers.
        :param fields: a comma separated string of register field values
        :param blocks: a comma separated string of register block values
        :return:
        """
        fields_el = fields
        blocks_el = blocks

        if len(fields_el) != len(blocks_el):
            logger.debug("{} fields".format(str(fields)))
            logger.debug("{} blocks".format(str(blocks)))
            logger.error("fields and blocks are of different lengths")
            return None

        overall_out = ""
        out = None
	for i in range(len(fields_el)):
            field = fields_el[i]
            block = blocks_el[i]

            result = subprocess.Popen(["./program", "-r", "-c",str(ip), "-f", str(field), "-b", str(block)], stdout=subprocess.PIPE)
            out, err = result.communicate()
            logger.info("Output " + str(out) + " error " + str(err))
            overall_out = overall_out + out
            #TODO check error handling to check if register write fails.

        return out

    @staticmethod
    def write_multiple_reg( ip, fields, blocks, values):
        """
        Reads multiple registers.
        :param fields: a comma separated string of register field values
        :param blocks: a comma separated string of register block values
        :return:
        """
        fields_el = fields
        blocks_el = blocks

        if len(fields_el) != len(blocks_el):
            logger.debug("{} fields".format(str(fields)))
            logger.debug("{} blocks".format(str(blocks)))
            logger.error("fields and blocks are of different lengths")
            return None

        overall_out = ""
        out = None
	for i in range(len(fields_el)):
            field = fields_el[i]
            block = blocks_el[i]
	    value = values[i]
            result = subprocess.Popen(["./program", "-w", "-c",str(ip), "-f", str(field), "-b", str(block),"-v", str(value)], stdout=subprocess.PIPE)
            out, err = result.communicate()
            logger.info("Output " + str(out) + " error " + str(err))
            overall_out = overall_out + out
            #TODO check error handling to check if register write fails.

        return out
