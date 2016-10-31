#ifndef PULSAR_GUARD_GRAPH__GRAPHITR_HPP_
#define PULSAR_GUARD_GRAPH__GRAPHITR_HPP_

namespace pulsar{
namespace datastore {
namespace LibGraph{

   ///Wrapper around BGL's iterators
     template<typename Return_t,typename Itr_t,typename Parent_t>
       class GraphItr{
          public:
             typedef GraphItr<Return_t,Itr_t,Parent_t> My_t;

             ///Allow user to copy iterator
             GraphItr(const My_t&)=default;

             ///Allow user to assign iterator
             GraphItr& operator=(const My_t&)=default;

             ///Advance this iterator
             GraphItr& operator++(){++Itr_;return *this;}

             ///Returns the edge/node the iterator is pointing to
             const Return_t& operator*()const{
                return Parent_[*Itr_];
             }

             ///Check if this iterator is equal to other
             bool operator==(const My_t& Other)const{
                return Itr_==Other.Itr_;
             }

             ///Check if this iterator is different than other
             bool operator!=(const My_t& Other)const{
                return !((*this)==Other);
             }

          private:


             ///Prohibit user from making new iterators
             GraphItr(Itr_t Itr,const Parent_t& Parent):
                        Itr_(Itr),Parent_(Parent){}

             Itr_t Itr_;

             const Parent_t& Parent_;

             ///Allow graph to create iterators
             friend Parent_t;

       };

} // close namespace LibGraph
} // close namespace datastore
} // close namespace pulsar

#endif /* GRAPH_GRAPHITR_HPP_ */
