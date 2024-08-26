import ScanCode
nextStringDelay = 0
defaultDelay = 0
defaultStringDelay =0

def scriptToHex(scriptPath):
    hexData = []
    with open(scriptPath, 'r') as file:
        # read a list of lines into data
        data = file.readlines()
    data = [x.strip() for x in data]
    for line in data:
        if line.startswith("REPEAT"):
            hexData=lastCommand*int(line.split(" ")[1])
            print("REPEAT")
        elif line.startswith("STRING_DELAY") or line.startswith("STRINGDELAY"):
            global nextStringDelay
            nextStringDelay=int(int(line.split(" ")[1])/10)
        elif line.startswith("DEFAULT_STRING_DELAY") or line.startswith("DEFAULTSTRINGDELAY"):
            global defaultStringDelay
            defaultStringDelay=int(int(line.split(" ")[1])/10)
        elif line.startswith("DEFAULT_DELAY") or line.startswith("DEFAULTDELAY"):
            global defaultDelay
            defaultDelay=int(int(line.split(" ")[1])/10)
        elif line != "" and not line.startswith("REM"):
            print(line)
            lastCommand=lineToHex(line)
            if defaultDelay != 0:
                if defaultDelay>255:
                    defaultDelay=255
                hexData.append(0xFF)
                hexData.append(defaultDelay)
            hexData=hexData+lastCommand
    hexData.append(0xFD)
    return hexData
            
            
