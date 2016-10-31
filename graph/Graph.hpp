#ifndef PULSAR_GUARD_GRAPH__GRAPH_HPP_
#define PULSAR_GUARD_GRAPH__GRAPH_HPP_

#include <functional>
#include <tuple>
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/adjacency_matrix.hpp>
#include <boost/graph/graph_utility.hpp>
#include <boost/graph/graphviz.hpp>

#include "pulsar/datastore/graph/FillMacro.h"
#include "pulsar/datastore/graph/GraphItr.hpp"

namespace pulsar{
namespace datastore {
namespace LibGraph{

///Forward declarations of algorithms
template<typename U> class DFS;
template<typename T,typename U> class DFSBase;
template<typename U> class BFS;
template<typename T,typename U> class BFSBase;
template<typename U> class FindSubGraph;

/** \brief A basic graph object
 *
 *  To some extent a graph is a glorified container.  However unlike the
 *  containers in the STL, graphs have two data locations: the nodes and
 *  the edges (although chemists forget the latter sometimes).  This
 *  means traversal of the graph is more complicated than it would be for
 *  say a list.  Furthermore edges may be directed, repeated (also known as
 *  parallel), or cyclic and we may want to traverse it in such a manner
 *  that we cross the minimal number of nodes, or the path we take has
 *  the minimal weight.  The point is, there are lots of things one may
 *  want to do with a graph and this class aims to allow you to do
 *  all of them.
 *
 *  This object is built around Boost's graph library (BGL).  This library
 *  has some weird quirks that I have tried to make transparent to you.
 *  However, one of these oddities that I can't quite hide is the fact that
 *  BGL both wants
 *  to manage your memory and doesn't at the same time.  Without going
 *  into much detail. I strongly suggest you make each node/edge in
 *  the graph be a shared pointer to a dynamically allocated instance.  Then
 *  reference the nodes/edges with shared pointers to these instances.
 *  For example:
 *  \code
 *  std::shared_ptr<MyClass> Node1(new MyClass);
 *  std::shared_ptr<MyClass> Node1Copy(new MyClass(*Node1));
 *  MyGraph.AddNode(Node1);
 *
 *  //...other stuff...
 *
 *  //Ask for number of edges Node1 has
 *  MyGraph.NEdges(Node1);
 *
 *  //One GOTCHA of this approach is you need to use shared pointers to
 *  //the *actual* objects, i.e.
 *  //This is NOT equivalent to the last call b/c they don't point to the same instance
 *  MyGraph.NEdges(Node1Copy);
 *
 *  Node1Copy=Node1;
 *  //Now it will work as both point to the same instance
 *  MyGraph.NEdges(Node1Copy);
 *  \endcode
 *
 *  The actual requirements for your Node and Edge types are that they
 *  are comparable via operator<() (this is for sorting) and that each
 *  node/edge is unique (this is because you will be looking them up by value
 *  so how can I tell the difference between say:
 *  \code
 *  int Node1=3,Node2=3;
 *
 *  ///Will this return the number of edges from Node1 or Node2???
 *  MyGraph.NEdges(3);
 *  \endcode
 *  unless they are unique?).  Pointers (either normal or shared) will
 *  enforce both of these requirements,  as each unique instance of your
 *  object will have a unique address and addresses can be compared using less than.
 *  Note that it is not important that we compare the actual objects,
 *  i.e. you don't have to write a less than operator that will dereference
 *  the pointer to do the comparison.  For sorting, all that matters is that
 *  there is an
 *  order, not that the order reflects the physical ordering of the
 *  pointed to objects.
 *
 *  Last point about your Node/Edge types, there will be a fair bit of
 *  copying involved b/c of BGL's philosophies (you can read them in
 *  their FAQ section) so your Node/Edge types should be light weight.
 *  Again this is why I recommend the pointer option, but there are
 *  some scenarios in which you want to be able to construct a separate
 *  node or graph and compare them.  In these cases I recommend you keep
 *  the objects thin.  Remember if you are just writing a one-off wrapper
 *  class, you don't need to put it in a header, you can put it in the
 *  cc file (or even inline in the function you are writing...).
 *
 *  By default the graph is bidirectional and allows for parallel edges.
 *  The latter is usually an undesired feature and can be rigorously
 *  prevented by using std::set to hold the edges, but at somewhat
 *  substantial extra cost to all algorithms.
 *
 *  I'll be honest, the actual code declarations below are pretty nasty,
 *  but their
 *  form is necessary to provide the syntax I'm about to describe, while
 *  not squelching BGL's flexibility.  To use this class I strongly recommend
 *  referring to this documentation rather than reading the source code.
 *  Consequentially add to this documentation if you think something is
 *  missing.
 *
 *  Anyways, to instantiate a graph, where each node is named with,
 *  say a string we do (as you will see all our strings are unique and they
 *  are small enough to just be copied by value, so we don't have to use
 *  pointers like described above):
 *  \code
 *  //Makes the graph, which has strings on the nodes
 *  Graph<std::string> MyGraph;
 *
 *  //Name node 1: "Node 1" (constructor actually supports an arbitrary
 *  // number of comma separated input nodes)
 *  MyGraph.AddNode("Node 1");
 *
 *  //Name nodes 2 and 3: "Node 2" and "Node 3" respectively
 *  //(Initializer list also supports an arbitrary number of new nodes)
 *  MyGraph.AddNode({"Node 2","Node 3"});
 *
 *  //If our nodes are already in some container, like:
 *  std::vector<std::string> NodeNames({"Node 4","Node 5","Node 6"});
 *
 *  //...can use iterator add:
 *  MyGraph.AddNode(NodeNames.begin(),NodeNames.end());
 *  \endcode
 *  Note that although we referred to the first node we made as node 1, etc.
 *  the nodes of a graph form an unordered set, so you should not rely on
 *  this class to maintain them in this order, especially if you
 *  ever delete some nodes.  If the order is important you can
 *  always bundle the order you made them with your object.
 *  The AddNode and construction steps can be combined as the constructor
 *  supports initialization with any of the three different ways of
 *  adding nodes.
 *
 *  Once we have some nodes, we can add edges.  In order to add an edge
 *  between two nodes, say \f$A\f$ and \f$B\f$, both \f$A\f$ and \f$B\f$
 *  must have already been added to the graph, and must still be there.
 *  Adding edges is slightly more complicated because in addition to the
 *  information stored on the edge (if any) we must also specify where the
 *  edge comes from and where it goes to (for an undirected graph the
 *  distinction between which node comes first is arbitrary, including in
 *  any returned edge).  By default the edge class is just an std::tuple
 *  of two of your node types (we'll discuss what to do if you want data
 *  on the edge in a second).
 *  If you forget what the edge's type is you can always get the type via
 *  EdgeType, as is shown in the code example:
 *  \code
 *  //Get the type of the edge for your graph
 *  //Will be std::tuple<std::string,std::string> in this case
 *  typedef Graph<std::string>::EdgeType Edge_t;
 *
 *  //Make an edge between the first two nodes we made
 *  MyGraph.AddEdge(Edge_t("Node 1","Node 2"));
 *
 *  //Can make several edges at once in an initializer list
 *  MyGraph.AddEdge({Edge_t("Node 1","Node 3"),Edge_t("Node 2","Node 3")});
 *
 *  //Or again put them in a container
 *  std::array<Edge_t,2> Edges({Edge_t("Node 3","Node 4"),
 *                              Edge_t("Node 5","Node 6")});
 *
 *  //Then use iterators
 *  MGraph.AddEdge(Edges.begin(),Edges.end());
 *  \endcode
 *  Note that by default our graph is a directed graph so these edges all
 *  run from the first node to the second node.
 *
 *  Similar code is used if we put things on the edges:
 *  \code
 *  //This will be the type of our Edges
 *  //Make life easy on yourself and just use an std::tuple where the
 *  //first two types are the same as your nodes
 *  typedef std::tuple<std::string,std::string,int> Edge_t;
 *
 *  //Declare the graph with strings on nodes and ints on edges
 *  //also start it with two nodes
 *  Graph<std::string,Edge_t> MyGraph2("Node 1","Node 2");
 *
 *  //We can still grab the type of an edge, but it's a bit silly presently
 *  //given that we just set it...
 *  typedef Graph<std::string,Edge_t>::EdgeType EdgeType;
 *
 *  //Make and add an edge
 *  MyGraph2.AddEdge(Edge_t("Node 1","Node 2",1));
 *  \endcode
 *
 *  Again graphs are by default bidirectional, if your graph doesn't have
 *  direction (equivalent to all nodes being a source and sink for all
 *  nodes they are connected to) there is computational speed-up that can
 *  be gained by noting this.  To do this, you can use the undirected graph
 *  typedef that we have provided:
 *  \code
 *  //Makes a graph where the edges are undirected and nodes are strings
 *  UGraph<std::string> MyGraph3;
 *
 *  //For consistency we have also provided a typedef that makes the
 *  //bidirectional nature of the default graph more apparent
 *
 *  BiGraph<std::string> MyGraph4;
 *
 *  //The above construction of MyGraph4 is exactly equivalent to:
 *  //Graph<std::string> MyGraph4;
 *
 *  //Also provided are dense versions of the above, although BGL's support
 *  //for dense graphs appears spotty at the moment...
 *
 *  //A dense undirected graph with strings for nodes
 *  DenseUGraph<std::string> MyGraph5;
 *
 *  //A dense bidirected graph with strings for nodes
 *  DenseBiGraph<std::string> MyGraph6;
 *  \endcode
 *
 *  All of these typedefs have exactly the same interface as far as you the
 *  user is concerned.  The types simply change the compile time logic.
 *
 *  Now that we know how to specify a graph let's talk about how to do
 *  stuff with it.
 *  \code
 *  //The most trivial operations:
 *  size_t NNodes=MyGraph.NNodes();//Returns the number of nodes
 *  size_t NEdges=MyGraph.NEdges();//Guess what this does...(Returns the number of edges)
 *
 *  //Removes any node, who by value, is equal to "Node 1"
 *  MyGraph.RemoveNode("Node 1");//This also clears all edges attached to it
 *
 *  //Removes the edge from "Node 3" to "Node 4"
 *  MyGraph.RemoveEdge(Edges[0]);
 *
 *  //Also could have done (don't call both though):
 *  //MyGraph.RemoveEdge("Node 3","Node 4");
 *  \endcode
 *
 *  BGL provides a host of algorithms, which because of the way we have
 *  defined this class, will take our graph object.  Hence if I have not
 *  wrapped your favorite BGL algorithm already you can still call it
 *  passing in this class.  That being said, the BGL algorithms, like the
 *  rest of the BGL, are a pain in the butt to use so I've went ahead and
 *  started wrapping them.  At the moment I have wrapped:
 *
 *  1. Breadth first search (class: BFS)
 *  2. Depth first search (class: DFS)
 *  2. Subgraph searches (class:: FindSubGraph)
 *
 *  Other interesting, and likely to be wrapped eventually, algorithms
 *  include: shortest path, minimum cut,
 *
 *
 *
 *  ******************* Technical Garbage Resides Below *******************
 *
 *
 *  Implementation notes (i.e. stuff you may want to know if you care
 *  how this wrapper works, but it is ultimately lots of nasty meta-template
 *  programming and rambilings pertaining to my experience with the BGL).
 *
 *
 *  At the moment this class is a wrapper around BGL.  Within the BGL, the
 *  fundamental class is
 *  the adjacency_list.  The name comes from the mathematical concept of an
 *  adjacency list, which is simply a list of the nodes connected to a
 *  given node.  You can think about it as a jagged array, with a row for
 *  each node and within a row a list of nodes that that row's node is
 *  connected to, i.e. the first element in the list is a list of nodes
 *  connected to the first node.  In practice this may
 *  not be how the data is actually stored, but conceptually this is how
 *  it is used.  Another way of thinking about a graph is as a real matrix
 *  (not a jagged array).  When done this way the adjacency list is
 *  typically called an adjacency matrix (which BGL implements via the
 *  creatively name adjacency_matrix class).  If
 *  we have say \f$N\f$ nodes, the adjacency matrix is \f$N\f$ by \f$N\f$.
 *  Letting this matrix be called \f$A\f$, \f$A_{ij}\f$ is the weight of an
 *  edge going from node \f$i\f$ to node \f$j\f$ (a weight of zero denotes that
 *  there is no such edge).  For a directed graph \f$A_{ij}\f$ need not
 *  equal \f$A_{ji}\f$, whereas conceptually an undirected graph is
 *  represented by a symmetric matrix.  The diagonal elements keep track
 *  of edges that start and end on the same node, so called self-loops.
 *  For dense graphs the adjacency matrix is usually preferred, whereas
 *  for sparse graphs the adjacency list is better. Density of a graph is
 *  usually defined as the number of edges divided by the square of the
 *  number of nodes; this (multiplied by 100) is the percentage of \f$A\f$
 *  that is filled.  If \f$A\f$ is mainly zeros the matrix representation
 *  is overkill.
 *
 *  BGL by default uses an adjacency list, thus it is best suited for
 *  sparse graphs, i.e. most nodes are not connected to each other,
 *  which is the usual scenario.  It demands we choose the
 *  container that holds our nodes and the container that holds our edges.
 *  Choices are: vecS (std::vector), listS (std::list), slistS (std::slist),
 *  setS (std::set), multisetS (std::multiset), and hash_setS
 *  (boost::unordered_set).  For vertices, I have made vecS the default
 *  choice as it allows quick insertion of vertices.  If you are going
 *  to delete large amounts of vertices, than listS will be a better
 *  choice.  In general the container must be a
 *  sequence and allow random access.  For edges one important design
 *  consideration is whether you allow parallel edges or not.  If you
 *  are not sure if someone is going to give you a parallel edge and you
 *  can't deal with them use setS.  This comes at additional lookup cost.
 *  I went for vecS as it is the fastest container to traverse and it
 *  seems somewhat unlikely that you wouldn't know you're getting a parallel
 *  edge.
 *
 *  The algorithms maintain their generality via property maps.  Basically,
 *  these are little wrapper structs that enforce a common interface on
 *  very uncommon objects.  As far as the objects you are sticking on the
 *  nodes and edges go you have to determine who owns the memory, the graph
 *  or someone else.  The former case is known as interior properties and
 *  the latter exterior.  The interior properties sound great at first until
 *  you realize that BGL maps them to size_t and pairs of size_t for nodes
 *  and edges respectively.  This is fine if it did this internally, but
 *  BGL demands you use the size_t and pairs of size_t mappings that it makes
 *  to access your elements.  This is why we need to store an std::map
 *  between the interior properties the user gave us and the stupid look
 *  up values BGL gives us.  BGL calls interior properties, bundled
 *  properties when they are part of the adjacency_list type.
 *
 *  In what amounts to an uber pain in my butt, the type of the returned
 *  iterator is different depending on a bunch of things.  I would
 *  like to get this down to just two iterators (one for a set of nodes and
 *  one for a set of edges), but type erasure for iterators seems difficult
 *  if not impossible.  At the moment I have an iterator for looping
 *  over all nodes and one for all edges.  If you
 *  want a subset (say all nodes emanating from node I), you get
 *  an std::vector of that subset.  This is because in the BGL,
 *  the iterators that go over all nodes have a different type than the
 *  iterators that go over only those adjacent to our node.  In theory this
 *  is close to what we need.
 *
 *  \todo Write two and only two iterators
 *
 *
 *   Lastly, this class takes a host of template parameters, many of which
 *   are just forwarded to BGL.  We derive from the adjacency_list so that
 *   the object can be passed to the BGL as is.
 *
 *  \param Node_t the type of the object stored on your nodes
 *  \param Edge_t the type of the object stored on your edges
 *                must be an std::tuple (or usable with std::get) such that
 *                the first two elements are of type Node_t and respectively
 *                are the source and the sink
 *  \param EdgeCon_t The container structure that holds the edges (default std::vector)
 *  \param NodeCon_t The container structure that holds the nodes (default std::vector)
 *  \param Impl_t The BGL class that actually implements the graph
 */
 template<typename Node_t,typename Edge_t=std::tuple<Node_t,Node_t>,
          typename EdgeCon_t=boost::vecS,typename NodeCon_t=boost::vecS,
          typename Impl_t=
              boost::adjacency_list<EdgeCon_t,NodeCon_t,
                     boost::bidirectionalS,Node_t,Edge_t>
>
 class Graph{
    private:
       ///Typedef of this's type
       typedef Graph<Node_t,Edge_t,EdgeCon_t,NodeCon_t,Impl_t> My_t;

