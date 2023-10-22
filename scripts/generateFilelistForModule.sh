#!/bin/bash

if [ "$#" -ne 1 ]; then
    echo "Please provide one and only one module name (e.g. Core) that corresponds to the directory name under src/ (e.g. src/Core)."
    exit 2
fi

BASE=$1
LOWBASE=$(echo "$BASE" | tr '[:upper:]' '[:lower:]')
LOWBASE=$(echo "$LOWBASE" | tr '/' '_')
OUTPUT="../src/${BASE}/filelist.cmake"

echo "generate [${BASE}] filelist"
echo "-- input files from         [../src/${BASE}]"
echo "-- output vars prefix       [${LOWBASE}]"
echo "-- output file is           [${OUTPUT}]"
DELIM='/'
SLASHES=$( awk -F"$DELIM" '{print NF-1}' <<<"${BASE}" )
(( SLASHES = 4 + SLASHES ))

rm -f "${OUTPUT}"

echo  "# ----------------------------------------------------" >> "${OUTPUT}"
echo  "# This file can be generated from a script:" >> "${OUTPUT}"
echo  "# To do so, run \"$0 ${BASE}\" " >> "${OUTPUT}"
echo  "# from ./scripts directory" >> "${OUTPUT}"
echo  "# ----------------------------------------------------" >> "${OUTPUT}"
echo ""  >> "${OUTPUT}"

export LC_ALL=C

function genList(){
    ext=$1
    suffix=$2
    L=$(find  "../src/${BASE}" -name "*.${ext}")
    if [ ! -z "$L" ]
    then
        echo  "set(${LOWBASE}_${suffix}" >> "${OUTPUT}"
        echo "${L}" | cut -f $SLASHES- -d/ | grep -v pch.hpp | sort | xargs -n1 echo "   "  >> "${OUTPUT}"
        echo ")" >> "${OUTPUT}"
        echo ""  >> "${OUTPUT}"
    fi
}

function genListIo(){
    ext=$1
    suffix=$2
    L=$(find  "../src/${BASE}" -name "*.${ext}")
    if [ ! -z "$L" ]
    then
        echo  "set(${LOWBASE}_${suffix}" >> "${OUTPUT}"
        echo "${L}" | grep -v pch.hpp | grep -v deprecated | grep -v AssimpLoader | grep -v TinyPlyLoader | grep -v VolumesLoader | cut -f 4- -d/ | sort | xargs -n1 echo "   "  >> "${OUTPUT}"
        echo ")" >> "${OUTPUT}"
        echo ""  >> "${OUTPUT}"
    fi
}

function genListIoAppend(){
    ext=$1
    suffix=$2
    L=$(find  "../src/${BASE}/deprecated" -name "*.${ext}")
    if [ ! -z "$L" ]
    then
        echo "list(APPEND ${LOWBASE}_${suffix}"  >> "${OUTPUT}"
        echo "${L}" | cut -f 4- -d/ | sort | xargs -n1 echo "   "  >> "${OUTPUT}"
        echo ")" >> "${OUTPUT}"
        echo ""  >> "${OUTPUT}"
    fi
}


function genListIoAppendSubdir(){
    ext=$1
    suffix=$2
    subdir=$3
    L=$(find  "../src/${BASE}/${subdir}" -name "*.${ext}")
    if [ ! -z "$L" ]
    then
        echo "list(APPEND ${LOWBASE}_${suffix}"  >> "${OUTPUT}"
        echo "${L}" | cut -f 4- -d/ | sort | xargs -n1 echo "   "  >> "${OUTPUT}"
        echo ")" >> "${OUTPUT}"
        echo ""  >> "${OUTPUT}"
    fi
}

if [ "$BASE" != "IO" ]; then
    genList "cpp" "sources"
    genList "hpp" "headers"
    genList "ui"   "uis"
    genList "qrc"  "resources"
fi

if [ "$BASE" = "PluginBase" ]; then
    genList "json" "json"
fi

if [ "$BASE" = "Engine" ]; then
    echo ""  >> "${OUTPUT}"
    echo  "set(${LOWBASE}_shaders"  >> "${OUTPUT}"
    find  "../Shaders" -name "*.glsl" | grep -v Deprecated | cut -f 3- -d/ | sort | xargs -n1 echo "   "  >> "${OUTPUT}"
    echo ")"  >> "${OUTPUT}"
fi

if [ "$BASE" = "IO" ]; then
    genListIo "cpp" "sources"
    genListIo "hpp" "headers"
    genListIo "ui"   "uis"
    genListIo "qrc"  "resources"
    echo "if(RADIUM_IO_DEPRECATED)"  >> "${OUTPUT}"
    genListIoAppend "cpp" "sources"
    genListIoAppend "hpp" "headers"
    echo "endif(RADIUM_IO_DEPRECATED)"  >> "${OUTPUT}"
    echo ""  >> "${OUTPUT}"
    echo "if( RADIUM_IO_ASSIMP )"  >> "${OUTPUT}"
    genListIoAppendSubdir "cpp" "sources" "AssimpLoader"
    genListIoAppendSubdir "hpp" "headers" "AssimpLoader"
    echo "endif( RADIUM_IO_ASSIMP )"  >> "${OUTPUT}"
    echo ""  >> "${OUTPUT}"
    echo "if( RADIUM_IO_TINYPLY )"  >> "${OUTPUT}"
    genListIoAppendSubdir "cpp" "sources" "TinyPlyLoader"
    genListIoAppendSubdir "hpp" "headers" "TinyPlyLoader"
    echo "endif( RADIUM_IO_TINYPLY )"  >> "${OUTPUT}"
    echo ""  >> "${OUTPUT}"
    echo "if( RADIUM_IO_VOLUMES )"  >> "${OUTPUT}"
    genListIoAppendSubdir "cpp" "sources" "VolumesLoader"
    genListIoAppendSubdir "hpp" "headers" "VolumesLoader"
    echo "endif( RADIUM_IO_VOLUMES )"  >> "${OUTPUT}"
fi
cmake-format -i "${OUTPUT}"
