# HOW TO RELEASE
```
# cd into dev/
pyinstaller --noconfirm --onedir --console --add-data "fw.elf;."  "pcb-util_CLI.py"
# release the entire folder at dist/pcb-util_CLI
```