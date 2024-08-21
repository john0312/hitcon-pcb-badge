import math
import time
import hid
import crc32

vendor_id = 1155
product_id = 22352

def get_hid_device():
    for device_dict in hid.enumerate():
        keys = list(device_dict.keys())
        keys.sort()
        for key in keys:
            print("%s : %s" % (key, device_dict[key]))
        print()
        
    global device
    device = hid.device()
    device.open(vendor_id, product_id)

# Set ShowName App Name
def set_name(name):
    datatosend = [0x00]*17
    datatosend[0] = 0x01
    datatosend[1:len(name)+1] = [ord(c) for c in name]
    print(datatosend)
    k=send_command(datatosend)
    return k
    
#BadUSB Commands
#Clear BadUSB
def clear_badusb():
    send_command([0x00]+[0x02])
    print("Clearing BadUSB")
    time.sleep(1)
    
#Send BadUSB Script
def send_badusb_script(script):
    datatosend = [0x00]*3
    datatosend[0] = 0x03
    datatosend[1:3] = len(script).to_bytes(2, 'big')
    crc = crc32.Crc32(0x04C11DB7)
    script = script+ [0x00]*(4-len(script)%4)
    checksum = crc.crc_int_to_bytes(crc.calculate(script))
    datatosend = datatosend+checksum+ script
    for i in range(0, math.ceil(len(datatosend)), 8):
        print(datatosend[i: i+8])
        if datatosend[i] == 0:
            datatosend[i] = 0xFC
            print(datatosend[i: i+8])
        send_command(datatosend[i: i+8])
        if len(datatosend) - 8 != i:
            device.read(8) # wait for response
    
    # for i in range(0, math.ceil(len(datatosend)), 8):
        # print(datatosend[i:i+8])
        # if datatosend[i] == 0:
        #     datatosend[i] = 0xFC
        # dataPacket = [0x00]*8
        # if(len(datatosend[i:]) < 8):
        #     dataPacket[0:len(datatosend) - i] = datatosend[i:]
        # else:
        #     dataPacket[0:8] = datatosend[i:i+8]
        # print(dataPacket)
        # send_command(dataPacket)
        # if len(datatosend) - 8 != i:
        #     tmp=device.read(8)
        #     print(tmp)

def send_command(command):
    k=device.write(command)
    return k
    

def close_device():
    device.close()


if __name__ == '__main__':
    get_hid_device()    
    close_device()

