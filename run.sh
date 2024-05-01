# Check if the correct number of arguments is provided
if [ "$#" != 1 ] && [ "$#" != 2 ]; then
    echo "Usage: $0 <server_or_client_or_loadbalancer> <node_id_if_client>"
    exit 1
fi

# Check if the first argument is either 0 (client) or 1 (server)
if [ "$1" != "0" ] && [ "$1" != "1" ] && [ "$1" != "2" ]; then
    echo "Invalid value for <server_or_client_or_loadbalancer>. Use 0 for client or 1 for server or 2 for load balancer."
    exit 1
fi

PROJECT_ROOT=$(pwd)

# Number of servers to spin up if $1 is 1
if [ "$1" -eq 1 ]; then
    NUM_SERVERS=5

    # Run the servers with a wait between each
    for ((i = 0; i < NUM_SERVERS; i++)); do
        # Run the server executable with the current node ID in a new terminal
        osascript -e 'tell application "Terminal" to do script "cd '"${PROJECT_ROOT}"' && sleep $((2 * '$i')) && ./Build/bin/restserver 1 '$i'"'
    done
elif [ "$1" -eq 2 ]; then
    # Run the executable
    ./Build/bin/restlb $1 $2
else
    ./Build/bin/restclient $1 $2
fi