(in hex)

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
