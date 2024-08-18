- 01 XX XX XX XX XX… Set Name
- 02 Clear BadUSB Script
- 03 XX XX Start writing script + script length

  - FF XX delay(20ms per unit)
  - FE XX Modifier
  - FD finish script
  - FC if at first byte of data (data[0]) represents o
  - other: keycode

- 04 XX XX XX XX YY YY YY YY ZZ

  - XX write address
  - YY content
  - ZZ type, bytes, half word, or word

- 05 XX XX XX XX ZZ
  - XX read address
  - ZZ type, bytes, half word, or word

USB return

FF program partial done

FE erase finish

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
