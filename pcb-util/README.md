# How to release
```
# cd into dev/
pyinstaller --noconfirm --onedir --console --add-data "fw.elf;."  "pcb-util_CLI.py"
# release the entire folder at dist/pcb-util_CLI
```

# Replace the ELF file

You don't have to rebuild. Just go to `release/_internal` and replace `fw.elf`.