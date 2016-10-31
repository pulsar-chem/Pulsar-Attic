#ifndef PULSAR_GUARD_GRAPH__DFS_HPP_
#define PULSAR_GUARD_GRAPH__DFS_HPP_

#include <boost/graph/depth_first_search.hpp>

namespace pulsar{
namespace datastore {
namespace LibGraph{

template<typename Parent,typename Graph_t> class DFSBase;

/** \brief A class for running a depth first search (DFS)
 *
 *  DFS starts with a node and then finds a node directly connected to
 *  it. Next it follows that node to another connected nodes, and continues
 *  until no edges are left to follow; at this poitn DFS back tracks until it
 *  finds a node that still has edges to follow.  Once all edges have been
 *  traversed, execution aborts.  DFS is primarily used to categorize edges
 *  and to impose an order on vertices.  You can use this class to run a DFS
 *  on your graph.  As the search progresses the various functions will be
 *  called along the way.  By default this class does nothing besides record
 *  which nodes were seen.
 *
 *  This class can be derived from to put your own spin on the DFS
 *  algorithm.  To do this just modify
 *  the functions described in the next paragraph appropriately.  Note
 *  that BGL is mean and takes things by value, so if you want the object
 *  that goes into DFS to have the same state as the object you made, member
 *  data needs to be references/pointers.
 *
 *  Unfortunately, DFS and BFS do not have the same visitor points, for
 *  example BFS doesn't have  an EdgeDone() function.  Initially, this seemed
 *  odd to me, but trying to line them up better shows that you could define
 *  the same steps for both algorithms, but some of the steps would be
 *  superfluous as they occur immediately after the previous visitor call, i.e.
 *  there is no room for the state to change.  Nevertheless, I have
 *  attempted to line them up.
 *
    For our first node, BFS first calls FoundNode() whereas DFS calls a
    function start_vertex  colors it gray, and then calls FoundNode(); I have omitted the visitor
 *  point start_vertex to line those up seeing as how changing the color
 *  of the node is likely to illicit little response from the user.
 *  Next BFS takes a node from the queue, whereas DFS only has one node in the queue at all times (and
 *  doesn't actually have a queue for that reason); BFS calls LookAtNode()
 *  here, whereas there really isn't an equivalent call for DFS as we
 *  just discovered it and have done nothing since.  Both BFS and DFS then
 *  LookAtEdge() connected to their current node.  The type of edge is
 *  then evaluated with DFS being able to distinguish between back
 *  and cross edges, but BFS can't; unfortunately, DFS can have forward edges
 *  which it can't distinguish from cross edges.  For BFS we call
 *  OtherEdge() for a back or cross edge whereas for DFS we call OtherEdge() for
 *  cross and forward.  BackEdge() is called for DFS's back edges.
 *  For both BFS and DFS TreeEdge() is called for tree edges.  At this point,
 *  if the node is a tree edge DFS plunges down, whereas BFS continues on
 *  to the next edge.  After exhausting all edges BFS and DFS call
 *  NodeDone().  For DFS this signals the return from recursion and the
 *  end of considering the edge one level up prompting a call to EdgeDone().
 *  Either way both proceed to the next viable node, BFS by taking
 *  it from the queue, DFS by backtracking up a level and the cycle continues,
 *  until we are out of viable nodes.  Here an interesting thing occurs,
 *  BFS calls itself done, whereas DFS moves on to the next node that it
 *  hasn't seen.
 *
 *  I don't like the behavior just described.  What I have done is to
 *  stop DFS once it has exhausted the input node, making it akin to BFS.
 *  Both DFS and BFS can be told to run again, started on an arbitrary
 *  node.  Doing this for all nodes recovers the default DFS behavior of
 *  BGL.
 */
template<typename Graph_t>
class DFS: private DFSBase<DFS<Graph_t>,Graph_t>{
   protected:
      typedef DFSBase<DFS<Graph_t>,Graph_t> Base_t;
      typedef typename Base_t::Node_t Node_t;
      typedef typename Base_t::Edge_t Edge_t;
   public:
      ///No clean-up, but don't want no warning
      virtual ~DFS(){}
      ///Makes a DFS class that will work with Graph
      DFS(const Graph_t& Graph):DFSBase<DFS<Graph_t>,Graph_t>(Graph,*this){}

