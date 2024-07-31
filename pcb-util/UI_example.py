# -*- coding: utf-8 -*-
"""
    
============ UI with auto detect/refresh for ST-Link -devices ================ 
"""
# TODO : besides board detect, we should also check STLINK connection

import curses
import threading
import time
import streamlit as st
import os
import subprocess

import fw_flash
import setting


# ------- Initialize at App start-up ---------
    
# const value
# ------- this part can be changed by frontend ----------
if 'FW_ELF_PATH' not in st.session_state:
        st.session_state.FW_ELF_PATH = "C:\\Users\\a8701\\Documents\\Development\\hitcon-pcb-badge\\pcb-util\\fw.elf"
if 'ST_PRO_PATH' not in st.session_state:
        st.session_state.ST_PRO_PATH = (
    "C:\\Program Files (x86)\\STMicroelectronics\\STM32Cube\\STM32CubeProgrammer\\bin"
)
if 'ST_PRO_EXE' not in st.session_state:
        st.session_state.ST_PRO_EXE = "STM32_Programmer_CLI.exe"
# ------- this part can be changed by frontend ----------

def thread_create(st_obj):
    
    if 'stop_event' not in st.session_state:
        st.session_state.stop_event = threading.Event()
    if 'my_thread' not in st.session_state:
        st.session_state.my_thread = threading.Thread(
        target=st_obj.bg_daemon, args=(st.session_state.stop_event,)  # here is comma `,` to create tuple
    )
    st.session_state.my_thread.daemon = True
    st.session_state.my_thread.start()
    return st.session_state.stop_event, st.session_state.my_thread


# ------- Setup STLink Devices ----------

# ------- Initialized Multidrop Threads ----------
if 'thread_pool' not in st.session_state:
    st.session_state.thread_pool = []  # store all thread instance, a thread == a STLINK work loop
if 'stop_event_pool' not in st.session_state:
    st.session_state.stop_event_pool = []  # store all stop event, to stop the thread of specific STLINK
if 'stlink_alive_sn_list' not in st.session_state:
    st.session_state.stlink_alive_sn_list = []  # store all current alive STLINK(SN) in every loop

# ------- Initialized Multidrop Threads Finished ----------


## setup shared value to class
if 'shared_info' not in st.session_state:
    st.session_state.shared_info = fw_flash.ST_CONFIG(
    ELF_PATH=st.session_state.FW_ELF_PATH, 
    ST_PRO_PATH=st.session_state.ST_PRO_PATH, 
    ST_PRO_EXE=st.session_state.ST_PRO_EXE
)
fw_flash.STLINK.initialize_shared(st.session_state.shared_info)

# init all object, depend on how many STLINK(SN) we have
# these are devices connected BEFORE program start

if 'stlink_sn_list' not in st.session_state:
    st.session_state.stlink_sn_list = st.session_state.shared_info.list_stlink()
st.session_state.stlink_alive_sn_list = st.session_state.stlink_sn_list

if 'st_obj_list' not in st.session_state:
    st.session_state.st_obj_list = []

print(f"init with : {st.session_state.stlink_sn_list}")
for init_sn in st.session_state.stlink_sn_list:
    st.session_state.st_obj_list.append(fw_flash.STLINK(init_sn))

# start all object daemon and store in pool
for st_obj in st.session_state.st_obj_list:
    st.session_state.stop_event, thread_instance = thread_create(st_obj)
    st.session_state.stop_event_pool.append(st.session_state.stop_event)
    st.session_state.thread_pool.append(thread_instance)

# ------- Finished Setup STLink Devices ----------


# ------- Initialization Completed ----------



# ------- Setup Streamlit App Layout ----------

## List of color assigned to each fw_flash.ST_STATUS
color = ["red", "orange", "yellow", "gray", "green"]

# Delcare app Layout
description = st.container()
path = st.container()

st.header("Status")
status = st.empty()
st.markdown("### Flash Progress")
progress_bar = st.empty()
status_text = st.empty()

st.header("Simulation")
st.write("Simulate progress action")
simulation = st.container()
st.header("Active ST-Link List")
STLinkList= st.empty()
cli = st.container()

# ------- Finished Setup Streamlit App Layout ----------



# Define the function to update fw_flash.ST_STATUS display in UI
def update_state(state_to_update):
    if len(st.session_state.st_obj_list) < 1:
        st.error("No ST-Link Detected!")
    else:
        st.session_state.box_color = color[st.session_state.st_obj_list[0].current_state]
    
    with status:
        st.markdown(
            f"""
            <div style="
                border: 1px solid #ccc;
                padding: 50px;
                border-radius: 5px;
                height: 50px;
                background-color: {st.session_state.get('box_color', 'red')};
            ">
            </div>
            """,
            unsafe_allow_html=True
        )
    with progress_bar:
        if fw_flash.ST_STATUS(state_to_update) == fw_flash.ST_STATUS.FINISHED:
            st.progress(1.0, text = "FW flash completed")
        else:
            st.progress((state_to_update)*(1/(len(fw_flash.ST_STATUS)-1)), text = "FW flashing......")
    with status_text:
        st.write(str(fw_flash.ST_STATUS(state_to_update).name))

