#!/bin/bash
### Usage: mpi_time OUTPUT_FILE command [args...]
### Appends MPI rank to file name and ensure format is consistent with GNU time 1.7
### Also setup variables for profiling with LSB and move files to the correct folder
set -euo pipefail
if [ $# -lt 2 ]; then
  echo 1>&2 "Usage: $0 OUTPUT_FILE command [args...]"
  exit 2
fi
OUTPUT_FILE="$1$OMPI_COMM_WORLD_RANK"
OUTPUT_FOLDER=$(sed 's/\/[^\/]*$//' <<< "$1")
LSB_OUTFILE=$(sed 's/.*\///' <<< "$1")
LSB_OUTPUT_FORMAT=efficient
LSB_TRUE_FILE="lsb.$LSB_OUTFILE.r$OMPI_COMM_WORLD_RANK"
export LSB_OUTFILE
export LSB_OUTPUT_FORMAT
FORMAT="%Uuser %Ssystem %Eelapsed %PCPU (%Xavgtext+%Davgdata %Mmaxresident)k\n%Iinputs+%Ooutputs (%Fmajor+%Rminor)pagefaults %Wswaps\n"
shift
command time -f "$FORMAT" -o "$OUTPUT_FILE" "$@"
[[ -d "$OUTPUT_FOLDER" && -f "$LSB_TRUE_FILE" ]] && mv -t "$OUTPUT_FOLDER" "$LSB_TRUE_FILE"
