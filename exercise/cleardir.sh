#!/bin/bash

# Function to create the 'data' directory if it doesn't exist
create_data_directory() {
    local dir="$1/data"
    if [[ ! -d "$dir" ]]; then
        mkdir -p "$dir"
    fi
}

# Function to move specified file types to 'data' directory
move_files() {
    local dir="$1"
    local data_dir="$dir/data"
    
    create_data_directory "$dir"
    
    # Moving specified file types to 'data' directory
    mv "$dir"/*.pam "$dir"/*.txt "$dir"/*.ppm "$dir"/*.hdr "$dir"/*.pmg "$dir"/*.huffdiff "$dir"/*.lz4 "$dir"/*.torrent "$data_dir" 2>/dev/null
}

# Main function
main() {
    local current_dir
    
    find . -type d | while read -r current_dir; do
        move_files "$current_dir"
    done
}

main "$@"