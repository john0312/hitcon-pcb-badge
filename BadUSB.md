# BadUSB

(in hex)

```python
device.write(arr)
```

The first byte of `arr` is the Report ID (value 2).  
If `len(arr) > 9`, every 10th, 20th, etc., byte is also a Report ID (2) and will be ignored.  
This means that for the first nine elements of `arr`, there are eight data bytes. Starting from the 10th byte, each segment consists of one Report ID followed by nine data bytes.
**The simplest approach is to always use an array of size 9, as the firmware is designed to handle this format.**
These results were tested on macOS; behavior on Linux may differ slightly but is generally the same.

- 01 XX XX XX XX XX… Set Name
- 02 Clear BadUSB Script
- 03 XX XX Start writing script + script length + CRC32 + ...

  - FF XX delay(20ms per unit)
  - FE XX Modifier
  - FD finish script
  - other: keycode
- 04 XX XX XX XX YY YY YY YY ZZ
  - XX write address
  - YY content
  - ZZ type, bytes, half word, or word

- 05 XX XX XX XX ZZ
  - XX read address
  - ZZ type, bytes, half word, or word

USB return

FF action done

[USB HID Keyboard scan codes](https://gist.github.com/MightyPork/6da26e382a7ad91b5496ee55fdc73db2)

## Windows 開發紀錄

BadUSB 的攻擊腳本是透過 HID 寫入 badge，在 mac 和 linux 上都可以寫入 hidraw，但 windows 上無法寫入。根據搜尋到的資料推測是 windows 不接受對一般鍵盤的 hid 界面寫入任意格式的資料，而 badge 已經模擬成鍵盤因此無法寫入。解決辦法是新增獨立的 usb interface，其中一個是鍵盤，另一個是自訂 hid，這樣 windows 就不會阻止對自訂 hid 的寫入操作了。

USB 裝置描述大致如下

Device Descriptor -> Configuration Descriptor -> Report Descriptor

### Device descriptor

宣告 vendor id (VID), product id (PID)

### Configuration Descriptor

這裡其實包含很多種 descriptor

- Configuration Descriptor * 1
  - Interface Descriptor * n
    - Class-Specific Descriptor: 根據 bInterfaceClass 緊接在後
    - Endpoint Descriptor * m

就是在這裡增加 interface 數量。修改時須參考 USB HID spec

### Report Descriptor

- Collection
  - Report * n

一個 interface 對應到一個 report descriptor，每個 report descriptor 中可能包含多個功能相近的 report，input (to PC) 和 output (from PC) 也是分開的 report。以鍵盤為例，input 是按鍵觸發事件，output 可以是 caps lock 亮燈。

### Debug 工具

```sh
sudo dmesg -w
```

顯示目前插入的 usb 裝置狀況

```sh
hexdump -C /sys/bus/usb/devices/3-1/3-1:1.0/0003:0483:5750.0007/report_descriptor
```

顯示 report descriptor，可丟 copilot 分析

```sh
sudo lsusb -v -d 0483:5750
```

顯示經過解析的 device descriptor, configuration descriptor 等

wireshark: 可以抓 usb 溝通過程

### References

- [使用STM32 H7实现HID组合设备（键盘、鼠标） - 来自Madliar](https://www.madliar.com/notebook/publish/i/caoliang.net/2023-08-27/-shi-yong-STM32H7-shi-xian-HID-zu-he-she-bei--jian-pan--shu-biao-.html)
- [Device Class Definition for Human Interface Devices (HID) version 1.11.pdf](https://www.usb.org/sites/default/files/documents/hid1_11.pdf)
- [Defined Class Codes | USB-IF](https://www.usb.org/defined-class-codes)
- [CaptureSetup/USB - Wireshark Wiki](https://wiki.wireshark.org/CaptureSetup/USB)
- [USB Component: Interface Descriptor](https://www.keil.com/pack/doc/mw/usb/html/_u_s_b__interface__descriptor.html)
- [【经验分享】基于STM32使用HAL库实现USB组合设备之多路CDC - STM32团队 ST意法半导体中文论坛](https://shequ.stmicroelectronics.cn/thread-634270-1-1.html)
