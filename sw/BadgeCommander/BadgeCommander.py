import re
import ttkbootstrap as ttk
import imageres
from tkinter import filedialog
from ttkbootstrap.dialogs import Messagebox
from tkinter import font as tkFont
import sys
import STM32HID
import tkinter as tk
import BadUSBTranslation

debugHID=False


#Check the connection of the Badge
def checkBadgeConnection():
    try:
        STM32HID.get_hid_device()
    except:
        Messagebox.show_error('Please Connect the PCB Badge to the Computer and try again', 'Error')
        checkBadgeConnection()


# Function to get the OS
def getOS():
    return sys.platform

# Function to get the zoom value of the Windows system
def getWinZoomValue():
    import ctypes
    user32 = ctypes.windll.user32
    user32.SetProcessDPIAware()
    zoomValue = user32.GetDpiForSystem() / 96
    return zoomValue

# Get the zoom value of the system, if the system is windows, it will return the zoom value, otherwise it will return 1
def getZoomValue():
    print(getOS())
    if getOS() == "win32":
        zoomValue = getWinZoomValue()
    #linux
    elif getOS() == "linux":
        try:
            import LinuxScale
            print(LinuxScale.get_scaling_factor())
            zoomValue = float(LinuxScale.get_scaling_factor())
        except:
            print('Error getting the zoom value of the system')
            zoomValue = int(input('Please input the zoom value of your system: '))
    else:
        zoomValue = 1
    return zoomValue

def writeMemory(addr, data, mode):
    #translate mode to int (Byte(8bit)', 'Hfwd(16bit)', 'Word(32bit))
    print(mode)
    if mode==0:
        mode=1
    elif mode==1:
        mode=2
    elif mode==2:
        mode=3
    else:
        Messagebox.show_error('Please Select a Valid Mode', 'Error')
        return
    
    try:
        k=STM32HID.write_memory(addr, data, mode)
        print(k)
    except:
        Messagebox.show_error('Please Connect the PCB Badge to the Computer and try again', 'Error')
        checkBadgeConnection()

def readMemory(addr, mode):
    #translate mode to int (Byte(8bit)', 'Hfwd(16bit)', 'Word(32bit))
    print(mode)
    if mode==0:
        mode=1
    elif mode==1:
        mode=2
    elif mode==2:
        mode=3
    else:
        Messagebox.show_error('Please Select a Valid Mode', 'Error')
        return
    
    try:
        k=STM32HID.read_memory(addr, mode)
        #translate the return value from byte array to hex string
        k=''.join(format(x, '02x') for x in k)
        print(k)
        
        #put the return value to the content entrybox
        contentEntry.delete(0, tk.END)
        if mode==1:
            contentEntry.insert(0, k[0:2])
        elif mode==2:
            contentEntry.insert(0, k[0:4])
        elif mode==3:
            contentEntry.insert(0, k[0:8])
    except:
        Messagebox.show_error('Please Connect the PCB Badge to the Computer and try again', 'Error')
        checkBadgeConnection()

def setName(Name):
    k=STM32HID.set_name(Name)
    
    if k==-1:
        #error
        checkBadgeConnection()
        
def writeBadUSB(scriptPath):
    if debugHID:
        rawData=BadUSBTranslation.scriptToHex(scriptPath)
        print(rawData)
        #print raw data as hex
        print(' '.join(format(x, '02x') for x in rawData))
        STM32HID.clear_badusb()
        STM32HID.send_badusb_script(rawData)
        Messagebox.show_info('BadUSB Script has been written to the Badge', 'Success')
        
    else:
        try:
            rawData=BadUSBTranslation.scriptToHex(scriptPath)
            if(len(rawData)>=2040):
                Messagebox.show_error('The script is too long, please shorten the script under 2KiB', 'Error')
                return
            print(rawData)
            #print raw data as hex
            print(' '.join(format(x, '02x') for x in rawData))
            STM32HID.clear_badusb()
            STM32HID.send_badusb_script(rawData)
            Messagebox.show_info('BadUSB Script has been written to the Badge', 'Success')
        except:
            Messagebox.show_error('Please Connect the PCB Badge to the Computer and try again', 'Error')
            checkBadgeConnection()

# Get the zoom value of the system
ZOOM_VALUE = getZoomValue()


# Create the main window
root = ttk.Window('HITCON PCB Badge Commander', 'cyborg')
#apply app icon from base64 imageres.HITCONICON
Icon=ttk.PhotoImage(data=imageres.HITCONICON)
root.iconphoto(False, Icon)
#apply app icon to all windows
root.iconphoto(True, Icon)


# Set the window size
windowWidth = int(500 * ZOOM_VALUE)
windowHeight = int(290 * ZOOM_VALUE)
root.geometry(f'{windowWidth}x{windowHeight}')
root.resizable(False, False)