       ///Typedef of this's base
       typedef Impl_t Base_t;

       ///Users don't need this typedef (it's what BGL maps our nodes to)
       typedef typename Impl_t::vertex_descriptor Vertex_t;

       ///Users don't need this typedef (it's what BGL maps our edges to)
       typedef typename Impl_t::edge_descriptor Arc_t;
       
       Base_t Base_;

       ///This is so I can dereference BGL's node types back to what you want
       std::map<Node_t,Vertex_t> NodeLookUp_;

       ///This is so I can dereference BGL's edges back to what you want
       std::map<Edge_t,Arc_t> EdgeLookUp_;

       ///So that algorithms can work on our wrapped class
       friend BFSBase<BFS<My_t>,My_t>;
       friend DFSBase<DFS<My_t>,My_t>;
       friend FindSubGraph<My_t>;


    public:

       ///Type of an object that is on an edge
       typedef Edge_t EdgeType;

       ///Type of the an object on a node
       typedef Node_t NodeType;

       ///Type of an iterator to a set of nodes
       typedef GraphItr<Node_t,typename Impl_t::vertex_iterator,My_t> NodeItr_t;

       ///Type of an iterator to a set of edges
       typedef GraphItr<Edge_t,typename Impl_t::edge_iterator,My_t> EdgeItr_t;


