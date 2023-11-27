#!/bin/bash

template=$(cat test_template.cpp)

# Initialize arrays for includes and function calls
includes=()
calls=()

# Walking through directories and files
while IFS= read -r path; do
    abs_path=$(realpath "$path")

    # Append to includes
    includes+=("#include \"$abs_path\"")

    # Read lines from file
    while IFS= read -r line; do
        # Check for function names
        if [[ "$line" =~ Test[a-zA-Z0-9_]+\(\) ]]; then
            s=$(echo "$line" | sed -e 's/int //' -e 's/().*//')
            calls+=("{$s,\"$s\"}")
        fi
    done < "$path"
done < <(find . -type f -name '*_test.cpp')

# Replace placeholders in the template
num_tests=${#calls[@]}
num_threads=8
tests=$(IFS=, ; echo "${calls[*]}")
includes=$(IFS=$'\n' ; echo "${includes[*]}")

# Update the template
template=${template//'/*{@NUM_TESTS}*/'/"+$num_tests"}
template=${template//'/*{@NUM_THREADS}*/'/"+$num_threads"}
template=${template//'/*{@TESTS}*/'/$tests}
template=${template//'/*{@INCLUDES}*/'/$includes}

# Print the updated template
echo "$template" | while IFS= read -r line; do
    echo "$line"
done

