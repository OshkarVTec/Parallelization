#!/bin/bash
# Usage: ./filter_machinefile.sh machinefile > machinefile_ok

inputfile="$1"
if [ -z "$inputfile" ]; then
    echo "Usage: $0 machinefile" >&2
    exit 1
fi

first=1
while IFS= read -r host || [ -n "$host" ]; do
    # Ignore blank lines and comments
    [[ -z "$host" ]] && continue
    [[ "$host" =~ ^# ]] && continue

    if [ $first -eq 1 ]; then
        echo "$host"
        first=0
    else
        # Extract hostname
        host_clean=$(echo "$host" | awk '{print $1}')
        echo "Checking $host_clean..." >&2

        if ssh -o BatchMode=yes -o ConnectTimeout=5 "$host_clean" "echo OK" >/dev/null 2>&1; then
            echo "$host"
        else
            echo "Skipping $host_clean (unreachable)" >&2
        fi
    fi
done < "$inputfile"