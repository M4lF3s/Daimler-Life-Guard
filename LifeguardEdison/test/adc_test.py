import mraa
import time
import os

os.nice(-20)
duration=10000
adc_values=[]
time_values=[]
adc_0=mraa.Aio(0)
adc_0.setBit(12)
starttime=time.time()
for i in range(duration):
    adc_values.append(adc_0.read())
    time_values.append(time.clock())
    time.sleep(0.002)
endtime=time.time()

print(endtime-starttime)
for i in range(duration):
    print('{0:f} {1:f}'.format(adc_values[i],time_values[i]))

