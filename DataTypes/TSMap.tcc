/**
 * \file TSMap.tcc
 **/

#pragma once

#include <condition_variable>
#include <iostream>
#include <map>
#include <mutex>
#include <vector>
#include "JsonBox.h"

#include "shared_mutex.h"
#include <atomic>
#include <chrono>
#include <future>
#include <iostream>
#include <limits>
#include <random>
#include <thread>
#include <string>
#include "assert.h"

namespace atl
{

    /*
     * \brief Threadsafe wrapper for standard library ordered map class
     */
    template<typename Key, typename Value> class TSMap
    {
        protected:
            std::map<Key, Value> m_map;     //!< Map of objects
            mutable shared_mutex m_mutex;
    
        public:
            // read functions
            std::pair<Value, bool>  find(Key k) const;
            std::pair<Value, bool>  lower_bound(Key k) const;
            std::pair<std::pair<Key,Value>, bool> lower_bound_key(Key k) const;
            size_t                  size() const;
            bool                    empty() const;
            std::vector<Key>        getKeyList() const;
    
            // write functions
            bool                    emplace(Key k, Value v, bool force = false);
            template<typename... Args>
            bool                    createInPlace(Key k, Args... args);
            std::pair<Value,bool>   replace(Key k, Value v, bool force = true);
            bool                    erase(Key k, std::function<bool(Key,Value&)> f = nullptr);
            std::pair<Value, bool>  remove(Key k);

            bool                    perform(Key k, std::function<bool(Key,Value&)> f = nullptr);
            bool                    perform_ro(Key k, std::function<bool(Key,const Value&)> f = nullptr) const;
            void                    clear();

            // function iterators
            size_t for_each_ro(std::function<bool(Key k, const Value& v)> f) const;
            size_t for_each(std::function<bool(Key k, Value& v)> f);
            size_t delete_if(std::function<bool(Key k, Value& v)> f);
    };


    ////////////////////////////////////////
    //            READ METHODS            //
    ////////////////////////////////////////
    
    /*
     * \brief Retrieves a value from the map
     * \param [in] k The key to query the map with
     *
     * \return The value correspoding to Key k
     */
    template<typename Key, typename Value> std::pair<Value, bool> TSMap<Key, Value>::
            find(Key k) const
    {
        bool rc = false;
        Value v{};
        atl::shared_lock lock(m_mutex);
        auto it = m_map.find(k);
        
        if (it != m_map.end()){
            rc = true;
            v = it->second;
        }
        return std::make_pair(v, rc);
    }
    
    /*
     * \brief Retrieves the first value with a key not less than the given k
     * \param [in] k The key to query the map with
     *
     * \return The value with the smallest key greater than or equal t
     * correspoding to Key k
     */
    template<typename Key, typename Value> std::pair<Value, bool> TSMap<Key, Value>::
            lower_bound(Key k) const
    {
        bool rc = false;
        Value v{};
        atl::shared_lock lock(m_mutex);
        auto it = m_map.lower_bound(k);

        if (it != m_map.end()){
            rc = true;
            v = it->second;
        }
        return std::make_pair(v, rc);
    }
    
    /*
     * \brief Retrieves the first value with a key not less than the given k
     * \param [in] k The key to query the map with
     *
     * \return The value with the smallest key greater than or equal t
     * correspoding to Key k; also returns the Key of this Value
     */
    template<typename Key, typename Value> 
            std::pair<std::pair<Key, Value>, bool> TSMap<Key, Value>::
            lower_bound_key(Key k) const
    {
        bool rc = false;
        Key returnKey{};
        Value v{};
        atl::shared_lock lock(m_mutex);
        auto it = m_map.lower_bound(k);

        if (it != m_map.end()){
            rc = true;
            returnKey = it->first;
            v = it->second;
        }
        return std::make_pair(std::make_pair(returnKey, v), rc);
    }
    
