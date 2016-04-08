import json
import multiprocessing
import urllib2
import time
import os
import random

import lifeguard.sensors as sensors


class JsonTalker:
    def __init__(self):
        pass

    @staticmethod
    def send(ip, message):
        try:
            r = urllib2.Request(ip, json.dumps(message), {"Content-Type": "application/json"})
            urllib2.urlopen(r)
        except urllib2.URLError as e:
            print("Connection timed out but yolo")


class PostProcessor:
    def __init__(self):
        pass

    @staticmethod
    def post_ekg(values):
        """

        :param values:
        :return: ppm
        """
        raise NotImplementedError()


if __name__ == '__main__':
    os.nice(5)

    # setup EKG
    ekg_queue = multiprocessing.Queue(100)
    ekg = sensors.EKG(ekg_queue, 0).start()

    while True:
        message = {}

        """"
        ekg_values = []
        while len(ekg_values) != 100:
            ekg_values.append(ekg_queue.get())
        # ekg_post = PostProcessor.post_ekg(ekg_values)

        acc_values = []
        """
        message["pulse"] = random.randint(0, 350)
        message["eyesOpen"] = random.randint(0, 100) / 100.0
        message["muscleActivity"] = random.randint(0, 500)
        JsonTalker.send('http://192.168.2.108:1337/measurements/', message)
        time.sleep(0.1)


