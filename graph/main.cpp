/*
 *@BEGIN LICENSE
 *
 * PSI4: an ab initio quantum chemistry software package
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 *@END LICENSE
 */

#include <iostream>
#include "Graph.hpp"
#include "BFS.hpp"
#include "DFS.hpp"
#include "FindSubGraph.hpp"

using namespace pulsar::LibGraph;

///Class to test our graph search algorithm
class NodeClass {
   private:
      std::string Label_;
      size_t Index_;
   public:
      NodeClass(std::string Name="", size_t Index=0) :
            Label_(Name), Index_(Index) {
      }
      bool operator==(const NodeClass& other) const {
         return (this->Label_==other.Label_);
      }
      bool operator<(const NodeClass& Other) const {
         return this->Index_<Other.Index_;
      }
      std::ostream& operator<<(std::ostream& os) const {
         os<<Label_<<" ( actual index: "<<Index_<<")";
         return os;
      }
};

std::ostream& operator<<(std::ostream& os, const NodeClass& node) {
   return node<<os;
}

int main() {

   Graph<std::string> MyGraph;

   typedef Graph<std::string>::EdgeType Edge_t;

   std::array<std::string, 5> Nodes({"Node 1", "Node 2", "Node 3", "Node 4",
         "Node 5"});

   MyGraph.AddNode(Nodes.begin(), Nodes.end());
   Edge_t Edge(Nodes[0], Nodes[1]),Edge1(Nodes[1], Nodes[2]),Edge2(Nodes[2],
         Nodes[3]),Edge3(Nodes[0], Nodes[2]),Edge4(Nodes[3], Nodes[0]),Edge5(
         Nodes[4], Nodes[1]);
   MyGraph.AddEdge(Edge, Edge1, Edge2, Edge3, Edge4, Edge5);

   std::cout<<MyGraph<<std::endl;

   std::cout<<Nodes[0]<<" and "<<Nodes[1]
         <<(MyGraph.AreConn(Nodes[0], Nodes[1]) ? " are " : " aren't ")
         <<"connected"<<std::endl<<std::endl;

   //Test BFS
   BFS<Graph<std::string> > bfs(MyGraph);
   bfs.Run(Nodes[0]);
   std::cout<<bfs<<std::endl;

   //Test DFS
   DFS<Graph<std::string> > dfs(MyGraph);
   dfs.Run(Nodes[0]);
   //No output

   NodeClass N1("Node 1", 1),N2("Node 2", 2),N3("Node 3", 3),N4("Node 1", 4),N5(
         "Node 2", 5),N6("Node 3", 6),N7("Node 4", 7),N8("Node 1", 8),N9(
         "Node 2", 9),N10("Node 3", 10);
   Graph<NodeClass> LargeGraph(N1, N2, N3, N4, N5, N6, N7),SmallGraph(N8, N9,
         N10);
   Graph<NodeClass>::EdgeType E1(N1, N2),E2(N2, N3),E3(N3, N4),E4(N4, N5),E5(N5,
         N6),E6(N6, N7),E7(N8, N9),E8(N9, N10);
   LargeGraph.AddEdge(E1, E2, E3, E4, E5, E6);
   SmallGraph.AddEdge(E7, E8);

   std::cout<<LargeGraph<<std::endl;
   std::cout<<SmallGraph<<std::endl;

   //Test find sub graph
   FindSubGraph<Graph<NodeClass> > FSG(LargeGraph);
   FSG.Run(SmallGraph);
   FSG<<std::cout;

   return 0;
}
