import mraa

def twos_comp(highbyte,lowbyte, bits):
    """compute the 2's compliment of int value val"""
    print('convert')
    print(bin(highbyte))
    print(bin(lowbyte))
    val=(highbyte<<8) | lowbyte
    val=val >>4
    print(bin(val))
    if (val & (1 << (bits - 1))) != 0: # if sign bit is set e.g., 8bit: 128-255
        val = val - (1 << bits)        # compute negative value
    return val 

i2c=mraa.I2c(6)
i2c.address(0x1D)
a=i2c.readReg(0x0D)
if a==0x2A:
    print("Sensor found")

#in ctrl_reg1 active bit setzen
i2c.writeReg(0x2A,0b00000001)

acc_raw_data=i2c.readBytesReg(0x01,6)
print(bin(acc_raw_data[0]))

x=twos_comp(acc_raw_data[0],acc_raw_data[1],12)
y=twos_comp(acc_raw_data[2],acc_raw_data[3],12)
z=twos_comp(acc_raw_data[4],acc_raw_data[5],12)
print('x: {0:4d} y: {1:4d} z: {2:4d}'.format(x,y,z))



