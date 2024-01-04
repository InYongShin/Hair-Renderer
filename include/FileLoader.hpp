//
//  FileLoader.hpp
//  AR_Framework
//
//  Created by Hyun Joon Shin on 2021/09/06.
//

#ifndef FileLoader_hpp
#define FileLoader_hpp

#include "Model/TriMesh.hpp"
#include "Model/Texture.hpp"
#include <tuple>


namespace AR {
template<typename T>
struct Range {
	T minVal = T(100000);
	T maxVal = T(-100000);
	Range( T mm=T(100000), T mM=T(-100000) ): minVal(mm), maxVal(mM){}
	
	template<typename T2> Range( T2 mm, T2 mM ): minVal(T(mm)), maxVal(T(mM)){}
	Range& operator +=(T v) {
		minVal = glm::min(minVal,v);
		maxVal = glm::max(maxVal,v);
		return *this;
	}
	Range& operator +=(const Range<T>& v) {
		minVal = glm::min(minVal,v.minVal);
		maxVal = glm::max(maxVal,v.maxVal);
		return *this;
	}
};
using Rangef = Range<float>;
using Range3 = Range<vec3>;

using MeshSet = std::vector<AR::TriMesh>;

extern Range3 loadMesh( const std::string& fn, MeshSet& meshSet, TextureLib& texLib );


}
#endif /* FileLoader_hpp */