def lineToHex(line):
    if line.startswith("DELAY"):
        return [0xFF, int(int(line.split(" ")[1])/10)]
    elif line.startswith("STRINGLN"):
        print (line[9:])
        return stringToHex(line[9:])+[ScanCode.KEY_ENTER]
    elif line.startswith("STRING"):
        print (line[7:])
        return stringToHex(line[7:])
    elif line.startswith("ALTCHAR"):
        return [0xFE, ScanCode.KEY_MOD_LALT]+modCharToHex(line.split(" ")[1])+[0x00]
    
    
    # combie the modifier keys
    # CTRL-ALT 	CTRL+ALT
    # CTRL-SHIFT 	CTRL+SHIFT
    # ALT-SHIFT 	ALT+SHIFT
    # ALT-GUI 	ALT+WIN
    # GUI-SHIFT 	WIN+SHIFT
    # GUI-CTRL 	WIN+CTRL 
    elif line.startswith("CTRL-ALT"):
        data=[0xFE, ScanCode.KEY_MOD_LCTRL+ScanCode.KEY_MOD_LALT]
        
        #check if the command is only one character
        if len(line.split(" ")[1])==1:
            data.extend(modCharToHex(line.split(" ")[1]))
        else:
            data.extend(modKeyToHex(line.split(" ")[1]))
        data.extend([0x00])
        return data
    
    elif line.startswith("CTRL-SHIFT"):
        data=[0xFE, ScanCode.KEY_MOD_LCTRL+ScanCode.KEY_MOD_LSHIFT]
        
        #check if the command is only one character
        if len(line.split(" ")[1])==1:
            data.extend(modCharToHex(line.split(" ")[1]))
        else:
            data.extend(modKeyToHex(line.split(" ")[1]))
        data.extend([0x00])
        return data
    
    elif line.startswith("ALT-SHIFT"):
        data=[0xFE, ScanCode.KEY_MOD_LALT+ScanCode.KEY_MOD_LSHIFT]
        
        #check if the command is only one character
        if len(line.split(" ")[1])==1:
            data.extend(modCharToHex(line.split(" ")[1]))
        else:
            data.extend(modKeyToHex(line.split(" ")[1]))
        data.extend([0x00])
        return data
    
    elif line.startswith("ALT-GUI"):
        data=[0xFE, ScanCode.KEY_MOD_LALT+ScanCode.KEY_MOD_LMETA]
        
        #check if the command is only one character
        if len(line.split(" ")[1])==1:
            data.extend(modCharToHex(line.split(" ")[1]))
        else:
            data.extend(modKeyToHex(line.split(" ")[1]))
        data.extend([0x00])
        return data
    
    elif line.startswith("GUI-SHIFT"):
        data=[0xFE, ScanCode.KEY_MOD_LMETA+ScanCode.KEY_MOD_LSHIFT]
        
        #check if the command is only one character
        if len(line.split(" ")[1])==1:
            data.extend(modCharToHex(line.split(" ")[1]))
        else:
            data.extend(modKeyToHex(line.split(" ")[1]))
        data.extend([0x00])
        return data
    
    elif line.startswith("GUI-CTRL"):
        data=[0xFE, ScanCode.KEY_MOD_LMETA+ScanCode.KEY_MOD_LCTRL]
        
        #check if the command is only one character
        if len(line.split(" ")[1])==1:
            data.extend(modCharToHex(line.split(" ")[1]))
        else:
            data.extend(modKeyToHex(line.split(" ")[1]))
        data.extend([0x00])
        return data
    
    elif line.startswith("GUI") or line.startswith("WINDOWS"):
        data=[0xFE, ScanCode.KEY_MOD_LMETA]
        
        #check if the command is only one character
        if len(line.split(" ")[1])==1:
            data.extend(modCharToHex(line.split(" ")[1]))
        else: #check other key
            data.extend(modKeyToHex(line.split(" ")[1]))
        data.extend([0x00])
        return data
    elif line.startswith("ALT"):
        data=[0xFE, ScanCode.KEY_MOD_LALT]
        
        #check if the command is only one character
        if len(line.split(" ")[1])==1:
            data.extend(modCharToHex(line.split(" ")[1]))
        else:
            data.extend(modKeyToHex(line.split(" ")[1]))
        data.extend([0x00])
        return data
    elif line.startswith("CTRL"):
        data=[0xFE, ScanCode.KEY_MOD_LCTRL]
        
        #check if the command is only one character
        if len(line.split(" ")[1])==1:
            data.extend(modCharToHex(line.split(" ")[1]))
        else:
            data.extend(modKeyToHex(line.split(" ")[1]))
        data.extend([0x00])
        return data
    elif line.startswith("SHIFT"):
        data=[0xFE, ScanCode.KEY_MOD_LSHIFT]
        
        #check if the command is only one character
        if len(line.split(" ")[1])==1:
            data.extend(modCharToHex(line.split(" ")[1]))
        else:
            data.extend(modKeyToHex(line.split(" ")[1]))
        data.extend([0x00])
        return data
    #special keys
    elif line.startswith("DOWNARROW") or line.startswith("DOWN"):
        return [ScanCode.KEY_DOWN]
    elif line.startswith("LEFTARROW") or line.startswith("LEFT"):
        return [ScanCode.KEY_LEFT]
    elif line.startswith("RIGHTARROW") or line.startswith("RIGHT"):
        return [ScanCode.KEY_RIGHT]
    elif line.startswith("UPARROW") or line.startswith("UP"):
        return [ScanCode.KEY_UP]
    elif line.startswith("ENTER"):
        return [ScanCode.KEY_ENTER]
    elif line.startswith("DELETE"):
        return [ScanCode.KEY_DELETE]
    elif line.startswith("BACKSPACE"):
        return [ScanCode.KEY_BACKSPACE]
    elif line.startswith("END"):
        return [ScanCode.KEY_END]
    elif line.startswith("HOME"):
        return [ScanCode.KEY_HOME]
    elif line.startswith("ESCAPE") or line.startswith("ESC"):
        return [ScanCode.KEY_ESC]
    elif line.startswith("INSERT"):
        return [ScanCode.KEY_INSERT]
    elif line.startswith("PAGEUP"):
        return [ScanCode.KEY_PAGEUP]
    elif line.startswith("PAGEDOWN"):
        return [ScanCode.KEY_PAGEDOWN]
    elif line.startswith("CAPSLOCK"):
        return [ScanCode.KEY_CAPSLOCK]
    elif line.startswith("NUMLOCK"):
        return [ScanCode.KEY_NUMLOCK]
    elif line.startswith("SCROLLLOCK"):
        return [ScanCode.KEY_SCROLLLOCK]
    elif line.startswith("PRINTSCREEN"):
        return [ScanCode.KEY_SYSRQ]
    elif line.startswith("BREAK") or line.startswith("PAUSE"):
        return [ScanCode.KEY_PAUSE]
    elif line.startswith("SPACE"):
        return [ScanCode.KEY_SPACE]
    elif line.startswith("TAB"):
        return [ScanCode.KEY_TAB]
    elif line.startswith("MENU") or line.startswith("APP"):
        return [ScanCode.KEY_MENU]
    elif line.startswith("F1"):
        return [ScanCode.KEY_F1]
    elif line.startswith("F2"):
        return [ScanCode.KEY_F2]
    elif line.startswith("F3"):
        return [ScanCode.KEY_F3]
    elif line.startswith("F4"):
        return [ScanCode.KEY_F4]
    elif line.startswith("F5"):
        return [ScanCode.KEY_F5]
    elif line.startswith("F6"):
        return [ScanCode.KEY_F6]
    elif line.startswith("F7"):
        return [ScanCode.KEY_F7]
    elif line.startswith("F8"):
        return [ScanCode.KEY_F8]
    elif line.startswith("F9"):
        return [ScanCode.KEY_F9]
    elif line.startswith("F10"):
        return [ScanCode.KEY_F10]
    elif line.startswith("F11"):
        return [ScanCode.KEY_F11]
    elif line.startswith("F12"):
        return [ScanCode.KEY_F12]
    else:
        return [0xFC]
    
    
