#!/bin/bash

if [ "$#" -ne 2 ]; then
    echo "Usage: $0 <client_id> <number_of_commands>"
    exit 1
fi

# Function to generate a random alphanumeric string of length n
generate_random_string() {
    local length=$1
    local result=""
    local characters="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789"
    for ((i=0; i<length; i++)); do
        random_index=$((RANDOM % ${#characters}))
        result+=${characters:$random_index:1}
    done
    echo "$result"
}

# Number of random inputs to generate
n=$2

# Define input commands

for ((i=0; i<n; i++)); do
    choice=$((RANDOM % 7 + 1)) # Generate a random number between 1 and 7 (inclusive)
    case $choice in
        1)
            key=$(generate_random_string 8)
            value=$(generate_random_string 8)
            echo "1" >> "client$1.txt"
            echo "$key" >> "client$1.txt"
            echo "$value" >> "client$1.txt"
            ;;
        2|3|4)
            key=$(generate_random_string 8)
            echo "$choice" >> "client$1.txt"
            echo "$key" >> "client$1.txt"
            ;;
        5|6)
            echo "$choice" >> "client$1.txt"
            ;;
    esac
done

echo "7" >> "client$1.txt"

# Run the program with input redirection
./run.sh 0 $1 < "client$1.txt"
