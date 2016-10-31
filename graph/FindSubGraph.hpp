#ifndef PULSAR_GUARD_GRAPH__FINDSUBGRAPH_HPP_
#define PULSAR_GUARD_GRAPH__FINDSUBGRAPH_HPP_

#include <map>
#include <vector>
#include <functional>
#include <boost/graph/vf2_sub_graph_iso.hpp>


namespace pulsar{
namespace datastore {
namespace LibGraph{

/** \brief Finds subgraphs of a graph
 *
 *
 *  Given a large graph, a common problem is to find a subgraph of the
 *  original graph.  More specific, a subgraph \f$H\f$ of some graph \f$G\f$
 *  is a graph whose nodes are a subset of those in \f$G\f$ and whose edges
 *  are also a subset of those in \f$G\f$.  In other words, \f$H\f$ can
 *  not contain any nodes or edges that are not in \f$G\f$.  If there is at
 *  least one node or edge in \f$G\f$ that is not in \f$H\f$ then \f$H\f$ is
 *  said to be a proper subgraph.  Furthermore, if can be formed from
 *  \f$G\f$ by deleting nodes from \f$G\f$ (as well as
 *  all edges that end in those deleted nodes), then \f$H\f$ is said to
 *  be an induced (proper) subgraph of \f$G\f$.
 *
 *  Note that by default this will find all isomorphisms between your two
 *  graphs.
 *
 *
 *  Syntax to use this class:
 *  \code
 *  FindSubGraph<Your Graph's type> FSG(YourGraph);
 *  FSG.Run(SubGraph);
 *  \endcode
 *
 *  You may optionally specify the criteria for determining if your
 *  nodes/edges are equal.  By default their operator==() member will
 *  be called.  If they do not have such a member, or you want to override
 *  it (particularly for dereferencing pointers...) you may supply your own
 *  functors.  They should return whether the nodes/edges are equal as a
 *  bool and a const reference to two nodes/edges (although it shouldn't
 *  matter, the first will be from the full graph, the second from the
 *  subgraph).
 *
 */
template<typename Graph_t>
class FindSubGraph{
   private:
      ///The type of nodes in the graphs
      typedef typename Graph_t::NodeType Node_t;
      ///The type of edges in the graphs
      typedef typename Graph_t::EdgeType Edge_t;
      ///The type of an edge mapping between the two graphs
      typedef std::map<Node_t,Node_t> Map_t;
      ///What BGL uses for nodes
      typedef typename boost::graph_traits<Graph_t>::vertex_descriptor
            V_t;
      ///What BGL uses for edges
      typedef typename boost::graph_traits<Graph_t>::edge_descriptor E_t;
      ///The type of a functor for comparing our node types
      typedef std::function<bool (const Node_t&,const Node_t&)> NodeComp_t;
      ///The type of a functor for comparing our edge types
      typedef std::function<bool (const Edge_t&,const Edge_t&)> EdgeComp_t;

      ///A functor for handling BGL's call back interface
      class CallBack{
         private:
            //The search this callback is tied to
            FindSubGraph<Graph_t>* Parent_;
            const Graph_t& LGraph_;//Large graph
            const Graph_t& SGraph_;//Small graph
            bool KeepGoing_;//Stop after this induced subgraph?
         public:
            ///Takes the subgraph search, the large and small graphs
            CallBack(FindSubGraph<Graph_t>* Parent,
                     const Graph_t& Graph,
                     const Graph_t& SubGraph,
                     bool KeepGoing=true):
               Parent_(Parent),LGraph_(Graph),SGraph_(SubGraph),
               KeepGoing_(KeepGoing){}

         ///The call BGL will use upon finding a subgraph
         template<typename Map1To2,typename Map2To1>
         bool operator()(Map1To2 Map1,Map2To1 Map2)const{
            Map_t Temp1,Temp2;
            BGL_FORALL_VERTICES_T(v,SGraph_,Graph_t){
               if(get(Map1,v)!=boost::graph_traits<Parent_::Base_t>::null_vertex())
                  Temp1[LGraph_[get(Map1,v)]]=SGraph_[v];
            }
            Parent_->Large2Small_.push_back(Temp1);
            //Parent_->Small2Large_.push_back(Temp2);
            return KeepGoing_;
         }
      };