def stringToHex(string):
    global nextStringDelay
    global defaultStringDelay
    if nextStringDelay != 0:
        if nextStringDelay>255:
            nextStringDelay=255
        rawDelay=[0xFF]+[nextStringDelay]
    elif defaultStringDelay != 0:
        if defaultStringDelay>255:
            defaultStringDelay=255
        rawDelay=[0xFF]+[defaultStringDelay]
    hexData = []
    for char in string:
        hexData.extend(charToHex(char))
        if nextStringDelay != 0 or defaultStringDelay != 0:
            hexData.extend(rawDelay)
    nextStringDelay=0
    return hexData

def modKeyToHex(key):
    # DOWNARROW / DOWN 	
    # LEFTARROW / LEFT 	
    # RIGHTARROW / RIGHT 	
    # UPARROW / UP 	
    # ENTER 	
    # DELETE 	
    # BACKSPACE 	
    # END 	
    # HOME 	
    # ESCAPE / ESC 	
    # INSERT 	
    # PAGEUP 	
    # PAGEDOWN 	
    # CAPSLOCK 	
    # NUMLOCK 	
    # SCROLLLOCK 	
    # PRINTSCREEN 	
    # BREAK 	Pause/Break key
    # PAUSE 	Pause/Break key
    # SPACE 	
    # TAB 	
    # MENU 	Context menu key
    # APP 	Same as MENU
    # Fx 
    if key == "DOWNARROW" or key == "DOWN":
        return [ScanCode.KEY_DOWN]
    elif key == "LEFTARROW" or key == "LEFT":
        return [ScanCode.KEY_LEFT]
    elif key == "RIGHTARROW" or key == "RIGHT":
        return [ScanCode.KEY_RIGHT]
    elif key == "UPARROW" or key == "UP":
        return [ScanCode.KEY_UP]
    elif key == "ENTER":
        return [ScanCode.KEY_ENTER]
    elif key == "DELETE":
        return [ScanCode.KEY_DELETE]
    elif key == "BACKSPACE":
        return [ScanCode.KEY_BACKSPACE]
    elif key == "END":
        return [ScanCode.KEY_END]
    elif key == "HOME":
        return [ScanCode.KEY_HOME]
    elif key == "ESCAPE" or key == "ESC":
        return [ScanCode.KEY_ESC]
    elif key == "INSERT":
        return [ScanCode.KEY_INSERT]
    elif key == "PAGEUP":
        return [ScanCode.KEY_PAGEUP]
    elif key == "PAGEDOWN":
        return [ScanCode.KEY_PAGEDOWN]
    elif key == "CAPSLOCK":
        return [ScanCode.KEY_CAPSLOCK]
    elif key == "NUMLOCK":
        return [ScanCode.KEY_NUMLOCK]
    elif key == "SCROLLLOCK":
        return [ScanCode.KEY_SCROLLLOCK]
    elif key == "PRINTSCREEN":
        return [ScanCode.KEY_SYSRQ]
    elif key == "BREAK" or key == "PAUSE":
        return [ScanCode.KEY_PAUSE]
    elif key == "SPACE":
        return [ScanCode.KEY_SPACE]
    elif key == "TAB":
        return [ScanCode.KEY_TAB]
    elif key == "MENU" or key == "APP":
        return [ScanCode.KEY_MENU]
    elif key == "F1":
        return [ScanCode.KEY_F1]
    elif key == "F2":
        return [ScanCode.KEY_F2]
    elif key == "F3":
        return [ScanCode.KEY_F3]
    elif key == "F4":
        return [ScanCode.KEY_F4]
    elif key == "F5":
        return [ScanCode.KEY_F5]
    elif key == "F6":
        return [ScanCode.KEY_F6]
    elif key == "F7":
        return [ScanCode.KEY_F7]
    elif key == "F8":
        return [ScanCode.KEY_F8]
    elif key == "F9":
        return [ScanCode.KEY_F9]
    elif key == "F10":
        return [ScanCode.KEY_F10]
    elif key == "F11":
        return [ScanCode.KEY_F11]
    elif key == "F12":
        return [ScanCode.KEY_F12]
    else:
        return [0xFC]

