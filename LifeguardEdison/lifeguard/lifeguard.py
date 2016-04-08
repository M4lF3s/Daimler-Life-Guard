import json
import multiprocessing
import os
import time
import urllib2

import sensors


class JsonTalker:
    def __init__(self):
        pass

    @staticmethod
    def send(ip, message):
        try:
            r = urllib2.Request(ip, json.dumps(message), {"Content-Type": "application/json"})
            urllib2.urlopen(r)
        except urllib2.URLError:
            print("Connection timed out but yolo")


class PostProcessor:
    def __init__(self):
        # EKG variables
        self.ekg_smo_sig = 0
        self.ekg_last_val = 0
        self.ekg_threshold = -60
        self.ekg_threshold_up = None
        self.ekg_t_last_pulse = None
        self.ekg_pulse_bpm = -1

        # muscle variables

    def post_ekg(self, t, new_val):
        self.ekg_smo_sig += 1. / 15. * (new_val - self.ekg_last_val)
        if self.ekg_smo_sig > self.ekg_threshold:
            self.ekg_threshold_up = 1

        if self.ekg_smo_sig < self.ekg_threshold and self.ekg_threshold_up == 1:
            self.ekg_threshold_up = 0
            if self.ekg_t_last_pulse is not None:
                self.ekg_pulse_bpm = 60 / (t - self.ekg_t_last_pulse)
            self.ekg_t_last_pulse = t

        self.ekg_last_val = new_val
        return self.ekg_pulse_bpm

    def post_acc(self, raw_acc):
        x = PostProcessor._two_comp(raw_acc[0], raw_acc[1])
        y = PostProcessor._two_comp(raw_acc[2], raw_acc[3])
        z = PostProcessor._two_comp(raw_acc[4], raw_acc[5])
        return x, y, z

    @staticmethod
    def _two_comp(high_byte, low_byte):
        val = (high_byte << 8) | low_byte
        val >>= 4

        if ( val & (1 << (12 - 1))) != 0:
            val -= 1 << 12
        return val

if __name__ == '__main__':
    os.nice(5)
    post = PostProcessor()
    last_send = time.time()

    # setup EKG
    ekg_queue = multiprocessing.Queue(100)
    ekg = sensors.EKG(ekg_queue, 0).start()

    # setup Acc
    acc_queue = multiprocessing.Queue(100)
    acc = sensors.Acc(acc_queue).start()

    # setup Muscle
    muscle_queue = multiprocessing.Queue(100)
    muscle = sensors.MuscleActivity(muscle_queue, 1).start()

    while True:
        ekg_time, ekg_adc = ekg_queue.get()
        bpm = post.post_ekg(ekg_time, -ekg_adc)

        acc_raw = acc_queue.get()
        x, y, z = post.post_acc(acc_raw)
        # print(str(x) + ";" + str(y) + ";" + str(z))

        muscle_adc = muscle_queue.get()
        print(muscle_adc)
        """

        if time.time() - last_send > 90:
            message = {"pulse": bpm}
            JsonTalker.send('http://192.168.2.108:1337/measurements/', message)
        """
