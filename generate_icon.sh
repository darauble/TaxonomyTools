#!/bin/sh
# Generate embedded icon header file
# Usage: generate_icon.sh <input_png> <output_header> <array_name>

INPUT_PNG="$1"
OUTPUT_HEADER="$2"
ARRAY_NAME="$3"

# Generate xxd output
xxd -i "$INPUT_PNG" | \
    sed "s/unsigned char [^[]*/unsigned char ${ARRAY_NAME}/" | \
    sed "s/unsigned int [^=]*/unsigned int ${ARRAY_NAME}_len/" > "${OUTPUT_HEADER}.tmp"

# Create final header with pragma once
echo '#pragma once' > "$OUTPUT_HEADER"
echo '' >> "$OUTPUT_HEADER"
echo "// Embedded icon data generated from $INPUT_PNG" >> "$OUTPUT_HEADER"
cat "${OUTPUT_HEADER}.tmp" >> "$OUTPUT_HEADER"

# Cleanup
rm -f "${OUTPUT_HEADER}.tmp"
