#ifndef PULSAR_GUARD_GRAPH__BFS_HPP_
#define PULSAR_GUARD_GRAPH__BFS_HPP_

#include <boost/graph/breadth_first_search.hpp>

namespace pulsar{
namespace datastore {
namespace LibGraph{

template<typename Parent,typename Graph_t> class BFSBase;

/** \brief A class for running a breadth first search (BFS)
 *
 *  BFS starts with a node and then finds all nodes directly connected to
 *  it. Next it finds all nodes connected to those nodes, and continues
 *  until all edges are exhausted.  Consequentially, BFS is primarily used
 *  to compute the distances from a given node to all other reachable nodes.
 *
 *  You can use this class to run a BFS on your graph.  As
 *  the search progresses the various functions will be called along the
 *  way.  By default the only thing this class does is find the distance
 *  from your source node to each node connected to it.
 *
 *  This class can be derived from to put your own spin on the BFS
 *  algorithm.  To do this just modify
 *  the functions described in the next paragraph appropriately.  Note
 *  that BGL is mean and takes things by value, so if you want the object
 *  that goes into BFS to have the same state as the object you made, member
 *  data needs to be references/pointers.
 *
 *
 *  Initially BFS will call FoundNode()
 *  on your input node, give you that node, and add it to the queue.
 *  Now we start the algorithm proper.  On each iteration, a node will
 *  be removed from the queue, when this happens LookAtNode() will be
 *  called on that node.  Next, we will loop over the edges connected
 *  to the node we are looking at.  For each of the edges LookAtEdge()
 *  will be called on it.  At this point we determine what kind of edge
 *  it is.  If it is a tree edge (an edge that we need to follow because
 *  it connects us to a node we haven't seen) then TreeEdge() will
 *  be called on it.  Two other choices are possible, the edge connects
 *  us to a node that we know about, but haven't exhausted or the node
 *  is known, but exhausted.  In either case the edge may be a backward
 *  or cross edge (there are no forward edges in a BFS search tree).  For
 *  simplicity we lump them both into one call OtherEdge().  Now if the
 *  edge is a tree edge
 *  DiscoverNode() will be called on the new node and it will be added to
 *  the queue.  After this we continue on to the next edge, if there is
 *  no next edge NodeDone() is called on the node.  After we finish a node
 *  we proceed to the next node in the queue and the cycle repeats.
 *
 */
template<typename Graph_t>
class BFS: private BFSBase<BFS<Graph_t>,Graph_t>{
   protected:
      typedef BFSBase<BFS<Graph_t>,Graph_t> Base_t;
      typedef typename Base_t::Node_t Node_t;
      typedef typename Base_t::Edge_t Edge_t;
   public:
      ///No clean-up, but don't want no warning
      virtual ~BFS(){}
      ///Makes a BFS class that will work with Graph
      BFS(const Graph_t& Graph):BFSBase<BFS<Graph_t>,Graph_t>(Graph,*this){}

      ///Algorithm call back points
      ///@{
      ///Called the first time you see a node in the BFS algorithm
      virtual void FoundNode(const Node_t&){}
      ///Called when we start looping over a Node's edges
      virtual void LookAtNode(const Node_t&){}
      ///Called on each edge as we look at
      virtual void LookAtEdge(const Edge_t&){}
      ///Called when we see that our edge is a tree edge
      virtual void TreeEdge(const Edge_t&){}
      ///Called when we see our edge leads to a known node
      virtual void OtherEdge(const Edge_t&){}
      ///Called when we exhaust a node
      virtual void NodeDone(const Node_t& Node){}
      ///@}

      /** \brief The function you use to start this beast
       *
       *   The initial run of this class looks through all paths of a given
       *   node to all other nodes in the graph.  In general, there will
       *   be nodes that have not been accessed this way, either because
       *   they are upstream (there is no edge leading to them that our
       *   node can reach) or our graph is made up of more than one disjoint
       *   graph.  If you like you can continue looping over the remaining
       *   nodes without resetting the algorithm.  That is whether a node
       *   has been seen or not will be retained as will the distances. To
       *   do this pass in the next node and set Clean to false.  If you
       *   would like to reset either or both the distances and whether
       *   or not they were seen use the default.  Finally note that if
       *   our graph looks like:  X-->Y-->Z and we originally passed in
       *   Y, X will not have been seen.  If we then pass in X, and don't
       *   clean, aside from X now being marked as seen, nothing will
       *   happen because Y is already marked as finished.
       *
       */
      void Run(const Node_t& Node, bool Clean=true){RunImpl(Node,Clean);}

