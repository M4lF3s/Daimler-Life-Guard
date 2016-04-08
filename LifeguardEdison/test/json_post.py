import json
import os
import random
import time
import urllib2


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


if __name__ == '__main__':
    os.nice(5)

    while True:
        message = {"pulse": random.randint(0, 350), "eyesOpen": random.randint(0, 100) / 100.0,
                   "muscleActivity": random.randint(0, 500)}

        JsonTalker.send('http://192.168.2.108:1337/measurements/', message)
        time.sleep(0.1)


