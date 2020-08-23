#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <queue>

#include <grpc/grpc.h>
#include <grpcpp/server.h>
#include <grpcpp/server_builder.h>
#include <grpcpp/server_context.h>
#include <grpcpp/security/server_credentials.h>
#include "graph_service.grpc.pb.h"

using namespace std;
using namespace grpc;

// data structure to store graph edges
struct GraphEdge {
	int src, dest, dist;
};

typedef pair<int, int> Pair;
// class to represent a graph object
class Graph
{
public:
	// construct a vector of vectors to represent an adjacency list
	vector<vector<Pair>> adjList;
    int graph_sz;

	// Graph Constructor
	Graph(vector<Edge> const &edges, int N):graph_sz(N)
	{
		// resize the vector to N elements of type vector<int>
		adjList.resize(N);

		// add edges to the directed graph
		for (auto &edge: edges)
		{
			// insert at the end
			adjList[edge.start()].push_back(make_pair(edge.end(),edge.dist()));

			// Uncomment below line for undirected graph
			adjList[edge.end()].push_back(make_pair(edge.start(),edge.dist()));
		}

	}

    int shortestPath(int start, int end) {
        if (start>= graph_sz || end>= graph_sz) {
            return -1;
        }

        vector<int> dist(graph_sz, INT_MAX);

        // Create a PQ.
        priority_queue<pair<int, int>, vector< pair<int, int> >, greater<pair<int, int> > > pq;

        // Add source to pq, where distance is 0.
        pq.push(make_pair(start, 0));
        dist[start] = 0;
        
        // While pq isn't empty...
        while(!pq.empty()) {
            // Get min distance vertex from pq. (Call it u.)
            int u = pq.top().first;
            pq.pop();
            
            // Visit all of u's friends. For each one (called v)....
            for(int i = 0; i < adjList[u].size(); i++)
            {
                int v = adjList[u][i].first;
                int weight = adjList[u][i].second;
                
                // If the distance to v is shorter by going through u...
                if(dist[v] > dist[u] + weight)
                {
                    // Update the distance of v.
                    dist[v] = dist[u] + weight;
                    // Insert v into the pq. 
                    pq.push(make_pair(v, dist[v]));
                }

                // if destination is already reached.
                if (v==end) {
                    break;
                }
            }
        }

        return dist[end];
    }
};

class GraphServiceImpl final : public GraphService::Service
{
public:
	Status postGraph(ServerContext* context, const PostInput* in, PostOutput* out) override
	{
        int num_edges = in->edge_size();
        vector<Edge> vecEdges;

        for (auto i=0;i<num_edges; i++) {
            vecEdges.push_back(in->edge(i));
        }

        Graph *graph = new Graph(vecEdges, num_edges);

        /* Acquire lock for critical section
           Ideally would only need a hash bucket lock if implementing own hashtable rather than using STL map
        */
        std::unique_lock<std::mutex> lock(mu);
        // increment the graphId if it is already taken
        while(hashMap[graphId] != nullptr) {
            graphId++;
        }

        hashMap[graphId] = graph;
        out->set_id(graphId);
        graphId++;

		return Status::OK;
	}

    Status shortestPath(ServerContext* context, const ShortestPathInput* in, ShortestPathOutput* out) override 
    {
        int dist;
        int64_t id;
        uint32_t start, end;
        Graph *graph;
    
        id = in->id();
        start = in->start();
        end = in->end();

        graph = hashMap[id];
        if (!graph) {
            cout<<"shortestPath: invalid id provided"<<endl;
            return Status::OK;
        }
        dist = graph->shortestPath(start,end);

        out->set_distance(dist);

        return Status::OK;
    }

    Status deleteGraph(ServerContext* context, const DeleteInput* in, DeleteOutput* out) override 
    {
        int64_t id = in->id();
        
        if (hashMap[id] == nullptr) {
            cout<<"deleteGraph: invalid id provided"<<endl;
            out->set_result(false);
            return Status::OK;
        }

        delete(hashMap[id]);
        hashMap[id] = nullptr;

        out->set_result(true);

        return Status::OK;
    }

private:
    static int64_t graphId;  //rotating ID that will be generated
    std::mutex mu;  // to prevent critical section errors
    unordered_map<int, Graph*> hashMap;
};

int64_t GraphServiceImpl::graphId=1;

void RunServer() {
  string server_address("0.0.0.0:50051");
  GraphServiceImpl service;

  ServerBuilder builder;
  builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
  builder.RegisterService(&service);
  std::unique_ptr<Server> server(builder.BuildAndStart());
  std::cout << "Server listening on " << server_address << std::endl;
  server->Wait();

}

int main(int argc, char *argv[])
{
    RunServer();
    return 1;
}
