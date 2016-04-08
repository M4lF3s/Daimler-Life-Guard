import mraa
import multiprocessing
import time
import os


class EKG(multiprocessing.Process):
    def __init__(self, queue, pin):
        super(EKG, self).__init__()
        self.queue = queue
        self.adc = mraa.Aio(pin)
        self.adc.setBit(12)
        os.nice(-20)

    def run(self):
        while True:
            value = self.adc.read()
            self.queue.put((time.time(), value))
            time.sleep(0.002)


class MuscleActivity(multiprocessing.Process):
    def __init__(self, queue, pin):
        super(MuscleActivity, self).__init__()
        self.queue = queue
        self.adc = mraa.Aio(pin)
        self.adc.setBit()
        os.nice(-20)

    def run(self):
        while True:
            value = self.adc.read()
            self.queue.put(value)
            time.sleep(0.02)


class Acc(multiprocessing.Process):
    def __init__(self, queue):
        super(Acc, self).__init__()
        self.queue = queue
        self.i2c = mraa.I2c(6)
        self.i2c.address(0x1D)
        if self.i2c.readReg(0x0D) != 0x2A:
            raise RuntimeError("Acc not connected.")

        self.i2c.writeReg(0x0E, 3)  # set scale
        self.i2c.writeReg(0x2A, 1)  # set active
        os.nice(-20)

    def run(self):
        while True:
            acc_raw = self.i2c.readBytesReg(0x01, 6)
            self.queue.put(acc_raw)
            time.sleep(0.02)
