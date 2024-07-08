#!/bin/bash

TARGET_DIR="$1/fw/Core"
TARGET_FILES=$(find $TARGET_DIR -type f -name "*.cc" -o -name "*.cpp" -o -name "*.c")

for file in "${TARGET_FILES[@]}"; do
  clang-format --dry-run --Werror $file &> /dev/null
  ret=$?
  if [[ $ret -ne 0 ]]; then
    echo "Fail on $file"
    exit $ret
  fi
done
