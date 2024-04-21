#!/usr/bin/env python3
"""This script merge STM32CubeIDE generated main.c and our main.cc to main.cc

Old main.cc will be removed.
Code must be inside `USER CODE BEGIN` and `USER CODE END` comment
"""
from pathlib import Path

DIR = Path(__file__).parent


def main():
    main_c = Path(DIR, "Core", "Src", "main.c")
    main_cc = Path(DIR, "Core", "Src", "main.cc")
    main_new = Path(DIR, "Core", "Src", "new_main.cc")
    if not main_c.exists():
        print("No generated main.c found. Return and do nothing")
        return
    if not main_cc.exists():
        print("No main.cc found, rename main.c to main.cc")
        main_c.rename(main_cc)
        return
    user_code = {}
    with main_cc.open() as fin:
        key = None
        val = ""
        for line in fin:
            if key is None:
                if line.strip().startswith("/* USER CODE BEGIN "):
                    key = line.strip()[19:].rsplit(" ", 1)[0]
                    val = ""
            else:
                if line.strip().startswith("/* USER CODE END "):
                    user_code[key] = val
                    key = None
                else:
                    val += line
    with main_new.open("w") as fout, main_c.open() as fin:
        key = None
        val = ""
        for line in fin:
            if key is None:
                fout.write(line)
                if line.strip().startswith("/* USER CODE BEGIN "):
                    key = line.strip()[19:].rsplit(" ", 1)[0]
                    val = ""
            else:
                if line.strip().startswith("/* USER CODE END "):
                    val = val if user_code.get(key) is None else user_code[key]
                    key = None
                    fout.write(val)
                    fout.write(line)
                else:
                    val += line
    main_c.unlink()
    main_cc.unlink()
    main_new.rename(main_cc)


if __name__ == "__main__":
    main()
