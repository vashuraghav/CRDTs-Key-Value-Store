#!/bin/bash

# Check if the correct number of arguments is provided
if [ "$#" != 1 ] && [ "$#" != 2 ]; then
    echo "Usage: $0 <server_or_client> <node_id_if_client>"
    exit 1
fi

# Check if the first argument is either 0 (client) or 1 (server)
if [ "$1" != "0" ] && [ "$1" != "1" ]; then
    echo "Invalid value for <server_or_client>. Use 0 for client or 1 for server."
    exit 1
fi

# Change directory to your project root (adjust this if needed)
make clean

# Run CMake
cmake .

# Build the project
make

PROJECT_ROOT=$(pwd)

# Number of servers to spin up if $1 is 1
if [ "$1" -eq 1 ]; then
    NUM_SERVERS=5
    INTERVAL=15  # Interval in seconds to stop and restart servers
    SERVER_PIDS=()  # Array to store process IDs of running servers

    # Run the servers with a wait between each
    for ((i = 0; i < NUM_SERVERS; i++)); do
        # Run the server executable with the current node ID in a new terminal
        osascript -e 'tell application "Terminal" to do script "cd '"${PROJECT_ROOT}"' && ./Build/bin/restserver 1 '$i'"' &

        sleep 1
        
    done

    sleep 3

    SERVER_PIDS=($(pgrep -f "restserver"))


    echo "Server PIDs: ${SERVER_PIDS[@]}"

    # Continuous restarting loop
    while true; do
        # Wait for the specified interval
        sleep $INTERVAL

        # Choose two random server node IDs to restart
        NUM_SERVERS_TO_RESTART=2
        SERVERS_TO_RESTART=()
        for ((i = 0; i < $NUM_SERVERS_TO_RESTART; i++)); do
            SERVERS_TO_RESTART+=($((RANDOM % NUM_SERVERS)))
        done

        echo "Servers to restart: ${SERVERS_TO_RESTART[@]}"

        # Restart the selected servers
        for node_id in "${SERVERS_TO_RESTART[@]}"; do
            # Get the corresponding process ID from the mapping
            pid=${SERVER_PIDS[$node_id]}
            echo "Killing process with PID: $pid"
            # Check if the process ID exists
            if [ -n "$pid" ]; then
                # Kill the server process with the obtained PID
                kill $pid
                # Run the server executable with the current node ID in a new terminal
                osascript -e 'tell application "Terminal" to do script "cd '"${PROJECT_ROOT}"' && ./Build/bin/restserver 1 '$node_id'"' &
                # Update the process ID in the mapping
                sleep 1
                SERVER_PIDS[$node_id]=$(pgrep -d ' ' -f "restserver" | sort -n | tail -n 1 | awk '{print $NF}')
                echo "Server up now ${SERVER_PIDS[$node_id]}"
            fi
        done
    done
else
    # Run the executable
    ./Build/bin/restserver $1 $2
fi