      /** Given a node, returns the distance from the input node to it.
       *  A value of 0 indicates either the node is not connected to the
       *  original node, or it is the original node.  We expect that
       *  the user knows what node they gave us and can tell the
       *  difference.
       */
      size_t Distance(const Node_t& Node)const{return Distance_.at(Node);}

      ///Returns true if the node was visited during the BFS
      bool WasSeen(const Node_t& Node)const{
         return Base_t::WasSeen(Node);
      }

      ///The function for printing this beast
      virtual std::ostream& operator<<(std::ostream& os)const{
         return Base_t::operator<<(os);
      }
};

///Allows a BFS to be passed to an ostream
template<typename T>
inline std::ostream& operator<<(std::ostream& os, const BFS<T>& bfs){
   return bfs<<os;
}

///This is an adaptor class, don't directly use it.  Use BFS class.
template<typename Parent,typename Graph_t>
class BFSBase:public boost::default_bfs_visitor{
   public:
      typedef typename Graph_t::Vertex_t Vertex_t;
      typedef typename Graph_t::Arc_t Arc_t;
      typedef typename Graph_t::NodeType Node_t;
      typedef typename Graph_t::EdgeType Edge_t;
      typedef std::map<Node_t,size_t> Map_t;
      typedef typename
            boost::vector_property_map<boost::default_color_type> Color_t;

      void discover_vertex(const Vertex_t& Node,const Graph_t&){
         Parent_.FoundNode(Graph_[Node]);
      }

      void examine_vertex(const Vertex_t& Node,const Graph_t&){
         Parent_.LookAtNode(Graph_[Node]);
      }

      void examine_edge(const Arc_t& Edge,const Graph_t&){
         Parent_.LookAtEdge(Graph_[Edge]);
      }

      void tree_edge(const Arc_t& Edge,const Graph_t&){
         (*Distance_)[Graph_[boost::target(Edge,Graph_)]]=
              (*Distance_)[Graph_[boost::source(Edge,Graph_)]]+1;
         Parent_.TreeEdge(Graph_[Edge]);
      }

      void non_tree_edge(const Arc_t& Edge,const Graph_t&){
         Parent_.OtherEdge(Graph_[Edge]);
      }

      /*//These calls seem superflous
      virtual void KnownEdge(Edge_t&)=0;
      void gray_target(const Arc_t& Edge,const Graph_t&){
          KnownEdge(Graph_[Edge]);
      }

      virtual void DeadEdge(Edge_t&)=0;
      void black_target(const Arc_t& Edge,const Graph_t&){
         DeadEdge(Graph_[Edge]);
      }
      */

      void finish_vertex(const Vertex_t& Node,const Graph_t&){
         Parent_.NodeDone(Graph_[Node]);
      }

      void RunImpl(const Node_t& Node,bool Clean){
         if(Clean)Reset();
         boost::breadth_first_visit(Graph_.Base_,Graph_.NodeLookUp_.at(Node),
               boost::visitor(*this).
               color_map(Colors_));
      }

      BFSBase(const Graph_t& Graph,Parent& P):
         Graph_(Graph),
         Parent_(P){Reset();}

      bool WasSeen(const Node_t& Node)const{
         return Colors_[Graph_.NodeLookUp_.at(Node)]==
               boost::color_traits<boost::default_color_type>::white();
      }

      std::ostream& operator<<(std::ostream& os)const{
         os<<"Node"<<'\t'<<"Distance to Source"<<std::endl;
         typename Map_t::const_iterator
         NI=Distance_->begin(),NEnd=Distance_->end();
         for(;NI!=NEnd;++NI)os<<NI->first<<'\t'<<NI->second<<std::endl;
         return os;
      }
   protected:
      void Reset(){
         Colors_=Color_t(Graph_.NNodes());
         Distance_=std::shared_ptr<Map_t>(new Map_t);
         typename Graph_t::NodeItr_t NI=Graph_.NodeBegin(),
                                     NEnd=Graph_.NodeEnd();
         for(;NI!=NEnd;++NI){
            (*Distance_)[*NI]=0;
            Colors_[Graph_.NodeLookUp_.at(*NI)]=
                  boost::color_traits<boost::default_color_type>::white();
         }
      }

      ///This is the graph for this search
      const Graph_t& Graph_;

      ///This is how BGL keeps track of the Nodes visited
      boost::vector_property_map<boost::default_color_type> Colors_;

      ///This is the distance from the start node to a given node
      std::shared_ptr<Map_t> Distance_;

      ///This is the derived class
      Parent& Parent_;
};

} // close namespace LibGraph
} // close namespace datastore
} // close namespace pulsar

#endif /* GRAPH_BFS_HPP_ */
