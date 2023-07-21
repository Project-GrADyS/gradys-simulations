import glob
import queue
import re
from typing import List, TypedDict

class Parameter(TypedDict):
    name: str
    type: str
    unit: str
    default: str
    description: str

def get_comments(end_line, lines):
    comment = ""
    if '//' in lines[end_line]:
        comment_line_index = end_line
        comment_lines = []
        while comment_line_index >= 0 and lines[comment_line_index].strip().startswith('//'):
            line: str = lines[comment_line_index]
            comment_lines = [line.strip().removeprefix('//')] + comment_lines
            comment_line_index -= 1

        
        comment = "\n".join(comment_lines)
    return comment

import os
import glob

# Removing old files
for f in glob.glob('./docs/Modules/*'):
    os.remove(f)

ned_filenames = [filename for filename in glob.iglob("../src/" + '**/*.ned', recursive=True)]
for filename in ned_filenames:
    with open(filename, "r") as file:
        module_name = filename.removesuffix('.ned')[filename.rfind('/') + 1:]
        lines = file.readlines()
        module_line = -1
        for index, line in enumerate(lines):
            if 'simple MAVLink' in line:
                module_line = index
                break
        
        if module_line == -1:
            continue
        module_extension = ""
        if 'extends' in lines[module_line]:
            module_extension = re.search(r"extends (\w+)", lines[module_line]).group(1)
            
            if any([module_extension in filename for filename in ned_filenames]):
                module_extension = f"[{module_extension}](/Modules/{module_extension}/)"
            module_extension = "Extends: " + module_extension

        module_description = get_comments(module_line - 1, lines)

        function_line = -1
        for index,line in enumerate(lines):
            if "parameters:" in line:
                function_line = index
                break
        
        parameters: List[Parameter] = []
        if function_line != -1:
            for index, line in enumerate(lines[function_line + 1:]):
                index += function_line + 1
                if 'submodules' in line or 'gates' in line:
                    break
                line = line.strip()
                if match := re.match(r'(volatile )?(\w+) (\w+)( @unit\((\w+)\))?( = (default\((.*)\))?)?;', line):
                    parameter = Parameter()
                    parameter['name'] = match.group(3)
                    if not parameter['name']:
                        continue
                    
                    parameter['type'] = match.group(2) or ""
                    parameter['unit'] = match.group(5) or ""
                    parameter['default'] = match.group(8) or ""
                    parameter['description'] = get_comments(index - 1, lines).replace('\n', '<br>')
                    parameters.append(parameter)

        documentation = f"# {module_name}\n" \
                        f"{module_extension}\n" \
                        f"## Description\n" \
                        f"{module_description}\n" \
                        f"## Parameters\n\n" \
                        f"| Name | Type | Unit | Default value | Description |\n" \
                        f"| ---- | ---- | ---- | ------------- | ----------- |\n"

        for parameter in parameters:
            documentation += f"| {parameter['name']} | {parameter['type']} | {parameter['unit']} | {parameter['default']} | {parameter['description']} |\n"

        with open(f"./docs/Modules/{module_name}.md", "w+") as doc_file:
            doc_file.write(documentation)


ned_filenames = [filename for filename in glob.iglob("../src/mavsimnet/utils/*.h", recursive=False)]
for filename in ned_filenames:
    with open(filename, "r") as file:
        module_name = filename.removesuffix('.h')[filename.rfind('/') + 1:]
        lines = file.readlines()
        module_line = -1
        for index, line in enumerate(lines):
            if f"namespace {module_name}" in line:
                module_line = index
                break
        
        if module_line == -1:
            continue

        module_description = get_comments(module_line - 1, lines)

        function_line = -1
        module_functions = ""
        function_pattern = re.compile(r"^[ \t]*.* (.*?)\(.*;$")
        while function_line < len(lines) - 1:
            function_line += 1

            match = function_pattern.match(lines[function_line])
            if match is None:
                continue
            function_name = match[1]
            
            function_description = get_comments(function_line - 1, lines)

            module_functions += f"### {function_name}\n\n" \
                                f"`{lines[function_line].strip()}`\n\n" \
                                f"{function_description}\n\n"

        documentation = f"# {module_name}\n" \
                        f"## Description\n" \
                        f"{module_description}\n\n" \
                        f"## Functions\n\n" \
                        f"{module_functions}\n\n"
        with open(f"./docs/Utils/{module_name}.md", "w+") as doc_file:
            doc_file.write(documentation)