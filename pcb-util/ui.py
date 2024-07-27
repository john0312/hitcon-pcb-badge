
from enum import Enum, auto
import streamlit as st
import time

# status enum class
class ST_STATUS(Enum):
    NO_DEVICE = 0
    UPLOADING = auto()
    VERIFYING = auto()
    TRIGGER_EXEC = auto()
    FINISHED = auto()

description = st.container()
path = st.container()

status = st.empty()
status_text = st.empty()
progress_bar = st.empty()

button = st.container()
simulation = st.container()

color = ["red", "orange", "yellow", "blue", "green"]

def update_state(state_to_update):
    st.session_state.box_color = color[state_to_update]
    
    with status:
        st.markdown(
            f"""
            <div style="
                border: 1px solid #ccc;
                padding: 50px;
                border-radius: 5px;
                height: 300px;
                background-color: {st.session_state.get('box_color', 'red')};
            ">
            </div>
            """,
            unsafe_allow_html=True
        )
    with status_text:
        st.write(str(ST_STATUS(state_to_update).name))
    with progress_bar:
        st.progress((state_to_update)*(1/(len(ST_STATUS)-1)), text = "FW flashing......")
        

# Display initial message

with description:
    st.header("HITCON 2024 FW Flasher")
    st.write("Click the buttons to flash firmware with ST-Link")

with status:
        st.markdown(
            f"""
            <div style="
                border: 1px solid #ccc;
                padding: 50px;
                border-radius: 5px;
                height: 300px;
                background-color: {st.session_state.get('box_color', 'red')};
            ">
            </div>
            """,
            unsafe_allow_html=True
        )
with status_text:
    st.write(str(ST_STATUS(0).name))

# Set path of the files

with path:
    FW_ELF_PATH = st.text_input("FW .elf Path")
    ST_PRO_PATH = st.text_input("Directory Path of ST programmer")
    ST_PRO_EXE = st.text_input("Name of ST programmer(.exe)")

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