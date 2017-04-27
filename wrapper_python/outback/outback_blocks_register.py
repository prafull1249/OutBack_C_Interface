__author__ = 'Prafull'

from __utils__ import logger
import blocks

class block:
    """
    Register class which has all the fields and block related to a
    """

    def __init__(self,name):

        self.registers = self.get_registers_in_block()
        self.num_of_reg = len(self.registers)
        self.name = name

    def get_registers_in_block(self, index):
        name = ""
        if index in blocks.blocks_name:
            name = blocks.blocks_name[index]
        else:
            logger.error("index {} is not in blocks_name ".format({index}))
        if name in blocks.blocks:
            return blocks.blocks[name]
        logger.error("name {} is not in blocks".format({name}))
