#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <fstream>
#include <queue>

#include <grpc/grpc.h>
#include <grpcpp/channel.h>
#include <grpcpp/client_context.h>
#include <grpcpp/create_channel.h>
#include <grpcpp/security/credentials.h>
#include "graph_service.grpc.pb.h"

using namespace std;
using namespace grpc;

/* Function to create edges from the client input */
void createEdges( const vector<vector<int>> &edgeInput, vector<Edge> &edges)
{
    Edge e;

    edges.clear();
    for (auto edge: edgeInput) {
        e.set_start(edge[0]);
        e.set_end(edge[1]);
        e.set_dist(edge[2]);
        edges.push_back(e);
    }
}

class GraphClient {
public:
    GraphClient(shared_ptr<Channel> channel):
        stub(GraphService::NewStub(channel)) {}

    bool post_graph(const vector<Edge> &edges, int N, int64_t & id){
        PostInput input;
        PostOutput output;
        ClientContext context;
        Edge * e;

        input.set_num_edges(N);
        /* Add each of the post input edge */
        for (auto edge: edges) {
            e = input.add_edge();
            e->set_start(edge.start());
            e->set_end(edge.end());
            e->set_dist(edge.dist());
        }

        Status status = stub->postGraph(&context, input, &output);
        if (!status.ok()) {
            std::cout << "post_graph rpc failed." << std::endl;
            return false;
        }

        // Save the returned id
        id = output.id();

        return true;
    }

    int shortest_path(int start, int end, int64_t id){
        int dist;
        ShortestPathInput input;
        ShortestPathOutput output;
        ClientContext context;

        input.set_id(id);
        input.set_start(start);
        input.set_end(end);

        Status status = stub->shortestPath(&context, input, &output);
        
        if (!status.ok()) {
            std::cout << "shortest_path rpc failed." << std::endl;
            return -1;
        }

        if (output.distance() == -1 ) {
            std::cout << "shortest_path points out of range" << std::endl;
            return -1;
        }
        /* if there is not path between the points */
        if (output.distance() == INT_MAX) {
            std::cout << "shortest_path not present" << std::endl;
            return -1;
        }

        return output.distance();
    }

    bool delete_graph(int64_t id){
        DeleteInput input;
        DeleteOutput output;
        ClientContext context;

        input.set_id(id);

        Status status = stub->deleteGraph(&context, input, &output);
        if (!status.ok()) {
            std::cout << "delete_graph rpc failed." << std::endl;
            return false;
        }

        if (!output.result()) {
            std::cout << "delete_graph invalid ID" << std::endl;
            return false;
        }

        return true;
    }
private:
	unique_ptr<GraphService::Stub> stub;
};

bool readFile(ifstream &infile, GraphClient &client, queue<int64_t> &q) {
    string line;
    int N, start, end, dist,i, count;
    vector<vector<int>> edgeInput;
    vector<Edge> edges;   
    int64_t id;
    bool res;

    while (infile >> N) {
        cout << N <<endl;
        edgeInput.clear();

        infile >> count;
        for (i=0;i<count;i++) {
            infile >> start >> end >> dist;
            cout<<start<<" "<< end <<"  " << dist <<endl; 
            edgeInput.push_back({start,end, dist});
        }
        createEdges(edgeInput, edges);

        res = client.post_graph(edges, N, id);
        if (!res) {
            return false;
        }
        cout<<"Client: post_graph Success id- "<<id<<endl;
        q.push(id);
    }

    return true;
}

int main(int argc, char *argv[])
{
    GraphClient client(grpc::CreateChannel("localhost:50051", grpc::InsecureChannelCredentials()));
    bool res;
    int shortedPath, src, dest;
    queue<int64_t> q;
    int64_t id;

    ifstream infile(argv[1]);

    infile >> src >> dest;

    // Read the input graph and post to server 
    if (!readFile(infile,client, q)) {
        cout<<"Client: post_graph failure"<<endl;
        return -1;
    }

    // Deque from q and calculate shortest path and delete the graph
    while(!q.empty()) {
        id = q.front();
        shortedPath = client.shortest_path(src, dest, id);
        if (shortedPath ==-1) {
            return -1;
        }
        cout<<"Client: shortest_path from"<< src <<"to"<<dest<<"is:"<<shortedPath<<endl;

        res = client.delete_graph(id);
        if (!res) {
            return -1;
        }
        cout<<"Client: delete_graph for id- "<<id<<"Success"<<endl;
        q.pop();
    }

    return 1;
}