      ///A dereference functor for the nodes/edges
      template<typename Fxn_t>
      class Compare{
         private:
            ///The function to call
            Fxn_t fxn_;
            ///The large graph
            const Graph_t& Large_;
            ///The small graph
            const Graph_t& Small_;
         public:
            ///Takes the large, small, and compare fxn
            Compare(const Graph_t& Large,const Graph_t& Small,Fxn_t& fxn):
               fxn_(fxn),Large_(Large),Small_(Small){}
            ///Dereferences a piece from the large and small graphs
            template<typename Part_t>
            bool operator()(const Part_t& p1,const Part_t& p2)const{
               return fxn_(Large_[p2],Small_[p1]);
            }
      };

      ///What will be the large graph
      const Graph_t& Graph_;

      ///The functor for comparing two nodes for equality
      NodeComp_t NodeComp_;

      ///The functor for comparing two edges for equality
      EdgeComp_t EdgeComp_;

      ///A mapping from the large graph to the small graph
      std::vector<Map_t> Large2Small_;

   public:
      ///Takes the large graph and optionally a way of comparing the nodes
      ///and the edges.  Compare functors should model equality
      FindSubGraph(const Graph_t& Graph,
                   NodeComp_t NodeComp=std::equal_to<Node_t>(),
                   EdgeComp_t EdgeComp=std::equal_to<Edge_t>()):
                   Graph_(Graph),NodeComp_(NodeComp),EdgeComp_(EdgeComp){}

      ///Returns the number of isomorphisms this class has found
      size_t NMatches()const{return Large2Small_.size();}

      ///Returns the i-th isomorphism
      size_t Match(size_t i)const{return Large2Small_[i];}

      ///Returns true if any isomorphisms have been found
      ///(clears existing isomorphisms)
      bool Run(const Graph_t& SubGraph,
             bool StopOnFind=false,
             bool Induced=true
            ){
         Large2Small_.clear();
         //The BGL allows us to specify a search order and in its
         //infinite wisdom decides we'd want to override that before we'd
         //want to stop letting all nodes be equal...Moral is we are on
         //the hook for this...The default is to order them by
         //multiplicity (largest to smallest),
         //which is all this nasty next set of lines does

         std::set<V_t,std::function<bool(const V_t&,const V_t&)> > Order(
               [&SubGraph](const V_t& v1,const V_t& v2){
               size_t LEdges=SubGraph.NEdges(SubGraph[v1]),
                      REdges=SubGraph.NEdges(SubGraph[v2]);
               bool LessEdges=LEdges<REdges,MoreEdges=LEdges>REdges;
               return (!LessEdges && !MoreEdges? v2<v1 : MoreEdges);
         });
         typename Graph_t::NodeItr_t NI=SubGraph.NodeBegin(),
                                     NEnd=SubGraph.NodeEnd();
         for(;NI!=NEnd;++NI)
            Order.insert(SubGraph.NodeLookUp_.at(*NI));

         CallBack CB(this,Graph_,SubGraph,!StopOnFind);
         Compare<NodeComp_t> VComp(Graph_,SubGraph,NodeComp_);
         Compare<EdgeComp_t> EComp(Graph_,SubGraph,EdgeComp_);
         return boost::vf2_subgraph_iso(SubGraph,Graph_.base_,CB,Order
                      ,boost::vertices_equivalent(VComp).
                       edges_equivalent(EComp)
                 );
                /* This is the call for non-induced
                 *
                 * boost::vf2_subgraph_mono(SubGraph,Graph_,CB,
                 *       boost::vertices_equivalent(NC))
                 */

      }

      std::ostream& operator<<(std::ostream& os)const{
         typename std::vector<Map_t>::const_iterator MapI=Large2Small_.begin(),
               MapEnd=Large2Small_.end();
         for(;MapI!=MapEnd;++MapI){
            typename Map_t::const_iterator NodeI=MapI->begin(),NodeEnd=MapI->end();
            for(;NodeI!=NodeEnd;++NodeI){
               os<<NodeI->first<<" ---> "<<NodeI->second<<std::endl;
            }
            os<<std::endl;
         }
         return os;
      }


};

} // close namespace LibGraph
} // close namespace datastore
} // close namespace pulsar


#endif /* GRAPH_FINDSUBGRAPH_HPP_ */
