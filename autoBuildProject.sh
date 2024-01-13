#!/bin/bash

echo "=================================== Building Project ==================================================="
echo "starting to build project in [./build] folder"
echo " "

echo "- - - - - - - - - - - - - - - - - - - - remove previous content - - - - - - - - - - - - - - - - - - - -"
echo " "

build_dir="build"


if [ -d "$build_dir" ]; then
    # Directory exists, remove its contents
    rm -R "$build_dir"/*
else
    # Directory doesn't exist, create it
    mkdir "$build_dir"
fi

cd build

echo "- - - - - - - - - - - - - - - - - - - - state [cmake] command - - - - - - - - - - - - - - - - - - - - -"
echo " "
cmake -DCMAKE_BUILD_TYPE=Debug ..

echo " "
echo "- - - - - - - - - - - - - - - - - - - - starting [make] command - - - - - - - - - - - - - - - - - - - -"
echo " "

build_output="build_output.txt"

# Run make, preserving colors, and redirect output to both terminal and file
script -q -c "make" "$build_output"

# Count the number of errors and warnings
errors=$(grep -c "error:" $build_output)
warnings=$(grep -c "warning:" $build_output)

cd ..

# Print result of make command
echo " "
echo "--------------------------------------------------------------------------------------------------------"
echo -n "[Errors: $errors] [Warnings: $warnings] ==> "

if [ $errors -gt 0 ]; then
    echo -e "\033[0;31mBUILD FAILED\033[0m"  # Echo in red
elif [ $warnings -gt 0 ]; then
    echo -e "\033[0;33mBUILD SUCCESSFULL (with warnings)\033[0m"  # Echo in orange
else
    echo -e "\033[0;32mBUILD SUCCESSFULL\033[0m"  # Echo in green
fi
echo "===================================  Finished Build  ==================================================="