       ///Various ways of initializing Nodes/Edges
       ///@{
       ///Constructors (only initializing nodes)
       ///@{

       // A bug in GCC prevents this from being used in an
       // explicitly-instantiated class. So we have to do it the old
       // fashioned way...
       // (bug: https://gcc.gnu.org/bugzilla/show_bug.cgi?id=51629)
       //Graph()=default;
       Graph() { };

       DEFINE_FILL_FXNS(Node_t,FillNodes,Graph,)
       ///@}

       ///Node filler upppers
       ///@{
       DEFINE_FILL_FXNS(Node_t,FillNodes,AddNode,void)
       ///@}

       ///Edge filler uppers
       ///@{
       DEFINE_FILL_FXNS(Edge_t,FillEdges,AddEdge,void)
       ///@}
       ///@}

       ///Removes NodeI (and all edges to it) all iterators are invalidated
       void RemoveNode(const Node_t& NodeI){
          boost::clear_vertex(NodeLookUp_.at(NodeI),Base_);
          boost::remove_vertex(NodeLookUp_.at(NodeI),Base_);
       }

       ///Removes edge from NodeI to NodeJ iterators to edges are invalidated
       void RemoveEdge(const Node_t& NodeI,const Node_t& NodeJ){
          boost::remove_edge(
          boost::edge(NodeLookUp_.at(NodeI),NodeLookUp_.at(NodeJ),Base_).first,
           Base_);
       }

