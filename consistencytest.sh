NUM_CLIENTS=$1
NUM_INPUTS=$2
PROJECT_ROOT=$(pwd)

# Check if both inputs are provided
if [ -z "$NUM_CLIENTS" ] || [ -z "$NUM_INPUTS" ]; then
    echo "Usage: ./script.sh <num_clients> <num_inputs>"
    exit 1
fi

PROJECT_ROOT=$(pwd)
# Run the servers with a wait between each
for ((i = 5; i < 5+NUM_CLIENTS; i++)); do
    # Run the server executable with the current node ID in a new terminal
    osascript -e 'tell application "Terminal" to do script "cd '"${PROJECT_ROOT}"' && sleep $((2 * '$i')) && ./generateclientinput.sh '$i' '$2'"'
done