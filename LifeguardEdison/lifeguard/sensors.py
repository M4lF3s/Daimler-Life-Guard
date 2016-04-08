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
            self.queue.put(value)
            time.sleep(0.002)
