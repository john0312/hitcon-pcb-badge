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
        import LinuxScale
        print(LinuxScale.get_scaling_factor())
        zoomValue = float(LinuxScale.get_scaling_factor())
    else:
        zoomValue = 1
    return zoomValue

def setName(Name):
    k=STM32HID.set_name(Name)
    
    if k==-1:
        #error
        checkBadgeConnection()
        
def writeBadUSB(scriptPath):
    try:
        rawData=BadUSBTranslation.scriptToHex(scriptPath)
        print(rawData)
        STM32HID.clear_badusb()
        STM32HID.send_badusb_script(rawData)
    except:
        Messagebox.show_error('Please Connect the PCB Badge to the Computer and try again', 'Error')
        checkBadgeConnection()

# Get the zoom value of the system
ZOOM_VALUE = getZoomValue()


# Create the main window
root = ttk.Window('HITCON PCB Badge Commander', 'cyborg')
windowWidth = int(500 * ZOOM_VALUE)
windowHeight = int(290 * ZOOM_VALUE)
root.geometry(f'{windowWidth}x{windowHeight}')
root.resizable(False, False)

OutSideFrame = ttk.Frame(root)
OutSideFrame.pack(fill='both', expand=True, padx=int(10*ZOOM_VALUE), pady=int(10*ZOOM_VALUE))

# Load the custom font
custom_font = tkFont.Font(family='SquareFont', size=12)
tkFont.nametofont("TkDefaultFont").configure(family="SquareFont", size=12)

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
    HITCONImage = ttk.PhotoImage(data=imageres.HITCON200x)
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
nameEntry = ttk.Entry(entryFrame, textvariable = entry_text, font=('SquareFont',16))
nameEntry.pack(side='top',fill='x', ipadx=int(10*ZOOM_VALUE), ipady=int(5*ZOOM_VALUE))
#setnameButton
setNameButton = ttk.Button(SetNameFrame, text='Set Name', command=lambda: setName(entry_text.get()), bootstyle='light')
setNameButton.pack(side='right', padx=int(10*ZOOM_VALUE), pady=int(10*ZOOM_VALUE), ipadx=int(10*ZOOM_VALUE), ipady=int(10*ZOOM_VALUE))

#Limit the character length of the nameEntry
def character_limit(entry_text):
    # 限制長度
    if len(entry_text.get()) > 16:
        entry_text.set(entry_text.get()[:16])
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



notebook.add(SetNameFrame, text='  Set Name  ', padding=int(10*ZOOM_VALUE))
notebook.add(BadUSBFrame, text='  BadUSB  ', padding=int(10*ZOOM_VALUE))
notebook.pack(side='bottom', fill='both', expand=True, padx=int(10*ZOOM_VALUE), pady=int(10*ZOOM_VALUE))

# Apply the custom font to widgets
# for frame in (SetNameFrame, BadUSBFrame):
#     for widget in frame.winfo_children():
#         widget['font'] = custom_font

checkBadgeConnection()


root.mainloop()