       ///Removes the passed in edge, iterators to edges are invalidated
       void RemoveEdge(const Edge_t& Edge){
          boost::remove_edge(EdgeLookUp_[Edge],Base_);
       }


       /** \brief Node accessors*/
       ///@{
       ///Returns the number of nodes in the graph
       size_t NNodes()const{return boost::num_vertices(Base_);}

       ///Returns an iterator to the first node
       NodeItr_t NodeBegin()const{
           return NodeItr_t(boost::vertices(Base_).first,*this);
       }

       ///Returns an iterator to the last node
       NodeItr_t NodeEnd()const{
           return NodeItr_t(boost::vertices(Base_).second,*this);
       }

       ///Returns an std::vector of the Nodes connected to NodeI
       std::vector<Node_t> ConNodes(const Node_t& NodeI)const{
           std::vector<Node_t> temp;
           typedef typename Impl_t::adjacency_iterator Itr_t;
           std::pair<Itr_t,Itr_t> Its=
                 boost::adjacent_vertices(NodeLookUp_.at(NodeI),Base_);
           for(;Its.first!=Its.second;++Its.first)
              temp.push_back(Base_[*Its.first]);
          return temp;
       }
       ///@}

       /** \brief Edge accessors*/
       ///@{
       ///Returns the number of edges in the graph
       size_t NEdges()const{return boost::num_edges(Base_);}

