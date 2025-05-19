#!/bin/bash

rm -f test_results.txt test_error_results.txt
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
    "😀"
    "অah😀"
    "অঀ"
    "অ😀"
    "A"
    "😀$(echo -e "\e")ab"
    "$(echo -e "\xe2\x80\xa8")"
    "'　'"
    "　"

    "-s 0985"
    "-s 😀"
    "-s অah😀"
    "-s A"

    "-a -s"
    "-a -f"
    "-a -h"
    "-s -f"
    "-s -h"
    "-f -h"
    "-a -l"

    "--language ja"
    "--language xyz" #this should give error in test_error.txt - part of testing

    "--list-languages"
    "--help"

    "-f 'Noto Sans Bengali' 0985"

    "abc def"
    "-f 'Noto Sans' abc def"

    "abc :: family style familylang"
    "abc :: family"
    "abc def :: family familylang"
    "-f 'Noto Sans Bengali' abc def :: fanily familylang"
)

output_file="test_results.txt"
error_file="test_error_results.txt"
had_error=0

> "$output_file"
> "$error_file"

echo "date-time: $(date '+%Y-%m-%d %H:%M:%S')" >> "$output_file"
echo "date-time: $(date '+%Y-%m-%d %H:%M:%S')" >> "$error_file"
echo >> "$output_file"
echo >> "$error_file"

for test_case in "${test_cases[@]}"
do
    command_output=$(./.builddir/whichfont $test_case 2>tmp_stderr)
    exit_status=$?

    #segmentation fault (exit code 139)
    if [ $exit_status -eq 139 ]; then
        echo "input: ./.builddir/whichfont $test_case" >> "$error_file"
        echo "error: Segmentation fault (core dumped)" >> "$error_file"
        echo "-------------------------------------------------------------------------------------------------------------------------------------" >> "$error_file"
        had_error=1
    else
        echo "input: ./.builddir/whichfont $test_case" >> "$output_file"
        echo "output: $command_output" >> "$output_file"
        echo "-------------------------------------------------------------------------------------------------------------------------------------" >> "$output_file"

        error_output=$(<tmp_stderr)
        if [ -n "$error_output" ]; then
            echo "input: ./.builddir/whichfont $test_case" >> "$error_file"
            echo "error: $error_output" >> "$error_file"
            echo "-------------------------------------------------------------------------------------------------------------------------------------" >> "$error_file"
            had_error=1
        fi
    fi

    rm -f tmp_stderr
done

echo "All test cases completed. Results saved to $output_file"
if [ $had_error -eq 1 ]; then
    echo "⚠️  Some test cases produced errors. Please check $error_file for details."
fi
