#!/bin/bash

# Parse Argument
while getopts ":i" opt; do
  case "$opt" in
    i)
      auto_fmt=true
      shift
      ;;
    *)
      echo "Option $opt not exists."
      exit 1
      ;;
  esac
done

if [[ "$1" == "" ]]; then
  TARGET_DIR="./fw/Core"
else
  TARGET_DIR="$1/fw/Core"
fi

TARGET_FILES=$(find $TARGET_DIR -type f -name "*.cc" -o -name "*.cpp" -o -name "*.c")

if $auto_fmt; then 
  for file in "${TARGET_FILES[@]}"; do
    echo "Formatting $file"
    clang-format -i $file &> /dev/null
  done
else
  for file in "${TARGET_FILES[@]}"; do
    clang-format --dry-run --Werror $file &> /dev/null
    ret=$?
    if [[ $ret -ne 0 ]]; then
      echo "Fail on $file"
      exit $ret
    fi
  done
fi