#Use for the Modifier + Key combinations
def modCharToHex(char):
    if char == "a" or char == "A":
        return [ScanCode.KEY_A]
    elif char == "b" or char == "B":
        return [ScanCode.KEY_B]
    elif char == "c" or char == "C":
        return [ScanCode.KEY_C]
    elif char == "d" or char == "D":
        return [ScanCode.KEY_D]
    elif char == "e" or char == "E":
        return [ScanCode.KEY_E]
    elif char == "f" or char == "F":
        return [ScanCode.KEY_F]
    elif char == "g" or char == "G":
        return [ScanCode.KEY_G]
    elif char == "h" or char == "H":
        return [ScanCode.KEY_H]
    elif char == "i" or char == "I":
        return [ScanCode.KEY_I]
    elif char == "j" or char == "J":
        return [ScanCode.KEY_J]
    elif char == "k" or char == "K":
        return [ScanCode.KEY_K]
    elif char == "l" or char == "L":
        return [ScanCode.KEY_L]
    elif char == "m" or char == "M":
        return [ScanCode.KEY_M]
    elif char == "n" or char == "N":
        return [ScanCode.KEY_N]
    elif char == "o" or char == "O":
        return [ScanCode.KEY_O]
    elif char == "p" or char == "P":
        return [ScanCode.KEY_P]
    elif char == "q" or char == "Q":
        return [ScanCode.KEY_Q]
    elif char == "r" or char == "R":
        return [ScanCode.KEY_R]
    elif char == "s" or char == "S":
        return [ScanCode.KEY_S]
    elif char == "t" or char == "T":
        return [ScanCode.KEY_T]
    elif char == "u" or char == "U":
        return [ScanCode.KEY_U]
    elif char == "v" or char == "V":
        return [ScanCode.KEY_V]
    elif char == "w" or char == "W":
        return [ScanCode.KEY_W]
    elif char == "x" or char == "X":
        return [ScanCode.KEY_X]
    elif char == "y" or char == "Y":
        return [ScanCode.KEY_Y]
    elif char == "z" or char == "Z":
        return [ScanCode.KEY_Z]
    elif char == "1":
        return [ScanCode.KEY_1]
    elif char == "2":
        return [ScanCode.KEY_2]
    elif char == "3":
        return [ScanCode.KEY_3]
    elif char == "4":
        return [ScanCode.KEY_4]
    elif char == "5":
        return [ScanCode.KEY_5]
    elif char == "6":
        return [ScanCode.KEY_6]
    elif char == "7":
        return [ScanCode.KEY_7]
    elif char == "8":
        return [ScanCode.KEY_8]
    elif char == "9":
        return [ScanCode.KEY_9]
    elif char == "0":
        return [ScanCode.KEY_0]
    elif char == " ":
        return [ScanCode.KEY_SPACE]
    elif char == "-":
        return [ScanCode.KEY_MINUS]
    elif char == "=":
        return [ScanCode.KEY_EQUAL]
    elif char == "[":
        return [ScanCode.KEY_LEFTBRACE]
    elif char == "]":
        return [ScanCode.KEY_RIGHTBRACE]
    elif char == "\\":
        return [ScanCode.KEY_BACKSLASH]
    elif char == "#":
        return [ScanCode.KEY_HASHTILDE]
    elif char == ";":
        return [ScanCode.KEY_SEMICOLON]
    elif char == "'":
        return [ScanCode.KEY_APOSTROPHE]
    elif char == "`":
        return [ScanCode.KEY_GRAVE]
    elif char == ",":
        return [ScanCode.KEY_COMMA]
    elif char == ".":
        return [ScanCode.KEY_DOT]
    elif char == "/":
        return [ScanCode.KEY_SLASH]
    else:
        return [0xFC]
        