OutSideFrame = ttk.Frame(root)
OutSideFrame.pack(fill='both', expand=True, padx=int(10*ZOOM_VALUE), pady=int(10*ZOOM_VALUE))

# Load the custom font
custom_font = tkFont.Font(family='SquareFont', size=12)
tkFont.nametofont("TkDefaultFont").configure(family="SquareFont", size=12)

print(ZOOM_VALUE)

# ImageLabel
if ZOOM_VALUE == 1:
    HITCONImage = ttk.PhotoImage(data=imageres.HITCON100x)
elif ZOOM_VALUE == 1.25:
    HITCONImage = ttk.PhotoImage(data=imageres.HITCON100x)
elif ZOOM_VALUE == 1.5:
    HITCONImage = ttk.PhotoImage(data=imageres.HITCON150x)
elif ZOOM_VALUE == 1.75:
    HITCONImage = ttk.PhotoImage(data=imageres.HITCON150x)
elif ZOOM_VALUE == 2:
    HITCONImage = ttk.PhotoImage(data=imageres.HITCON175x)
    print('2')
else:
    HITCONImage = ttk.PhotoImage(data=imageres.HITCON100x)
    

ImageLabel = ttk.Label(OutSideFrame, image=HITCONImage)
ImageLabel.pack(side='left', padx=int(10*ZOOM_VALUE), pady=int(10*ZOOM_VALUE))

notebookFrame = ttk.Frame(OutSideFrame)
notebookFrame.pack(side='right', fill='both', expand=True)

titleLabel = ttk.Label(notebookFrame, text='HITCON PCB Badge \nCommander', font=('SquareFont', 20))
titleLabel.pack(side='top', fill='x', padx=int(10*ZOOM_VALUE), pady=int(10*ZOOM_VALUE))


# Create the Notebook for function Selection
notebook = ttk.Notebook(notebookFrame, bootstyle='light')

#SetName Frame
SetNameFrame = ttk.Frame()
nameLabel = ttk.Label(SetNameFrame, text='Please Enter Your Name:', font=('SquareFont', 12))
nameLabel.pack(side='top', fill='x', padx=int(10*ZOOM_VALUE), pady=int(10*ZOOM_VALUE))

entryFrame = ttk.Frame(SetNameFrame)
entryFrame.pack(side='top', fill='x', padx=int(10*ZOOM_VALUE))
entry_text = tk.StringVar()
nameEntry = ttk.Entry(entryFrame, textvariable = entry_text, font=('Arial',16))
nameEntry.pack(side='top',fill='x', ipadx=int(10*ZOOM_VALUE), ipady=int(5*ZOOM_VALUE))
#setnameButton
setNameButton = ttk.Button(SetNameFrame, text='Set Name', command=lambda: setName(entry_text.get()), bootstyle='light')
setNameButton.pack(side='right', padx=int(10*ZOOM_VALUE), pady=int(10*ZOOM_VALUE), ipadx=int(10*ZOOM_VALUE), ipady=int(10*ZOOM_VALUE))

#Limit the character length of the nameEntry
def character_limit(entry_text):
    # 限制長度
    if len(entry_text.get()) > 22:
        entry_text.set(entry_text.get()[:22])
    # 只允許 ASCII 字母和符號
    valid_text = re.sub(r'[^ -~]', '', entry_text.get())
    if entry_text.get() != valid_text:
        entry_text.set(valid_text)
        
        
entry_text.trace_add("write", lambda *args: character_limit(entry_text))

# Naming Rules
ruleLabel = ttk.Label(SetNameFrame, text='ASCII Character, Symbol Only', font=('SquareFont',10))
ruleLabel.pack(side='top', fill='x', padx=int(10*ZOOM_VALUE), pady=int(10*ZOOM_VALUE))


BadUSBFrame = ttk.Frame()
badUSBLabel = ttk.Label(BadUSBFrame, text='Please Select the BadUSB script File:', font=('SquareFont', 12))
badUSBLabel.pack(side='top', fill='x', padx=int(10*ZOOM_VALUE), pady=int(10*ZOOM_VALUE))
scriptPathFrame = ttk.Frame(BadUSBFrame)
scriptPathFrame.pack(side='top', fill='x', padx=int(10*ZOOM_VALUE))
scriptPathLabel = ttk.Label(scriptPathFrame, text='Please Select Script File', font=('SquareFont', 10), bootstyle='light', width=25)
scriptPathLabel.pack(side='left', fill='x', expand=True, ipadx=int(10*ZOOM_VALUE), ipady=int(5*ZOOM_VALUE))
scriptBrowseButton = ttk.Button(scriptPathFrame, text='Browse', command=lambda: scriptPathLabel.config(text=filedialog.askopenfilename()), bootstyle='light')
scriptBrowseButton.pack(side='right', ipadx=int(10*ZOOM_VALUE), ipady=int(5*ZOOM_VALUE))
writeButton = ttk.Button(BadUSBFrame, text='Write Script to Badge', command=lambda: writeBadUSB(scriptPathLabel.cget('text')), bootstyle='light')
writeButton.pack(side='right', padx=int(10*ZOOM_VALUE), pady=int(10*ZOOM_VALUE), ipadx=int(10*ZOOM_VALUE), ipady=int(0*ZOOM_VALUE))

