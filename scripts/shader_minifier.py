# Copyright (c) 2024 Le Juez Victor
# 
# This software is provided "as-is", without any express or implied warranty. In no event 
# will the authors be held liable for any damages arising from the use of this software.
# 
# Permission is granted to anyone to use this software for any purpose, including commercial 
# applications, and to alter it and redistribute it freely, subject to the following restrictions:
# 
#   1. The origin of this software must not be misrepresented; you must not claim that you 
#   wrote the original software. If you use this software in a product, an acknowledgment 
#   in the product documentation would be appreciated but is not required.
# 
#   2. Altered source versions must be plainly marked as such, and must not be misrepresented
#   as being the original software.
# 
#   3. This notice may not be removed or altered from any source distribution.

import sys

def format_shader(input_string):
    """
    Minifies GLSL shader code by removing comments, extra whitespace, and unnecessary line breaks. 
    Preserves preprocessor directives (#define, #version, etc.) with proper formatting.

    Args:
        input_string (str): The GLSL shader source code as a single string.

    Returns:
        str: Minified shader code where comments are removed, and code lines are compacted.
    """
    output_lines = []
    buffer_line = ""  # Accumulates contiguous lines of GLSL code
    in_multiline_comment = False  # Tracks whether the parser is inside a multiline comment

    for line in input_string.splitlines():
        # Handle multiline comments
        if in_multiline_comment:
            end_comment_index = line.find("*/")
            if end_comment_index != -1:
                # End of multiline comment found, resume processing the remainder of the line
                line = line[end_comment_index + 2:]
                in_multiline_comment = False
            else:
                # Entire line is within a multiline comment, skip it
                continue

        # Detect the start of a multiline comment
        start_comment_index = line.find("/*")
        if start_comment_index != -1:
            end_comment_index = line.find("*/", start_comment_index + 2)
            if end_comment_index != -1:
                # Multiline comment starts and ends on the same line, strip it out
                line = line[:start_comment_index] + line[end_comment_index + 2:]
            else:
                # Multiline comment starts but doesn't end, strip everything after its start
                line = line[:start_comment_index]
                in_multiline_comment = True

        if line.strip():  # Ignore empty lines
            formatted_line = line.lstrip()  # Remove leading whitespace

            if not formatted_line.startswith("//"):  # Skip single-line comments
                code_line = formatted_line.split("//")[0].rstrip()  # Remove trailing inline comments

                if code_line.startswith("#"):
                    # Preprocessor directive, flush accumulated buffer and add directive
                    if buffer_line:
                        output_lines.append(buffer_line)
                        buffer_line = ""
                    output_lines.append(code_line)
                else:
                    # Accumulate GLSL code in a single buffer line
                    buffer_line += code_line

    # Append the remaining accumulated code line if any
    if buffer_line:
        output_lines.append(buffer_line)

    return "\n".join(output_lines)


def main():
    """
    Main entry point for the script. Reads a GLSL shader file, processes it using format_shader,
    and outputs the minified shader code to the standard output.
    """
    if len(sys.argv) < 2:
        print("Usage: python glsl_minifier.py <path_to_shader>")
        return

    filepath = sys.argv[1]
    try:
        with open(filepath, "r") as file:
            input_shader = file.read()

        formatted_shader = format_shader(input_shader)
        print(formatted_shader, end="")  # Avoids trailing newlines
    except FileNotFoundError:
        print(f"Error: File not found [{filepath}]")
    except Exception as e:
        print(f"An error occurred: {e}")


if __name__ == "__main__":
    main()
