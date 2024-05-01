NUM_CLIENTS=2

PROJECT_ROOT=$(pwd)
# Run the servers with a wait between each
for ((i = 5; i < 5+NUM_CLIENTS; i++)); do
    # Run the server executable with the current node ID in a new terminal
    osascript -e 'tell application "Terminal" to do script "cd '"${PROJECT_ROOT}"' && sleep $((2 * '$i')) && ./generateclientinput.sh '$i' 10"'
done