      ///Algorithm call back points
      ///@{
      ///Called the first time you see a node in the DFS algorithm
      virtual void FoundNode(const Node_t&){}
      ///Called when we start a new edge
      virtual void LookAtEdge(const Edge_t&){}
      ///Called when we see that our edge is a tree edge
      virtual void TreeEdge(const Edge_t&){}
      ///Called when we see that our edge is a back edge
      virtual void BackEdge(const Edge_t&){}
      ///Called when we see our edge is either a forward or cross edge
      virtual void OtherEdge(const Edge_t&){}
      ///Called when we exhaust a node
      virtual void NodeDone(const Node_t& Node){}
      ///Called when we finish using an edge by returning from recursion
      virtual void EdgeDone(const Edge_t& Edge){}
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

      ///The function for printing this beast
      virtual std::ostream& operator<<(std::ostream& os)const{
         return Base_t::operator<<(os);
      }
};

///Allows a BFS to be passed to an ostream
template<typename T>
inline std::ostream& operator<<(std::ostream& os, const DFS<T>& dfs){
   return dfs<<os;
}



///This is an adaptor class, don't directly use it.  Use dFS class.
template<typename Parent,typename Graph_t>
class DFSBase:public boost::default_dfs_visitor{
   private:
      class DFSException: public std::exception{};
   public:
      typedef typename Graph_t::Vertex_t Vertex_t;
      typedef typename Graph_t::Arc_t Arc_t;
      typedef typename Graph_t::NodeType Node_t;
      typedef typename Graph_t::EdgeType Edge_t;
      typedef std::map<Node_t,size_t> Map_t;
      typedef typename
            boost::vector_property_map<boost::default_color_type> Color_t;
      void start_vertex(const Vertex_t& Node,const Graph_t&){
         if(Colors_[Node]!=
               boost::color_traits<boost::default_color_type>::white())
            throw DFSException();//This is BGL wants us to abort early...
      }

      void discover_vertex(const Vertex_t& Node,const Graph_t&){
         Parent_.FoundNode(Graph_[Node]);
      }

      void examine_edge(const Arc_t& Edge,const Graph_t&){
         Parent_.LookAtEdge(Graph_[Edge]);
      }

      void tree_edge(const Arc_t& Edge,const Graph_t&){
         Parent_.TreeEdge(Graph_[Edge]);
      }

      void back_edge(const Arc_t& Edge,const Graph_t&){
         Parent_.BackEdge(Graph_[Edge]);
      }


      void forward_or_cross_edge(const Arc_t& Edge,const Graph_t&){
         Parent_.OtherEdge(Graph_[Edge]);
      }

      void finish_vertex(const Vertex_t& Node,const Graph_t&){
         Parent_.NodeDone(Graph_[Node]);
      }

      void finish_edge(const Arc_t& Edge,const Graph_t&){
         Parent_.EdgeDone(Graph_[Edge]);
      }

      void RunImpl(const Node_t& Node,bool Clean){
         if(Clean)Reset();
         try{
         boost::depth_first_visit(Graph_.Base_,Graph_.NodeLookUp_.at(Node),
               *this,Colors_);
         }
         catch(const DFSException& e){}//Just let control continue...
      }

      DFSBase(const Graph_t& Graph,Parent& P):
         Graph_(Graph),Parent_(P){Reset();}

      std::ostream& operator<<(std::ostream& os)const{
         return os;
      }
   protected:
      void Reset(){
         Colors_=Color_t(Graph_.NNodes());
         typename Graph_t::NodeItr_t NI=Graph_.NodeBegin(),
                                     NEnd=Graph_.NodeEnd();
         for(;NI!=NEnd;++NI)
            Colors_[Graph_.NodeLookUp_.at(*NI)]=
               boost::color_traits<boost::default_color_type>::white();
      }

      ///This is the graph for this search
      const Graph_t& Graph_;

      ///This is how BGL keeps track of the Nodes visited
      boost::vector_property_map<boost::default_color_type> Colors_;

      ///This is the derived class
      Parent& Parent_;
};

} // close namespace LibGraph
} // close namespace datastore
} // close namespace pulsar

#endif /* GRAPH_DFS_HPP_ */
