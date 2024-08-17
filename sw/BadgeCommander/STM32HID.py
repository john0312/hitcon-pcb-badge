import math
import hid

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
    send_command([0x02])
    print("Clearing BadUSB")
    tmp=device.read(8)
    if tmp[2] != 0xFF:
        print("Error")
    
#Send BadUSB Script
def send_badusb_script(script):
    datatosend = [0x00]*3
    datatosend[0] = 0x03
    datatosend[1:3] = len(script).to_bytes(2, 'big')
    datatosend = datatosend+ script
    for i in range(0, math.ceil(len(datatosend)), 8):
        print(datatosend[i:i+8])
        if datatosend[i] == 0:
            datatosend[i] = 0xFC
            print(datatosend[i:i+8])
        send_command(datatosend[i:i+8])
        if len(datatosend) - 8 != i:
            tmp=device.read(8)

def send_command(command):
    k=device.write(command)
    return k
    

def close_device():
    device.close()


if __name__ == '__main__':
    get_hid_device()
    for i in range(33):
        datatosend = [0x00]*32
        send_command(datatosend)
    close_device()

