#!/usr/bin/env python3

def main():
    buf = {}
    max_len = -1
    for i in range(8):
        buf[i] = input(f"Row [{i}]")
        str_len = len(buf[i])
        if max_len != -1 and max_len != str_len:
            exit("Length not align")
        else:
            max_len = str_len
    print("Packed buf:")
    for i in range(max_len):
        out = 0
        bit_str = ""
        for j in range(8):
            if buf[j][i] == "o":
                out += 1 * pow(2, j)
                bit_str += "1"
            else:
                bit_str += "0"
        print(f"{hex(out)}, /* {bit_str} */")

if __name__ == "__main__":
    main()
