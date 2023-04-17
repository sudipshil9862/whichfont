#!/bin/bash

rm -f test_results.txt
./build.sh

test_cases=(
    "u+0985"
    "U+0985"
    "U+0985sudiphex"
    "U+sudip"
    "0x0985"
    "0X0985"
    "0x0985sudiphex"
    "0xsudip"
    "0985"
    "-a 0985"
    "-s 0985"
    "😀"
    "-a 😀"
    "-s 😀"
    "অah😀"
    "-a অah😀"
    "-s অah😀"
    "A"
    "-a A"
    "-s A"
)

output_file="test_results.txt"

echo "date-time: $(date '+%Y-%m-%d %H:%M:%S')" >> $output_file
echo >> $output_file

for test_case in "${test_cases[@]}"
do
    command_output=$(./.builddir/whichfont $test_case)
    echo "input: ./.builddir/whichfont $test_case" >> $output_file
    echo "output: $command_output" >> $output_file
    echo "-------------------------------------------------------------------------------------------------------------------------------------" >> $output_file
done