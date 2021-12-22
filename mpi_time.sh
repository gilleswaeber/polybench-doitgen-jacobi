#!/bin/sh
### Usage: mpi_time OUTPUT_FILE command [args...]
### Appends MPI rank to file name and ensure format is consistent with GNU time 1.7
OUTPUT_FILE="$1$OMPI_COMM_WORLD_RANK"
LSB_OUTFILE=$(sed 's/.*\///' <<< "$1")
export LSB_OUTFILE
FORMAT="%Uuser %Ssystem %Eelapsed %PCPU (%Xavgtext+%Davgdata %Mmaxresident)k\n%Iinputs+%Ooutputs (%Fmajor+%Rminor)pagefaults %Wswaps\n"
shift
command time -f "$FORMAT" -o "$OUTPUT_FILE" "$@"