#Use for the stringToHex function
def charToHex(char):
    if char == "a":
        return [ScanCode.KEY_A]
    elif char == "A":
        # Shift + a
        return [0xFE, ScanCode.KEY_MOD_LSHIFT, ScanCode.KEY_A, 0x00]
    elif char == "b":
        return [ScanCode.KEY_B]
    elif char == "B":
        return [0xFE, ScanCode.KEY_MOD_LSHIFT, ScanCode.KEY_B, 0x00]
    elif char == "c":
        return [ScanCode.KEY_C]
    elif char == "C":
        return [0xFE, ScanCode.KEY_MOD_LSHIFT, ScanCode.KEY_C, 0x00]
    elif char == "d":
        return [ScanCode.KEY_D]
    elif char == "D":
        return [0xFE, ScanCode.KEY_MOD_LSHIFT, ScanCode.KEY_D, 0x00]
    elif char == "e":
        return [ScanCode.KEY_E]
    elif char == "E":
        return [0xFE, ScanCode.KEY_MOD_LSHIFT, ScanCode.KEY_E, 0x00]
    elif char == "f":
        return [ScanCode.KEY_F]
    elif char == "F":
        return [0xFE, ScanCode.KEY_MOD_LSHIFT, ScanCode.KEY_F, 0x00]
    elif char == "g":
        return [ScanCode.KEY_G]
    elif char == "G":
        return [0xFE, ScanCode.KEY_MOD_LSHIFT, ScanCode.KEY_G, 0x00]
    elif char == "h":
        return [ScanCode.KEY_H]
    elif char == "H":
        return [0xFE, ScanCode.KEY_MOD_LSHIFT, ScanCode.KEY_H, 0x00]
    elif char == "i":
        return [ScanCode.KEY_I]
    elif char == "I":
        return [0xFE, ScanCode.KEY_MOD_LSHIFT, ScanCode.KEY_I, 0x00]
    elif char == "j":
        return [ScanCode.KEY_J]
    elif char == "J":
        return [0xFE, ScanCode.KEY_MOD_LSHIFT, ScanCode.KEY_J, 0x00]
    elif char == "k":
        return [ScanCode.KEY_K]
    elif char == "K":
        return [0xFE, ScanCode.KEY_MOD_LSHIFT, ScanCode.KEY_K, 0x00]
    elif char == "l":
        return [ScanCode.KEY_L]
    elif char == "L":
        return [0xFE, ScanCode.KEY_MOD_LSHIFT, ScanCode.KEY_L, 0x00]
    elif char == "m":
        return [ScanCode.KEY_M]
    elif char == "M":
        return [0xFE, ScanCode.KEY_MOD_LSHIFT, ScanCode.KEY_M, 0x00]
    elif char == "n":
        return [ScanCode.KEY_N]
    elif char == "N":
        return [0xFE, ScanCode.KEY_MOD_LSHIFT, ScanCode.KEY_N, 0x00]
    elif char == "o":
        return [ScanCode.KEY_O]
    elif char == "O":
        return [0xFE, ScanCode.KEY_MOD_LSHIFT, ScanCode.KEY_O, 0x00]
    elif char == "p":
        return [ScanCode.KEY_P]
    elif char == "P":
        return [0xFE, ScanCode.KEY_MOD_LSHIFT, ScanCode.KEY_P, 0x00]
    elif char == "q":
        return [ScanCode.KEY_Q]
    elif char == "Q":
        return [0xFE, ScanCode.KEY_MOD_LSHIFT, ScanCode.KEY_Q, 0x00]
    elif char == "r":
        return [ScanCode.KEY_R]
    elif char == "R":
        return [0xFE, ScanCode.KEY_MOD_LSHIFT, ScanCode.KEY_R, 0x00]
    elif char == "s":
        return [ScanCode.KEY_S]
    elif char == "S":
        return [0xFE, ScanCode.KEY_MOD_LSHIFT, ScanCode.KEY_S, 0x00]
    elif char == "t":
        return [ScanCode.KEY_T]
    elif char == "T":
        return [0xFE, ScanCode.KEY_MOD_LSHIFT, ScanCode.KEY_T, 0x00]
    elif char == "u":
        return [ScanCode.KEY_U]
    elif char == "U":
        return [0xFE, ScanCode.KEY_MOD_LSHIFT, ScanCode.KEY_U, 0x00]
    elif char == "v":
        return [ScanCode.KEY_V]
    elif char == "V":
        return [0xFE, ScanCode.KEY_MOD_LSHIFT, ScanCode.KEY_V, 0x00]
    elif char == "w":
        return [ScanCode.KEY_W]
    elif char == "W":
        return [0xFE, ScanCode.KEY_MOD_LSHIFT, ScanCode.KEY_W, 0x00]
    elif char == "x":
        return [ScanCode.KEY_X]
    elif char == "X":
        return [0xFE, ScanCode.KEY_MOD_LSHIFT, ScanCode.KEY_X, 0x00]
    elif char == "y":
        return [ScanCode.KEY_Y]
    elif char == "Y":
        return [0xFE, ScanCode.KEY_MOD_LSHIFT, ScanCode.KEY_Y, 0x00]
    elif char == "z":
        return [ScanCode.KEY_Z]
    elif char == "Z":
        return [0xFE, ScanCode.KEY_MOD_LSHIFT, ScanCode.KEY_Z, 0x00]
    elif char == "1":
        return [ScanCode.KEY_1]
    elif char == "!":
        return [0xFE, ScanCode.KEY_MOD_LSHIFT, ScanCode.KEY_1, 0x00]
    elif char == "2":
        return [ScanCode.KEY_2]
    elif char == "@":
        return [0xFE, ScanCode.KEY_MOD_LSHIFT, ScanCode.KEY_2, 0x00]
    elif char == "3":
        return [ScanCode.KEY_3]
    elif char == "#":
        return [0xFE, ScanCode.KEY_MOD_LSHIFT, ScanCode.KEY_3, 0x00]
    elif char == "4":
        return [ScanCode.KEY_4]
    elif char == "$":
        return [0xFE, ScanCode.KEY_MOD_LSHIFT, ScanCode.KEY_4, 0x00]
    elif char == "5":
        return [ScanCode.KEY_5]
    elif char == "%":
        return [0xFE, ScanCode.KEY_MOD_LSHIFT, ScanCode.KEY_5, 0x00]
    elif char == "6":
        return [ScanCode.KEY_6]
    elif char == "^":
        return [0xFE, ScanCode.KEY_MOD_LSHIFT, ScanCode.KEY_6, 0x00]
    elif char == "7":
        return [ScanCode.KEY_7]
    elif char == "&":
        return [0xFE, ScanCode.KEY_MOD_LSHIFT, ScanCode.KEY_7, 0x00]
    elif char == "8":
        return [ScanCode.KEY_8]
    elif char == "*":
        return [0xFE, ScanCode.KEY_MOD_LSHIFT, ScanCode.KEY_8, 0x00]
    elif char == "9":
        return [ScanCode.KEY_9]
    elif char == "(":
        return [0xFE, ScanCode.KEY_MOD_LSHIFT, ScanCode.KEY_9, 0x00]
    elif char == "0":
        return [ScanCode.KEY_0]
    elif char == ")":
        return [0xFE, ScanCode.KEY_MOD_LSHIFT, ScanCode.KEY_0, 0x00]

    
    # KEY_SPACE =0x2c # Keyboard Spacebar
    # KEY_MINUS =0x2d # Keyboard - and _
    # KEY_EQUAL =0x2e # Keyboard = and +
    # KEY_LEFTBRACE =0x2f # Keyboard [ and {
    # KEY_RIGHTBRACE =0x30 # Keyboard ] and }
    # KEY_BACKSLASH =0x31 # Keyboard \ and |
    # KEY_HASHTILDE =0x32 # Keyboard Non-US # and ~
    # KEY_SEMICOLON =0x33 # Keyboard ; and :
    # KEY_APOSTROPHE =0x34 # Keyboard ' and "
    # KEY_GRAVE =0x35 # Keyboard ` and ~
    # KEY_COMMA =0x36 # Keyboard , and <
    # KEY_DOT =0x37 # Keyboard . and >
    # KEY_SLASH =0x38 # Keyboard / and ?
    
    elif char == " ":
        return [ScanCode.KEY_SPACE]
    elif char == "-":
        return [ScanCode.KEY_MINUS]
    elif char == "_":
        return [0xFE, ScanCode.KEY_MOD_LSHIFT, ScanCode.KEY_MINUS, 0x00]
    elif char == "=":
        return [ScanCode.KEY_EQUAL]
    elif char == "+":
        return [0xFE, ScanCode.KEY_MOD_LSHIFT, ScanCode.KEY_EQUAL, 0x00]
    elif char == "[":
        return [ScanCode.KEY_LEFTBRACE]
    elif char == "{":
        return [0xFE, ScanCode.KEY_MOD_LSHIFT, ScanCode.KEY_LEFTBRACE, 0x00]
    elif char == "]":
        return [ScanCode.KEY_RIGHTBRACE]
    elif char == "}":
        return [0xFE, ScanCode.KEY_MOD_LSHIFT, ScanCode.KEY_RIGHTBRACE, 0x00]
    elif char == "\\":
        return [ScanCode.KEY_BACKSLASH]
    elif char == "|":
        return [0xFE, ScanCode.KEY_MOD_LSHIFT, ScanCode.KEY_BACKSLASH, 0x00]
    elif char == "#":
        return [ScanCode.KEY_HASHTILDE]
    elif char == "~":
        return [0xFE, ScanCode.KEY_MOD_LSHIFT, ScanCode.KEY_HASHTILDE, 0x00]
    elif char == ";":
        return [ScanCode.KEY_SEMICOLON]
    elif char == ":":
        return [0xFE, ScanCode.KEY_MOD_LSHIFT, ScanCode.KEY_SEMICOLON, 0x00]
    elif char == "'":
        return [ScanCode.KEY_APOSTROPHE]
    elif char == "\"":
        return [0xFE, ScanCode.KEY_MOD_LSHIFT, ScanCode.KEY_APOSTROPHE, 0x00]
    elif char == "`":
        return [ScanCode.KEY_GRAVE]
    elif char == "~":
        return [0xFE, ScanCode.KEY_MOD_LSHIFT, ScanCode.KEY_GRAVE, 0x00]
    elif char == ",":
        return [ScanCode.KEY_COMMA]
    elif char == "<":
        return [0xFE, ScanCode.KEY_MOD_LSHIFT, ScanCode.KEY_COMMA, 0x00]
    elif char == ".":
        return [ScanCode.KEY_DOT]
    elif char == ">":
        return [0xFE, ScanCode.KEY_MOD_LSHIFT, ScanCode.KEY_DOT, 0x00]
    elif char == "/":
        return [ScanCode.KEY_SLASH]
    elif char == "?":
        return [0xFE, ScanCode.KEY_MOD_LSHIFT, ScanCode.KEY_SLASH, 0x00]
    else:
        return [0xFC]
    
    
    
if __name__ == '__main__':
    print(list(map(hex, scriptToHex("/home/justin/Downloads/demo_windows.txt"))))
    data=scriptToHex("/home/justin/Downloads/demo_windows.txt")
    with open('test', 'wb') as f:
        for i in data:
            f.write(bytes((i,)))