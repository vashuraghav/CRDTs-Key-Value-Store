# Distributed-Key-Value-Store using CRDTs.

- The project is to design and implement a distributed key-value store utilizing Conflict-free Replicated Data Types (CRDTs). We aim to provide a robust, scalable, and fault-tolerant solution for storing and accessing key-value data in a distributed environment. By leveraging CRDTs, the goal is to achieve high availability and eventual consistency while allowing concurrent operations across distributed nodes.

## Build Project
- Run `./make.sh` in the project root directory.

## Setup Cluster Nodes
- To run live nodes,  `./run.sh 1` in the project root directory. (5 Nodes will be up)
- To run fail-recover nodes,  `./run-stop.sh 1` in the project root directory. (5 Nodes will be up, up to 2 will be fail and recover at any instant)
- Run `./run.sh 2` in the project root directory. (Load Balancer will be up)

  ### Note:
  - The first step will work on MacOS with the native terminal. For other operating systems, you need to either manually setup each server with their corresponding id starting from 0. or you can create another script similar to run.sh compatible with the operating system.
  - To run a single node, `./run.sh 1 <node_id>`  (node_id is a number ranging from 0 to 4)
   

## Setup Clients for interactive mode
- Run `./run.sh 0 <client_id>` in the project root directory. (client_id is a number ranging from 5 to 8).

## Run simulation for multiple clients 
- Run `./consistencytest.sh <num_clients> <operation_count>` in the project root directory.

  