# ------- Display initial message ----------

with description:
    st.header("HITCON 2024 FW Flasher(Singledrop)")
    st.write("Click the buttons to flash firmware with ST-Link")
with status:
        st.markdown(
            f"""
            <div style="
                border: 1px solid #ccc;
                padding: 50px;
                border-radius: 5px;
                height: 50px;
                background-color: {st.session_state.get('box_color', 'red')};
            ">
            </div>
            """,
            unsafe_allow_html=True
        )
with status_text:
    st.write(str(fw_flash.ST_STATUS(0).name))
    
# ------- Display initial message Finished ----------



# ------- Update FW/ST-LINK_Programmer CLI Path ----------

# Set path of the files
with path:
    FW_ELF_PATH_temp = st.text_input("Path of FW to be flashed (.elf) ", key="FWpath")
    
    # Check if the file name ends with ".elf"
    if FW_ELF_PATH_temp.endswith(".elf"):
        st.success("valid path")
    else:
        st.error("FW path should end with .elf")
    
    ST_PROGRAMMER_PATH_temp = st.text_input("Directory Path of ST programmer(.exe)", key="STpath")
    
    ST_PRO_PATH_temp, ST_PRO_EXE_temp = os.path.split(ST_PROGRAMMER_PATH_temp)
    # Check if the file name ends with ".exe"
    if ST_PRO_EXE_temp.endswith(".exe"):
        st.success("valid path")
    else:
        st.error("ST_PROGRAMMER path should end with .exe")
    
    if st.button("Refresh Path", key="RefreshPath"):
        st.session_state.FW_ELF_PATH = FW_ELF_PATH_temp
        st.session_state.ST_PRO_PATH = ST_PRO_PATH_temp
        st.session_state.ST_PRO_EXE = ST_PRO_EXE_temp

    # Test if path is correctly parsed  
    st.markdown("**FW_ELF_PATH**")
    st.write(st.session_state.FW_ELF_PATH)
    st.markdown("**ST_PRO_PATH**")
    st.write(st.session_state.ST_PRO_PATH)
    st.markdown("**ST_PRO_EXE**")
    st.write(st.session_state.ST_PRO_EXE)

# ------- Update FW/ST-LINK_Programmer CLI Path Finished ----------



# ------- Scan for ST-Link change ----------

# check if stlink list changed
if 'stlink_add_list' not in st.session_state:
    st.session_state.stlink_add_list = list(set(st.session_state.stlink_alive_sn_list) - set(st.session_state.stlink_sn_list))
if 'stlink_del_list' not in st.session_state:
    st.session_state.stlink_del_list = list(set(st.session_state.stlink_sn_list) - set(st.session_state.stlink_alive_sn_list))

# if somthing changed, object list update
## add new object
if st.session_state.stlink_add_list:
    print("add : " + str(st.session_state.stlink_add_list))
    # for new devices
    for add_sn in st.session_state.stlink_add_list:
        # do object add
        st.session_state.st_obj_list.append(fw_flash.STLINK(add_sn))
        st.session_state.stop_event, thread_instance = thread_create(st.session_state.st_obj_list[-1])
        st.session_state.stop_event_pool.append(st.session_state.stop_event)
        st.session_state.thread_pool.append(thread_instance)
## del object
if st.session_state.stlink_del_list:
    print("del : " + str(st.session_state.stlink_del_list))
    # do object del
    for del_sn in st.session_state.stlink_del_list:
        for index, st_obj in enumerate(st.session_state.st_obj_list):
            if st_obj.SN == del_sn:
                st.session_state.st_obj_list.pop(index)
                # stop the thread
                st.session_state.stop_event_pool[index].set()
                st.session_state.thread_pool[index].join()
                break

# update the list
st.session_state.stlink_sn_list = st.session_state.stlink_alive_sn_list

# wait for all thread to stop
for thread in st.session_state.thread_pool:
    thread.join()

# ------- Scan for ST-Link change Finished ----------



# ------- Update ST-Link change ----------

with STLinkList:
    st.write(st.session_state.stlink_alive_sn_list)

# ------- Update ST-Link change Finished ----------



# ------- Other App Function ----------

# Spawn a CLI subprocess
def run_cli_app():
    subprocess.run(["python", "cli_example.py"])
if st.button("Start CLI App"):
    run_cli_app()

# Simulate the UI action as actual use case
with simulation:
    if st.button("Simulation", key="Simulation"):
        for i in range(0,5):
            update_state(i)
            time.sleep(0.5)
