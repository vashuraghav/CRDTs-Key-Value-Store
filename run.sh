# Check if the correct number of arguments is provided
if [ "$#" -ne 2 ]; then
    echo "Usage: $0 <server_or_client> <node_id>"
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

# Run the executable
./Build/bin/restserver $1 $2