#!/bin/sh
# test all examples

for file in *.htex ; do
  clear ; echo "This code from \`$file'..." ; cat "$file"
  read -p "Press any key to continue..." -n 1
  clear ; echo "Produce this results..." ; ../htmlex "$file"
  read -p "Press any key to continue..." -n 1
done
