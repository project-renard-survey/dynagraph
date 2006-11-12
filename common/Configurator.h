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

namespace Dynagraph {

/*
	Configurators are semi-autonomous builders of Engines and Worlds
	The Configurators gets built from the inside out.
	Each Configurator (so far) either pre/appends Engines to the chain,
	or boxes up a chain and world into an InternalWorld, 
	and creates a new world in which it's an Engine.
	All of this sounds almost philosophical, no?  
	Unforch it craves better language; for now it's a mixed-up metaphor.

	Each configurator implements a method config(), which is templated both on
	* the vector typelist of the rest of the Configurators
	* the layout type
	In this way it can conceivably act on any type of layout, and also it can change the type
	by wrapping it in another layer of layout.  When it's done it recursively calls configureLayout
	so that the new type can be passed on to the next Configurator.  (Since the types are "passed" at
	compile time, this means that a tree of layout-specialized Configurators gets generated by the compiler.)
*/

// default implementation of the concept
struct Configurator {
	template<typename Configurators,typename DestConfiguration,typename SourceConfiguration> 
	static void Classify(DString name,const StrAttrs &attrs,const SourceConfiguration &source) {
		classifyConfig<Configurators>(name,attrs,source);
	}
	template<typename Configurators,typename SourceConfiguration,typename DestConfiguration> 
	static void Create(DString name,const StrAttrs &attrs,const SourceConfiguration &source,DestConfiguration &dest) {
		createConfiguration<Configurators>(name,attrs,source,dest);
	}
};
struct PopConfigurator {
	template<typename Configurators,typename Layout,typename SourceLayout>
	static bool config(DString name,const StrAttrs &attrs,ChangingGraph<Layout> *world,EnginePair<Layout> engines,SourceLayout *source) {
		typedef typename boost::mpl::front<Configurators>::type FirstConfigurator;
		typedef typename boost::mpl::pop_front<Configurators>::type Rest;
		return FirstConfigurator::template config<Rest>(name,attrs,world,engines,source);
	}
};
struct DoNothingConfigurator {
	template<typename Configurators,typename Layout,typename SourceLayout>
	static bool config(DString name,const StrAttrs &attrs,ChangingGraph<Layout> *world,EnginePair<Layout> engines,SourceLayout *source) {
		return false;
	}
};
template<typename Configurators,typename Layout,typename SourceLayout>
bool configureLayout(DString name,const StrAttrs &attrs,ChangingGraph<Layout> *world,EnginePair<Layout> engines,SourceLayout *source) {
	return boost::mpl::if_<boost::mpl::empty<Configurators>,DoNothingConfigurator,PopConfigurator>::type
		::template config<Configurators>(name,attrs,world,engines,source);
}
template<typename Configurators,typename SourceLayout>
bool configureLayout(DString name,const StrAttrs &attrs,SourceLayout *source) {
	return configureLayout<Configurators,void>(name,attrs,0,EnginePair<void>(),source);
}
struct PassConfigurator {
	template<typename Configurators,typename Layout,typename SourceLayout>
	static bool config(DString name,const StrAttrs &attrs,ChangingGraph<Layout> *world,EnginePair<Layout> engines,SourceLayout *source) {
		return configureLayout<Configurators>(name,attrs,world,engines,source);
	}
};

} // namespace Dynagraph

#endif // Configurator_h
