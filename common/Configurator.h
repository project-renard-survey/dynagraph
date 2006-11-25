/**********************************************************
*            This software is part of Dynagraph           *
*                http://www.dynagraph.org/                *
*                                                         *
*            Copyright (c) 2006 Gordon Woodhull           *
*        Portions Copyright (c) 1994-2005 AT&T Corp.      *
*  Licensed under the Common Public License, Version 1.0  *
*                                                         *
*      Dynagraph originates in the Graphviz toolset       *
*                   http://graphviz.org                   *
*        Information and Software Systems Research        *
*              AT&T Research, Florham Park NJ             *
*                       Many thanks.                      *
**********************************************************/

#ifndef Configurator_h
#define Configurator_h

#include "common/StrAttr.h"
#include "common/ChangingGraph.h"
#include "ChangeProcessor.h"
#include "EnginePair.h"
#include <boost/mpl/assert.hpp>
#include <boost/mpl/apply.hpp>
#include <boost/mpl/front.hpp>
#include <boost/mpl/pop_front.hpp>
#include <boost/mpl/empty.hpp>
#include <boost/mpl/if.hpp>
#include <boost/mpl/pair.hpp>
#include <boost/mpl/clear.hpp>
#include <boost/mpl/vector.hpp>
#include <boost/mpl/for_each.hpp>
#include "common/generate_hierarchy.hpp"
#include <memory>

namespace Dynagraph {
namespace Configurator {

/*
	Configurators are semi-autonomous builders of Engines and Worlds
	The Configurators gets built from the inside out.
	Each Configurator (so far) either pre/appends Engines to the chain,
	or boxes up a chain and world into an InternalWorld, 
	and creates a new world in which it's an Engine.
	All of this sounds almost philosophical, no?  
	Unforch it craves better language; for now it's a mixed-up metaphor.

	Each configurator implements a method Create(), which is templated both on
	* the vector typelist of the rest of the Configurators
	* the layout type
	In this way it can conceivably act on any type of layout, and also it can change the type
	by wrapping it in another layer of layout.  When it's done it recursively calls createConfiguration
	so that the new type can be passed on to the next Configurator.  (Since the types are "passed" at
	compile time, this means that a tree of layout-specialized Configurators gets generated by the compiler.)
*/

// The Configuration type consists of a pair of CurrLevel,DataList
template<typename Configuration>
struct CurrLevel : boost::mpl::first<Configuration>
{};
template<typename Configuration>
struct DataList : boost::mpl::second<Configuration>
{};
template<typename CurrLevel,typename DataList> 
struct Configuration :
	boost::mpl::pair<CurrLevel,DataList>
{};
typedef Configuration<void,boost::mpl::clear<boost::mpl::vector<void> >::type > EmptyConfig;


// An instantiation of the Configuration glues items of DataList using MI
template<typename Configuration0>
struct Data : boost::generate_spine<typename DataList<Configuration0>::type> {
	// make original types easily accessible
	typedef Configuration0 Configuration;
	typedef typename CurrLevel<Configuration>::type CurrLevel;
	typedef typename DataList<Configuration>::type DataList;

	template<typename Data2>
	struct Copier {
		Data *data;
		Data2 *data2;
		Copier(Data *data,Data2 *data2) : data(data),data2(data2) {}
		template<typename T>
		void apply(T) {
			*static_cast<T*>(data) = *static_cast<T*>(data2);
		}
	};
	template<typename Data2>
	Data(Data2 &data2) {
		//boost::mpl::for_each<typename Data2::DataList>(Copier(this,&data2));
	}
	Data() {}
};

struct Pop {
	template<typename Configurators,typename Source,typename Dest> 
	static bool Create(DString name,const StrAttrs &attrs,Source &source,Dest dest) {
		typedef typename boost::mpl::front<Configurators>::type FirstConfigurator;
		typedef typename boost::mpl::pop_front<Configurators>::type Rest;
		return FirstConfigurator::template Create<Rest>(name,attrs,source,dest);
	}
};
struct DoNothing {
	template<typename Configurators,typename Source,typename Dest> 
	static bool Create(DString name,const StrAttrs &attrs,Source &source,Dest dest) {
		return false;
	}
};
template<typename Configurators,typename Source,typename Dest> 
static bool Create(DString name,const StrAttrs &attrs,Source &source,Dest dest) {
	return boost::mpl::if_<boost::mpl::empty<Configurators>,DoNothing,Pop>::type
		::template Create<Configurators>(name,attrs,source,dest);
}
// default implementation of the Configurator concept: just forward
struct ConfigConcept {
	template<typename Configurators,typename Source,typename Dest> 
	static bool Create(DString name,const StrAttrs &attrs,Source &source,Dest dest) {
		return Configurator::Create<Configurators>(name,attrs,source,dest);
	}
};
// some common Configuratons
template<typename Layout,typename Tag>
struct Engine {
	std::auto_ptr<ChangeProcessor<Layout> > engine;
};
template<typename Layout,typename Tag>
struct Level {
	std::auto_ptr<ChangingGraph<Layout> > world;
	EnginePair<Layout> engines; // owned by others
};

// some level tags (this much not at all generalized--?)
struct LayoutLevel {};
struct EmphasizedLevel {};
struct OutermostLevel {};

} // namespace Configurator
} // namespace Dynagraph

#endif // Configurator_h