    /*
     * \brief returns the number of entries in the map
     */
    template<typename Key, typename Value> size_t TSMap<Key, Value>::
            size() const
    {
        atl::shared_lock lock(m_mutex);
        size_t s = m_map.size();
        return s;
    }

    /*
     * \brief checks if the map is empty
     */
    template<typename Key, typename Value> bool TSMap<Key, Value>::
            empty() const
    {
        atl::shared_lock lock(m_mutex);
        bool rc = m_map.empty();
        return rc;
    }

    template<typename Key, typename Value> std::vector<Key> TSMap<Key, Value>::
            getKeyList() const
    {
        std::vector<Key> keyList;
        atl::shared_lock lock(m_mutex);

        for (auto it = m_map.cbegin(); it != m_map.cend(); it++) {
            keyList.push_back(it->first);
        }
        return keyList;
    }


    ////////////////////////////////////////
    //           WRITE METHODS            //
    ////////////////////////////////////////
    
    /*
     * \brief Add a key-value pair to the map
     * \param [in] k The key associated with Value v
     * \param [in] v The value to insert into the map
     *
     * return true if no element previously existed, false if one did
     */
    template<typename Key, typename Value> bool TSMap<Key, Value>::
            emplace(Key k, Value v, bool force)
    {
        std::unique_lock<atl::shared_mutex> lock(m_mutex);

        if (!force && m_map.find(k) != m_map.end()) {
            return false;
        }
        m_map.erase(k);

        return m_map.emplace(k, v).second;
    }

    /*
     * \brief Create a value in place in the map
     * \param [in] k The key associated with Value v
     * \param [in] args the arguments to the constructor of the Value
     *
     * return true if the value was successfully created
     */
    template<typename Key, typename Value>
    template<typename... Args> bool TSMap<Key,Value>::
        createInPlace(Key k, Args... args)
    {
        std::unique_lock<atl::shared_mutex> lock(m_mutex);
        auto ret = m_map.emplace(std::piecewise_construct,
                                  std::forward_as_tuple(k),
                                  std::forward_as_tuple(args...));
        return ret.second;
    }

    /*
     * \brief Add a key-value pair to the map
     * \param [in] k The key associated with Value v
     * \param [in] v The value to insert into the map
     *
     * \return pair containing value previously in the specified index
     * and bool containing true if no element previously existed, false if one
     * did previously exist
     *
     * note: if nothing was in this location previously, the return Value will
     * be the value that was passed in. 
     */
    template<typename Key, typename Value> std::pair<Value,bool> TSMap<Key, Value>::
            replace(Key k, Value v, bool force)
    {
        std::unique_lock<atl::shared_mutex> lock(m_mutex);
        Value newVal = v;
        auto it = m_map.emplace(k, v);

        if (!it.second) {
            newVal = it.first->second;
            if (force) {
                m_map[k] = v;
            }
        }
        return std::pair<Value,bool>(newVal, it.second);
    }

    /*
     * \brief Erases an entry from the map
     * \param [in] k The key of the entry to erase
     * \param [in] f The function to perform on the key-value pair.
     * If this function returns false, the entry will not be erased
     *
     * \return true if the element was erased, false otherwise
     */
    template<typename Key, typename Value> bool TSMap<Key, Value>::
            erase(Key k, std::function<bool(Key,Value&)> f)
    {
        std::unique_lock<atl::shared_mutex> lock(m_mutex);
        auto it = m_map.find(k);

        if (it == m_map.end()){
            return false;
        }

        if (!f || f(k, it->second)) {
            size_t rc = m_map.erase(k);
            return rc == 1;
        } else {
            return false;
        }
    }

/**
 * \brief Returns the value associated with a key and erases it from the map;
 *        If the key is not found, nothing is erased
 * \param[in] k The key to find, return, and remove
 * \return A pair of the value and a bool to indicate success
 **/
    template<typename Key, typename Value> std::pair<Value, bool> TSMap<Key, Value>::
            remove(Key k)
    {
        std::unique_lock<atl::shared_mutex> lock(m_mutex);
        bool rc = false;
        Value v{};
        auto it = m_map.find(k);

        if (it != m_map.end()) {
            rc = true;
            v = it->second;
            m_map.erase(k);
        }
        return std::make_pair(v, rc);
    }
    
