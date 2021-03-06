/*****************************************************************************************
 *                                                                                       *
 * OpenSpace                                                                             *
 *                                                                                       *
 * Copyright (c) 2014-2016                                                               *
 *                                                                                       *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this  *
 * software and associated documentation files (the "Software"), to deal in the Software *
 * without restriction, including without limitation the rights to use, copy, modify,    *
 * merge, publish, distribute, sublicense, and/or sell copies of the Software, and to    *
 * permit persons to whom the Software is furnished to do so, subject to the following   *
 * conditions:                                                                           *
 *                                                                                       *
 * The above copyright notice and this permission notice shall be included in all copies *
 * or substantial portions of the Software.                                              *
 *                                                                                       *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,   *
 * INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A         *
 * PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT    *
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF  *
 * CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE  *
 * OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                                         *
 ****************************************************************************************/

#ifndef __LRU_CACHE_H__
#define __LRU_CACHE_H__

#include <glm/glm.hpp>
#include <memory>
#include <ostream>
#include <unordered_map>
#include <list>



namespace openspace {

    // Templated class implementing a Least-Recently-Used Cache
    template<typename KeyType, typename ValueType>
    class LRUCache {
    public:
        LRUCache(size_t size);
        ~LRUCache();


        void put(const KeyType& key, const ValueType& value);
        void clear();
        bool exist(const KeyType& key) const;
        ValueType get(const KeyType& key);
        size_t size() const;


    private:
        void clean();


    // Member varialbes
    private:
        
        std::list<std::pair<KeyType, ValueType>> _itemList;
        std::unordered_map<KeyType, decltype(_itemList.begin())> _itemMap;
        size_t _cacheSize;

    };


} // namespace openspace


#include <modules/globebrowsing/other/lrucache.inl>

#endif // __LRU_CACHE_H__
