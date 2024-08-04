def set_name(name):
    datatosend = [0x00]*17
    datatosend[0] = 0x01
    datatosend[1:len(name)+1] = [ord(c) for c in name]
    print(datatosend)

set_name("HITCONN")