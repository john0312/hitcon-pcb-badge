# HITCON PCB Badge

## Project Setup

After cloning the repository, execute the following commands to prevent the environment from being checked by Git.

```shell
git config --local filter.eclipse_env_hash.clean "sed 's/env-hash=\"[^\"]*\"/env-hash=\"\"/g'"
git config --local filter.eclipse_env_hash.smudge cat
```

ref: [Eclipse Community Forums: C / C++ IDE (CDT) &raquo; Keeping language.settings.xml under source control?](https://www.eclipse.org/forums/index.php/t/1074031/)

## Firmware Development

execute `fw/merge.py` to merge STM32CubeIDE generated `main.c` with user code in `main.cc` into `main.cc`.
