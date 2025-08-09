
from enum import Enum, auto
import streamlit as st
import time
import os
import subprocess

# status enum class
class ST_STATUS(Enum):
    NO_DEVICE = 0
    UPLOADING = auto()
    VERIFYING = auto()
    TRIGGER_EXEC = auto()
    FINISHED = auto()

# List of color assigned to each ST_STATUS
color = ["red", "orange", "yellow", "gray", "green"]

# Delcare app Layout
description = st.container()
path = st.container()

status = st.empty()
progress_text = st.container()
progress_bar = st.empty()
status_text = st.empty()

button = st.container()
simulation = st.container()

# Define the function to update ST_STATUS display in UI
def update_state(state_to_update):
    st.session_state.box_color = color[state_to_update]

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
        if ST_STATUS(state_to_update) == ST_STATUS.FINISHED:
            st.progress(1.0, text = "FW flash completed")
        else:
            st.progress((state_to_update)*(1/(len(ST_STATUS)-1)), text = "FW flashing......")
    with status_text:
        st.write(str(ST_STATUS(state_to_update).name))

# Display initial message
with description:
    st.header("HITCON 2025 FW Flasher(Singledrop)")
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
    st.write(str(ST_STATUS(0).name))
with progress_text:
    st.markdown("### Flash Progress")

# Set path of the files
with path:
    FW_ELF_PATH = st.text_input("Path of FW to be flashed (.elf) ")

    # Check if the file name ends with ".elf"
    if FW_ELF_PATH.endswith(".elf"):
        st.success("valid path")
    else:
        st.error("FW path should end with .elf")

    ST_PROGRAMMER_PATH = st.text_input("Directory Path of ST programmer(.exe)")

    ST_PRO_PATH, ST_PRO_EXE = os.path.split(ST_PROGRAMMER_PATH)
    # Check if the file name ends with ".exe"
    if ST_PRO_EXE.endswith(".exe"):
        st.success("valid path")
    else:
        st.error("ST_PROGRAMMER path should end with .exe")

    # Test if path is correctly parsed
    st.markdown("**FW_ELF_PATH**")
    st.write(FW_ELF_PATH)
    st.markdown("**ST_PRO_PATH**")
    st.write(ST_PRO_PATH)
    st.markdown("**ST_PRO_EXE**")
    st.write(ST_PRO_EXE)

# UI test inputs for different stage
with button:
    col1, col2, col3, col4, col5 = st.columns(5)
    with col1:
        if st.button("NO_DEVICE"):
            update_state(ST_STATUS.NO_DEVICE.value)
    with col2:
        if st.button("UPLOADING"):
            update_state(ST_STATUS.UPLOADING.value)
    with col3:
        if st.button("VERIFYING"):
            update_state(ST_STATUS.VERIFYING.value)
    with col4:
        if st.button("TRIGGER_EXEC"):
            update_state(ST_STATUS.TRIGGER_EXEC.value)
    with col5:
        if st.button("FINISHED"):
            update_state(ST_STATUS.FINISHED.value)


# Simulate the UI action as actual use case
with simulation:
    st.header("Simulation")
    st.write("Simulate progress action")
    if st.button("Simulation"):
        for i in range(0,5):
            update_state(i)
            time.sleep(0.5)