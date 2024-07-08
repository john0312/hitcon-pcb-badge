#!/bin/bash

TARGET_DIR=$1
TARGET_FILES=$(find . -type f -name "*.cc" -o -name "*.cpp" -o -name "*.c")

for file in "${TARGET_FILES[@]}"; do
  clang-format --dry-run --Werror $file &> /dev/null
  ret=$?
  if [[ $ret -ne 0 ]]; then
    echo "Fail on $file"
    exit $ret
  fi
done
