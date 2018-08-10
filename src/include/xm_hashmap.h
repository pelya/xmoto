#ifndef __XMHASHMAP_H__
#define __XMHASHMAP_H__

#ifdef __ANDROID__
#include <unordered_map>
namespace HashNamespace = std;
#else
#include <tr1/unordered_map>
namespace HashNamespace = std::tr1;
#endif
#endif