       ///Returns the number of edges emanating from NodeI
       size_t NEdges(const Node_t& NodeI)const{
          return boost::out_degree(NodeLookUp_.at(NodeI),Base_);
       }
       ///Returns the number of edges ending in NodeI
       size_t NInEdges(const Node_t& NodeI)const{
          return boost::in_degree(NodeLookUp_.at(NodeI),Base_);
       }

       ///Returns an iterator to the first edge
       EdgeItr_t EdgeBegin()const{
           return EdgeItr_t(boost::edges(Base_).first,*this);
       }

       ///Returns an iterator just past the last edge
       EdgeItr_t EdgeEnd()const{
           return EdgeItr_t(boost::edges(Base_).second,*this);
       }

       ///Returns an std::vector of edges emanating from NodeI
       std::vector<Edge_t> Edges(const Node_t& NodeI)const{
          std::vector<Edge_t> temp;
          typedef typename Impl_t::out_edge_iterator Itr_t;
          std::pair<Itr_t,Itr_t> Its=
                boost::out_edges(NodeLookUp_.at(NodeI),Base_);
          for(;Its.first!=Its.second;++Its.first)
             temp.push_back(Base_[*Its.first]);
         return temp;
       }

       ///Returns an std::vector of edges ending in NodeI
       std::vector<Edge_t> InEdges(const Node_t& NodeI)const{
          std::vector<Edge_t> temp;
          typedef typename Impl_t::in_edge_iterator Itr_t;
          std::pair<Itr_t,Itr_t> Its=
                boost::in_edges(NodeLookUp_.at(NodeI),Base_);
          for(;Its.first!=Its.second;++Its.first)
             temp.push_back(Base_[*Its.first]);
         return temp;
       }
       ///@}


