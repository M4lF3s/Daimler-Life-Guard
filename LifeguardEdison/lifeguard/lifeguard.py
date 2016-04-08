import json
import multiprocessing
import os
import time
import urllib2
import Queue
import sensors


class JsonTalker:
    def __init__(self):
        pass

    @staticmethod
    def send(ip, message):
        try:
            r = urllib2.Request(ip, json.dumps(message), {"Content-Type": "application/json"})
            urllib2.urlopen(r)
        except urllib2.URLError as e:
            print(e.reason())


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
        self.muscle_smo_sig = None

        # acc variables
        self.acc_last_z = 0
        self.acc_smo_sig = 0

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

    def post_muscle(self, new_val):
        if self.muscle_smo_sig is None:
            self.muscle_smo_sig = new_val
            return False
        self.muscle_smo_sig = 199. / 200. * self.muscle_smo_sig + 1. / 200. * (new_val)

        if new_val > self.muscle_smo_sig * 1.2 or new_val < self.muscle_smo_sig * 0.8:
            return True
        return False

    def post_acc(self, z_raw):
        z = PostProcessor._two_comp(z_raw[0], z_raw[1])

        self.acc_smo_sig = 49. / 50. * self.acc_smo_sig + 1. / 50. * abs(z - self.acc_last_z)
        self.acc_last_z = z

        if self.acc_smo_sig > 18:
            return True
        return False

    @staticmethod
    def _two_comp(high_byte, low_byte):
        val = (high_byte << 8) | low_byte
        val >>= 4

        if (val & (1 << (12 - 1))) != 0:
            val -= 1 << 12
        return val


if __name__ == '__main__':
    os.nice(5)
    post = PostProcessor()
    last_send = time.time()

    # setup EKG
    ekg_queue = multiprocessing.Queue(100)
    ekg = sensors.EKG(ekg_queue, 0).start()
    bpm = None
    ekg_adcs = []
    last_ekg_time = 0

    # setup Acc
    acc_queue = multiprocessing.Queue(100)
    acc = sensors.Acc(acc_queue).start()
    is_shaking = None

    # setup Muscle
    muscle_queue = multiprocessing.Queue(100)
    muscle = sensors.MuscleActivity(muscle_queue, 1).start()
    muscle_adc = None
    muscle_adcs = []
    is_seizure = None

    # setup contact sensor
    contact_queue = multiprocessing.Queue(100)
    contact = sensors.ContactSensor(contact_queue, 2).start()
    is_contacted = None

    while True:
        ekg_time, ekg_adc = ekg_queue.get()
        last_ekg_time = ekg_time
        ekg_adcs.append(ekg_adc)
        bpm = post.post_ekg(ekg_time, -ekg_adc)

        try:
            z_raw = acc_queue.get_nowait()
            is_shaking = post.post_acc(z_raw)
        except Queue.Empty:
            pass

        try:
            muscle_adc = muscle_queue.get_nowait()
            muscle_adcs.append(muscle_adc)
            is_seizure = post.post_muscle(muscle_adc)
        except Queue.Empty:
            pass

        try:
            is_contacted = contact_queue.get_nowait()
        except Queue.Empty:
            pass

        if time.time() - last_send > 0.1:
            message = {"muscleActivity": muscle_adcs, "isSeizure": is_seizure, "pulse": bpm,
                       "pulseActivity": ekg_adcs, "isWheelTouched": is_contacted, "isShaking": is_shaking}
            JsonTalker.send('http://192.168.2.108:1337/measurements/', message)
            last_send = time.time()
            muscle_adcs = []
            ekg_adcs = []