memoryFrame = ttk.Frame()
memoryLabel = ttk.Label(memoryFrame, text='Address:          Content:        Type:', font=('SquareFont', 12))
memoryLabel.pack(side='top', fill='x', padx=int(10*ZOOM_VALUE), pady=int(10*ZOOM_VALUE))
memoryEntryFrame = ttk.Frame(memoryFrame)
memoryEntryFrame.pack(side='top', fill='x', padx=int(10*ZOOM_VALUE))
memoryEntryPrefix = ttk.Label(memoryEntryFrame, text='0x', font=('SquareFont', 12))
memoryEntryPrefix.pack(side='left')

memoryEntry_text = tk.StringVar()
memoryEntry = ttk.Entry(memoryEntryFrame, textvariable=memoryEntry_text, font=('SquareFont', 12), width=6)
memoryEntry.pack(side='left', ipadx=int(10*ZOOM_VALUE))

#Limit the character length of the nameEntry
def character_limit4memory(entry_text):
    # 限制長度
    if len(entry_text.get()) > 8:
        entry_text.set(entry_text.get()[:8])
    # 只允許 hex 字母和符號
    valid_text = re.sub(r'[^0-9A-Fa-f]', '', entry_text.get())
    if entry_text.get() != valid_text:
        entry_text.set(valid_text)
        
memoryEntry_text.trace_add("write", lambda *args: character_limit4memory(memoryEntry_text))

memoryContent_text = tk.StringVar()
contentEntryPrefix = ttk.Label(memoryEntryFrame, text=' 0x', font=('SquareFont', 12))
contentEntryPrefix.pack(side='left')
contentEntry = ttk.Entry(memoryEntryFrame, textvariable=memoryContent_text, font=('SquareFont', 12), width=6)
contentEntry.pack(side='left', ipadx=int(10*ZOOM_VALUE))

#Limit the character length of the nameEntry
def character_limit4memoryContent(entry_text):
    # 限制長度
    if len(entry_text.get()) > 8:
        entry_text.set(entry_text.get()[:8])
    # 只允許 hex 字母和符號
    valid_text = re.sub(r'[^0-9A-Fa-f]', '', entry_text.get())
    if entry_text.get() != valid_text:
        entry_text.set(valid_text)
        
memoryContent_text.trace_add("write", lambda *args: character_limit4memoryContent(memoryContent_text))
typeCombobox = ttk.Combobox(memoryEntryFrame, values=['Byte(8bit)', 'Hfwd(16bit)', 'Word(32bit)'], font=('SquareFont', 12), width=8)
typeCombobox.pack(side='right', ipadx=int(10*ZOOM_VALUE))

memoryButtonFrame = ttk.Frame(memoryFrame)
memoryButtonFrame.pack(side='top', fill='x')
readButton = ttk.Button(memoryButtonFrame, text='Read Memory', bootstyle='light', command=lambda: readMemory(memoryEntry_text.get(), typeCombobox.current()))
readButton.pack(side='left', padx=int(10*ZOOM_VALUE), pady=int(10*ZOOM_VALUE), ipadx=int(10*ZOOM_VALUE), ipady=int(0*ZOOM_VALUE))
writeButton = ttk.Button(memoryButtonFrame, text='Write Memory', bootstyle='light', command=lambda: writeMemory(memoryEntry_text.get(), memoryContent_text.get(), typeCombobox.current()))
writeButton.pack(side='right', padx=int(10*ZOOM_VALUE), pady=int(10*ZOOM_VALUE), ipadx=int(10*ZOOM_VALUE), ipady=int(0*ZOOM_VALUE))



notebook.add(SetNameFrame, text='  Set Name  ', padding=int(10*ZOOM_VALUE))
notebook.add(BadUSBFrame, text='  BadUSB  ', padding=int(10*ZOOM_VALUE))
notebook.add(memoryFrame, text='  Memory Browser  ', padding=int(10*ZOOM_VALUE))
notebook.pack(side='bottom', fill='both', expand=True, padx=int(10*ZOOM_VALUE), pady=int(10*ZOOM_VALUE))

# Apply the custom font to widgets
# for frame in (SetNameFrame, BadUSBFrame):
#     for widget in frame.winfo_children():
#         widget['font'] = custom_font

checkBadgeConnection()


root.mainloop()
