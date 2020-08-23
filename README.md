# gRPCGraph

 - To run the sample files one needs to install gRPC, cmake and protobufs. 
 - Link for the installation can be seen here https://grpc.io/docs/languages/cpp/quickstart/
 - Follow the below steps for running the files
   -  export MY_INSTALL_DIR=$HOME/.local
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
  
 - Run client in different terminals
