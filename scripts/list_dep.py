#!/usr/bin/python3
# run from Radium-Engine/external directory
# This files generates information about Radium externals, and is used to generate the Documentation,

from pathlib import Path
import re

p = Path('.')

filenames=list(p.glob('**/CMakeLists.txt'))
full = re.compile(r'(Add\([^\)]*\))')
name = re.compile(r'Add\([\s]*(\S*)')
git = re.compile(r'[^\)]*GIT_REPOSITORY (.*)')
tag = re.compile(r'[^\)]*GIT_TAG (.*)')
option = re.compile(r'[^\)](-D\S*)')
optionfilter = re.compile(r'indent')

modules={}
dep = {}
debug = False

for filename in filenames:
    with open(filename, "r") as f:
        currentname="dummy"
        filetext = f.read()
        start=0
        match0 = full.search(filetext)
        while(match0):
            cmake_add_text = match0.group(1)
            match1 = name.search(cmake_add_text)

            currentname = match1.group(1) if match1 else 'dummy'

            if debug : print( " --------- ", currentname)
            if debug : print(cmake_add_text)
            if debug : print( " --------- " )

            dep[currentname]={}

            for key,regex  in zip(['git', 'tag'], [git, tag]):
                match2 = regex.search(cmake_add_text)
                if match2 and currentname != 'dummy':
                    option_text = match2.group(1)
                    if key in dep[currentname]:
                        dep[currentname][key] += " " + option_text
                    else:
                        dep[currentname][key] = option_text
                    if debug : print(key, dep[currentname][key])

            key,regex = 'option', option
            match3 = regex.search(cmake_add_text)
            while match3:
                option_text = match3.group(1)
                if not optionfilter.search(option_text) and currentname != 'dummy':
                    if debug : print("[",option_text,"]")
                    if key in dep[currentname]:
                        dep[currentname][key] += " " + option_text
                    else:
                        dep[currentname][key] = option_text
                match3 = regex.search(cmake_add_text, match3.end(1))
            start = match0.end(1)
            match0 = full.search(filetext, start)
    if len(dep) >0 : modules[filename.parts[0]] = dep
    dep={}

for module in {'Core', 'Engine', 'IO', 'Gui', 'Headless'}:
    if module in modules:
        dep = modules[module]
        for key in dep:
            print(f"* `{key}_DIR`")

print("\n\nRadium is compiled and tested with specific version of dependencies, as given in the external's folder CMakeLists.txt and state here for the record\n\n")

for module in {'Core', 'Engine', 'IO', 'Gui', 'Headless'}:
    if module in modules:
        dep = modules[module]
        print(f"* [{module}]")
        for key in dep:
            print(f" * {key}: {dep[key]['git']}, [{dep[key]['tag']}],\n    * with options `{dep[key]['option'] if 'option' in dep[key] else None }`")
