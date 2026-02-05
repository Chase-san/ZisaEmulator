#!/usr/bin/env python3
"""
CMake shader processor - converts shader GLSL files to C header files
Usage: cmake_shader_gen.py <input_shader> <shader_name> <template> <output>

Outputs shader data as a char array with individual byte initializers
to avoid exceeding the C99/C17 string literal length limit (4095 chars).
"""
import sys
from pathlib import Path

BYTES_PER_LINE = 16  # number of bytes per line in the output

if len(sys.argv) != 5:
    print("Usage: cmake_shader_gen.py <input_shader> <shader_name> <template> <output>", file=sys.stderr)
    sys.exit(1)

input_file = Path(sys.argv[1])
shader_name = sys.argv[2]
template_file = Path(sys.argv[3])
output_file = Path(sys.argv[4])

if not input_file.is_file():
    print(f"Error: file {input_file} does not exist", file=sys.stderr)
    sys.exit(1)

if not template_file.is_file():
    print(f"Error: template {template_file} does not exist", file=sys.stderr)
    sys.exit(1)

# Read shader content
content = input_file.read_bytes()

# Convert each byte to 0xHH format for array initializer
hex_bytes = [f'0x{b:02x}' for b in content]

# Group into lines of BYTES_PER_LINE bytes
lines = []
for i in range(0, len(hex_bytes), BYTES_PER_LINE):
    chunk = hex_bytes[i:i + BYTES_PER_LINE]
    lines.append('    ' + ', '.join(chunk) + ',')

# Join lines with newlines
byte_string = '\n'.join(lines)

# Read template
template = template_file.read_text()

# Substitute variables
output_content = template.replace('@SHADER_NAME@', shader_name)
output_content = output_content.replace('@SHADER_SRC@', byte_string)

# Write output
output_file.write_text(output_content)

print(f"Generated {output_file} from {input_file}")
