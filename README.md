# gRPCGraph

 - To run the sample files one needs to install gRPC, cmake and protobufs. 
 - Link for the installation can be seen here https://grpc.io/docs/languages/cpp/quickstart/
 - Follow the below steps for running the files
   - export MY_INSTALL_DIR=$HOME/.local
   - Change to the directory having code 
    > cd cpp/graph_service
   -  Using cmake build the binary 
    > mkdir -p cmake/build  
    > pushd cmake/build 
    > cmake -DCMAKE_PREFIX_PATH=$MY_INSTALL_DIR ../..  
    > make -j    
    > Files are generated in the cmake/build folder
 - Run server in terminal 
  > ./graph_server 
  > Server listening on 0.0.0.0:50051
  
 - Run client in different terminals each client also reads the graph input from the inputfiles.
   > ./graph_client ../../../../inputFiles/file1.txt  
6 
0 1  2 
1 2  4 
1 3  4 
2 0  4 
3 2  2 
3 4  3 
2 5  3 
0 5  3 
Client: post_graph Success id: 1 
5 
0 2  4 
0 1  2 
1 4  3 
2 3  2 
3 1  4 
4 3  2 
Client: post_graph Success id: 2 
Client: shortest_path from 0 to 3 is: 6 
Client: delete_graph for id: 1 Success 
Client: shortest_path from 0 to 3 is: 6 
Client: delete_graph for id: 2 Success 

   > ./graph_client ../../../../inputFiles/file2.txt  
6 
0 1  2 
1 2  4 
1 3  4 
2 0  4 
3 2  2 
3 4  3 
2 5  3 
0 5  3 
Client: post_graph Success id: 3 
5 
0 2  4 
0 1  2 
1 4  3 
2 3  2 
3 1  4 
4 3  2 
Client: post_graph Success id: 4 
Client: shortest_path from 0 to 4 is: 9 
Client: delete_graph for id: 3 Success 
Client: shortest_path from 0 to 4 is: 5 
Client: delete_graph for id: 4 Success 
  
- Inputfile  
> It contains the graph as well as the number of edges in the graph. Also it contains the source and destination for which the shortest path needs to be calculated.
 
## Design 
 - The post operation is done by reading from the file having the graph input. The gRPC client then calls the server API post_graph. The Server then creats the graph and a unique ID is generated. The unique ID generated is a round robin one and it is given back to the client. Each client keeps note of its id in a queue.
The server keeps a hashMap to save the id versus graph mapping. 
 - The shortest path calculation from source to destination is done using Djikstra algorithm. It returns the shortest path given the ID, src and destination. 
 - Delete graph is done on the ID. The client sends the delete to the server which then releases the memory for the graph given ID. 
 - A lock is added to protect from the critical section when the ID is generated and many POST graph can happen from multiple clients. This can be enhanced further by allocating each client a fixed chunk of indexes and then locking wont be necessary.
 - Assumption is that each client will operate only on its own ID's. 
