#!/bin/bash

# Parse Argument
auto_fmt=false
while getopts ":i" opt; do
  case "$opt" in
    i)
      auto_fmt=true
      shift
      ;;
  esac
done

if [[ "$1" == "" ]]; then
  TARGET_DIR="./fw/Core/Hitcon"
else
  TARGET_DIR="$1/fw/Core/Hitcon"
fi

TARGET_FILES=$(find $TARGET_DIR -type f -name "*.cc" -o -name "*.cpp" -o -name "*.c")

if $auto_fmt; then 
  echo "Auto format"
  while read file; do
      echo "Formatting $file"
      clang-format -i $file &> /dev/null
  done <<< "$TARGET_FILES"
else
  echo "Check format"
  while read file; do
    echo $file
    clang-format --dry-run --Werror $file
    ret=$?
    if [[ $ret -ne 0 ]]; then
      echo "Fail on $file"
      exit $ret
    fi
  done <<< "$TARGET_FILES"
fi