    /*
     * \brief performs the given function on the key value pair specified
     * \param [in] k The key of the entry to operate on
     * \param [in] f The function to perform
     *
     * \return The value returned by the fucntion
     */
    template<typename Key, typename Value> bool TSMap<Key, Value>::
    perform(Key k, std::function<bool(Key,Value&)> f)
    {
        std::unique_lock<atl::shared_mutex> lock(m_mutex);
        auto it = m_map.find(k);

        if (it == m_map.end() || !f) {
            return false;
        }

        return f(k, it->second);
    }

    /*
     * \brief performs the given function on the key value pair specified (read-only)
     * \param [in] k The key of the entry to operate on
     * \param [in] f The (read-only) function to perform
     *
     * \return The value returned by the fucntion
     */
    template<typename Key, typename Value> bool TSMap<Key, Value>::
    perform_ro(Key k, std::function<bool(Key,const Value&)> f) const
    {
        atl::shared_lock lock(m_mutex);
        auto it = m_map.find( k );

        if (it == m_map.end() || !f) {
            return false;
        }

        return f(k, it->second);
    }

    /*
     * \brief Clears all entries from the map
     */
    template<typename Key, typename Value> void TSMap<Key, Value>::
            clear()
    {
        std::unique_lock<atl::shared_mutex> lock(m_mutex);
        m_map.clear();
    }


    ////////////////////////////////////////
    //         FUNCTION ITERATORS         //
    ////////////////////////////////////////

    /*
     * \brief Takes a function pointer and applies it to all
     *        elements in the map
     * \param [in] f The function to apply to each element in the map;
     *               should return true on success, false on failure
     * \return number of successful returns from f
     */
    template<typename Key, typename Value> size_t TSMap<Key, Value>::
            for_each_ro(std::function<bool(Key k, const Value& v)> f) const
    {
        size_t numSuccess = 0;
        atl::shared_lock lock(m_mutex);

        for (auto it = m_map.cbegin(); it != m_map.cend(); it++) {
            if (f(it->first, it->second)){
                numSuccess++;
            }
        }
        return numSuccess;
    }

    /*
     * \brief Takes a function pointer and applies it to all
     *        elements in the map
     * \param [in] f The function to apply to each element in the map;
     *               should return true on success, false on failure
     * \return number of successful returns from f
     */
    template<typename Key, typename Value> size_t TSMap<Key, Value>::
            for_each(std::function<bool(Key k, Value& v)> f)
    {
        size_t numSuccess = 0;
        std::unique_lock<atl::shared_mutex> lock(m_mutex);

        for (auto it = m_map.begin(); it != m_map.end(); it++) {
            if (f(it->first, it->second)){
                numSuccess++;
            }
        }
        return numSuccess;
    }

    /*
     * \brief Iterates through the map and deletes each element that
     *        meets some condition
     * \param [in] f Function that takes Key, Value pair and returns 
     *        a boolean, applied to each element; If the function 
     *        returns false on an element, the element is deleted
     * \return the number of entries deleted
     */
    template<typename Key, typename Value> size_t TSMap<Key, Value>::
            delete_if(std::function<bool(Key k, Value& v)> f)
    {
        size_t numErased = 0;
        std::unique_lock<atl::shared_mutex> lock(m_mutex);

        for (auto it = m_map.begin(); it != m_map.end(); it++) {
            if (f(it->first, it->second)) {
                it = m_map.erase(it);
                numErased++;
            } else {
                it++;
            }
        }
        return numErased;
    }
    
}
