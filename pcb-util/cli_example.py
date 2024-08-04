# -*- coding: utf-8 -*-
"""
Created on Fri Jul 12 14:16:46 2024
@author: Arthur (Tora0615)
    
============ CLI with auto detect/refresh in multi-devices ================ 
"""
# TODO : besides board detect, we should also check STLINK connection

import curses
import threading
import time
import subprocess

import fw_flash
import setting

# const value
# ------- this part can be changed by frontend ----------
FW_ELF_PATH = "C:\\Users\\a8701\\Documents\\Development\\hitcon-pcb-badge\\pcb-util\\fw.elf"
ST_PRO_PATH = (
    "C:\\Program Files (x86)\\STMicroelectronics\\STM32Cube\\STM32CubeProgrammer\\bin"
)
ST_PRO_EXE = "STM32_Programmer_CLI.exe"
# ------- this part can be changed by frontend ----------


def thread_create(st_obj):
    #
    stop_event = threading.Event()
    my_thread = threading.Thread(
        target=st_obj.bg_daemon, args=(stop_event,)  # here is comma `,` to create tuple
    )
    my_thread.daemon = True
    my_thread.start()
    return stop_event, my_thread


def main_loop(stdscr):
    """main logic for cli interactive interface"""

    thread_pool = []  # store all thread instance, a thread == a STLINK work loop
    stop_event_pool = []  # store all stop event, to stop the thread of specific STLINK
    stlink_alive_sn_list = []  # store all current alive STLINK(SN) in every loop

    # curses init
    stdscr.clear()
    stdscr.nodelay(True)  # non-blocking mode, only wait $timeout when meet getch()
    stdscr.timeout(int(setting.CLI_QUIT_SCAN_INTERVAL * 1000))  # in ms

    # setup shared value to class
    shared_info = fw_flash.ST_CONFIG(
        ELF_PATH=FW_ELF_PATH, ST_PRO_PATH=ST_PRO_PATH, ST_PRO_EXE=ST_PRO_EXE
    )
    fw_flash.STLINK.initialize_shared(shared_info)

    # init all object, depend on how many STLINK(SN) we have
    # these are devices connected BEFORE program start
    stlink_sn_list = shared_info.list_stlink()
    stlink_alive_sn_list = stlink_sn_list
    st_obj_list = []
    print(f"init with : {stlink_sn_list}")
    for init_sn in stlink_sn_list:
        st_obj_list.append(fw_flash.STLINK(init_sn))

    # start all object daemon and store in pool
    for st_obj in st_obj_list:
        stop_event, thread_instance = thread_create(st_obj)
        stop_event_pool.append(stop_event)
        thread_pool.append(thread_instance)

    # scan for stlink change
    
    start_time = time.time()
    print("start_time =")
    print(start_time)
    while True:
        # input scan of list and quit
        input_cmd = stdscr.getch()
        ## list all current avaliable stlink
        # TODO : auto detect STLINK connection -----> have bug in this part, postponded
        
        #if time.time()-start_time >1 :
        #    stlink_alive_sn_list = shared_info.list_stlink()

        if input_cmd == ord("r"):
            stlink_alive_sn_list = shared_info.list_stlink()
        ## quit the program
        if input_cmd == ord("q"):
            count = 0
            # set all stop event
            for stop_event in stop_event_pool:
                stop_event.set()
                count += 1
            print(f"exit thread: {count}")
            # break the loop
            break

        # curses display
        stdscr.refresh()
        stdscr.addstr(
            0, 0, "=== STM32 upload tool ('q' to exit, 'r' to refersh all) ==="
        )

        # check if stlink list changed
        stlink_add_list = list(set(stlink_alive_sn_list) - set(stlink_sn_list))
        stlink_del_list = list(set(stlink_sn_list) - set(stlink_alive_sn_list))

        # if somthing changed, object list update
        ## add new object
        if stlink_add_list:
            print("add : " + str(stlink_add_list))
            # for new devices
            for add_sn in stlink_add_list:
                # do object add
                st_obj_list.append(fw_flash.STLINK(add_sn))
                stop_event, thread_instance = thread_create(st_obj_list[-1])
                stop_event_pool.append(stop_event)
                thread_pool.append(thread_instance)
            
        ## del object
        if stlink_del_list:
            print("del : " + str(stlink_del_list))
            # do object del
            for del_sn in stlink_del_list:
                for index, st_obj in enumerate(st_obj_list):
                    if st_obj.SN == del_sn:
                        st_obj_list.pop(index)
                        # stop the thread
                        stop_event_pool[index].set()
                        thread_pool[index].join()
                        break

        # refresh the window
        ## for exist lines
        for index, st_obj in enumerate(st_obj_list):
            stdscr.addstr(
                setting.CURSES_RESERVE_LINE + index,
                0,
                f"ST-00{index}({st_obj.SN}) : "
                + str(st_obj.current_state)
                + " " * stdscr.getmaxyx()[1],
            )
        ## for empty linesl
        for i in range(setting.MAX_ST_QTY - len(st_obj_list)):
            stdscr.addstr(
                setting.CURSES_RESERVE_LINE + len(st_obj_list) + i,
                0,
                f"ST-00{len(st_obj_list)+i}() : " + " " * stdscr.getmaxyx()[1],
            )

        # update the list
        stlink_sn_list = stlink_alive_sn_list

    # wait for all thread to stop if we quit while loop
    for thread in thread_pool:
        thread.join()


if __name__ == "__main__":
    curses.wrapper(main_loop)  # type: ignore [attr-defined]
    