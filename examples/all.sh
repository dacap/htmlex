#!/bin/sh
# test all examples

for file in *.htex ; do
  clear ; echo "This code from \`$file'..."
  cat "$file"
  echo -n "Press any key to continue..." ; read
  clear ; echo "Produce this results..."
  ../htmlex "$file"
  echo -n "Press any key to continue..." ; read
done
