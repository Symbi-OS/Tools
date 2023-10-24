#!/bin/bash

# Define the path to your vmlinuz file
VMLINUZ_PATH=~/Symbi-OS/linux/vmlinux

# Temporary file to hold the updated values
TEMP_FILE=$(mktemp)

# Read each line from the input file
while IFS= read -r line; do
    # Check if the line contains an "=" symbol
    if [[ "$line" == *"="* ]]; then
        # Extract the name of the symbol (everything before the "=")
        symbol_name=$(echo "$line" | awk -F'=' '{print $1}' | tr -d ' ')

        # Use nm and grep to find the symbol in vmlinuz and store it in a variable
        symval=$(nm "$VMLINUZ_PATH" | grep -w "$symbol_name" | awk '{print $1}')
        
        # Write the new value to the temporary file
        echo "${symbol_name} = 0x${symval};"
        echo "${symbol_name} = 0x${symval};" >> "$TEMP_FILE"
    else
        # If the line does not contain "=", write it as-is to the temporary file
        echo "$line" >> "$TEMP_FILE"
    fi
done < "symhelper.ld"

# Replace the original file with the temporary file
mv "$TEMP_FILE" "symhelper.ld"