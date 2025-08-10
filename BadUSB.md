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

## Example (set name)

```bash
pip install hidapi
```

```python
import hid

FUNC_SET_NAME = 1
FUNC_CLR_SCRIPT = 2
FUNC_WR_SCRIPT = 3
FUNC_WR_MEM = 4
FUNC_RD_MEM = 5

vendor_id = 1155
product_id = 22352

device = hid.device()
device.open(vendor_id, product_id)

NAME = list("Tuzki".encode())
NAME_LEN = 22
NAME += (NAME_LEN-len(NAME) + 1)*[0]
data = [FUNC_SET_NAME] + NAME
device.write([0] + data)
```