       ///Returns true if two nodes are connected such that u-->v
       bool AreConn(const Node_t& u,const Node_t& v)const{
          return boost::edge(NodeLookUp_.at(u),NodeLookUp_.at(v),Base_).second;
       }

       ///Prints graph out, assumes your nodes can be passed to std::ostream
       std::ostream& operator<<(std::ostream & os)const{
          class label_writer {
          public:
            label_writer(const Impl_t& _name) : name(_name) {}
            void operator()(std::ostream& out, const Vertex_t& v) const {
              out << "[label=\"" << name[v] << "\"]";
            }
            ///todo: Actually print edges
            void operator()(std::ostream& out, const Arc_t&)const{
               out<<"[label=\""<<'\0'<<"\"]";
            }
          private:
            const Impl_t& name;
          };

          boost::write_graphviz(os,Base_,label_writer(Base_));
          /*os<<"Graph contains: "<<NNodes()<<" nodes and "
                   <<NEdges()<<" edges."<<std::endl;
          NodeItr_t NI=NodeBegin(),NIEnd=NodeEnd();
          for(;NI!=NIEnd;++NI){
             os<<*NI<<"'s connections:"<<std::endl;
             std::vector<Node_t> temp=ConNodes(*NI);
             typename std::vector<Node_t>::iterator NJ=temp.begin(),
                   NJEnd=temp.end();
             for(;NJ!=NJEnd;++NJ)os<<*NI<<" --> "<<*NJ<<std::endl;
          }*/
          return os;
       }

    private:
       ///Actual function that fills in the BGL base class
       template<typename BeginItr_t,typename EndItr_t>
       void FillNodes(BeginItr_t BeginItr, EndItr_t EndItr){
          for(;BeginItr!=EndItr;++BeginItr){
             NodeLookUp_[*BeginItr]=
                   boost::add_vertex(*BeginItr,Base_);
          }
       }

       /** \brief Fills in the edges the user gave us */
       template<typename BeginItr_t,typename EndItr_t>
       void FillEdges(BeginItr_t BeginItr,EndItr_t EndItr){
          for(;BeginItr!=EndItr;++BeginItr)
             EdgeLookUp_[*BeginItr]=
             boost::add_edge(NodeLookUp_.at(std::get<0>(*BeginItr)),
                             NodeLookUp_.at(std::get<1>(*BeginItr)),
                             Base_).first;
       }
 };

 template<typename Node_t,typename Edge_t,typename EdgeCon_t,
          typename NodeCon_t,typename Impl_t>
inline std::ostream& operator<<(std::ostream& os,
      const Graph<Node_t,Edge_t,EdgeCon_t,NodeCon_t,Impl_t>& g){
      return g<<os;
 }


 ///A graph in which all nodes may be sources and sinks
 template<typename Node_t,typename Edge_t>
 using BiGraph=Graph<Node_t,Edge_t>;

 ///A graph with no direction
 template<typename Node_t,typename Edge_t,
          typename EdgeCon_t=boost::vecS,
          typename NodeCon_t=boost::vecS>
 using UGraph=Graph<Node_t,Edge_t,EdgeCon_t,NodeCon_t,
       boost::adjacency_list<EdgeCon_t,NodeCon_t,boost::undirectedS,
       Node_t,Edge_t> >;

 ///A dense graph with no direction
 template<typename Node_t,typename Edge_t,
          typename EdgeCon_t=boost::vecS,
          typename NodeCon_t=boost::vecS>
 using DenseUGraph=Graph<Node_t,Edge_t,EdgeCon_t,NodeCon_t,
       boost::adjacency_matrix<boost::undirectedS,NodeCon_t,EdgeCon_t> >;

 ///A dense bidirectional graph
 template<typename Node_t,typename Edge_t,
          typename EdgeCon_t=boost::vecS,
          typename NodeCon_t=boost::vecS>
 using DenseBiGraph=Graph<Node_t,Edge_t,EdgeCon_t,NodeCon_t,
       boost::adjacency_matrix<boost::bidirectionalS,NodeCon_t,EdgeCon_t> >;

} // close namespace LibGraph
} // close namespace datastore
} // close namespace pulsar


#endif /* GRAPH_GRAPH_HPP_ */
