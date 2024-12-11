/*
 * Copyright (c) 2024 Le Juez Victor
 * 
 * This software is provided "as-is", without any express or implied warranty. In no event 
 * will the authors be held liable for any damages arising from the use of this software.
 * 
 * Permission is granted to anyone to use this software for any purpose, including commercial 
 * applications, and to alter it and redistribute it freely, subject to the following restrictions:
 * 
 *   1. The origin of this software must not be misrepresented; you must not claim that you 
 *   wrote the original software. If you use this software in a product, an acknowledgment 
 *   in the product documentation would be appreciated but is not required.
 * 
 *   2. Altered source versions must be plainly marked as such, and must not be misrepresented
 *   as being the original software.
 * 
 *   3. This notice may not be removed or altered from any source distribution.
 */

#ifndef R3D_DETAIL_BATCH_MAP_HPP
#define R3D_DETAIL_BATCH_MAP_HPP

#include <vector>
#include <map>

namespace r3d {

/**
 * @brief A template class for managing and sorting batches of draw calls based on a specific key.
 * 
 * The `BatchMap` allows organizing draw calls into batches and sorting them by a key. This can be used to sort 
 * draw calls by materials or lights for efficient rendering, such as during shadow map rendering. It provides 
 * functions for adding, removing, and retrieving batches of draw calls, and for managing the draw calls within 
 * those batches.
 *
 * @tparam Key The type of key used to categorize the draw calls (e.g., material or light).
 * @tparam DrawCall The type of draw call stored in the batch.
 * @tparam Compare The comparison function used for sorting the keys (default is `std::less<Key>`).
 */
template <typename Key, typename DrawCall, typename Compare = std::less<Key>>
class BatchMap
{
public:
    using Batch = std::vector<DrawCall>; ///< Represents a collection of draw calls in a batch.

public:
    /**
     * @brief Default constructor.
     */
    BatchMap() = default;

    /**
     * @brief Checks if a batch for a given key exists in the map.
     * 
     * @param key The key to check for.
     * @return `true` if the batch exists, `false` otherwise.
     */
    bool isBatchExist(Key key) const {
        return mMap.find(key) != mMap.cend();
    }

    /**
     * @brief Adds a new batch with the specified key to the map.
     * 
     * If a batch for the given key already exists, it will not be overwritten.
     * 
     * @param key The key for the new batch.
     */
    void addBatch(Key key) {
        mMap.emplace(key, std::move(std::vector<DrawCall>()));
    }

    /**
     * @brief Erases the batch associated with the specified key from the map.
     * 
     * @param key The key of the batch to remove.
     */
    void eraseBatch(Key key) {
        auto it = mMap.find(key);
        if (it != mMap.end()) {
            mMap.erase(it);
        }
    }

    /**
     * @brief Retrieves the batch of draw calls for the given key.
     * 
     * @param key The key of the batch to retrieve.
     * @return A reference to the batch of draw calls.
     */
    Batch& getBatch(Key key) {
        return mMap.at(key);
    }

    /**
     * @brief Retrieves the batch of draw calls for the given key (const version).
     * 
     * @param key The key of the batch to retrieve.
     * @return A const reference to the batch of draw calls.
     */
    const Batch& getBatch(Key key) const {
        return mMap.at(key);
    }

    /**
     * @brief Pushes a draw call into the batch associated with the specified key.
     * 
     * If the batch does not exist, it will be created first.
     * 
     * @param key The key of the batch.
     * @param drawCall The draw call to add.
     */
    void pushDrawCall(Key key, const DrawCall& drawCall) {
        getBatch(key).push_back(drawCall);
    }

    /**
     * @brief Provides an iterator to the beginning of the map.
     * 
     * @return An iterator to the beginning of the map.
     */
    auto begin() noexcept {
        return mMap.begin();
    }

    /**
     * @brief Provides an iterator to the end of the map.
     * 
     * @return An iterator to the end of the map.
     */
    auto end() noexcept {
        return mMap.end();
    }

    /**
     * @brief Provides a const iterator to the beginning of the map.
     * 
     * @return A const iterator to the beginning of the map.
     */
    auto begin() const noexcept {
        return mMap.cbegin();
    }

    /**
     * @brief Provides a const iterator to the end of the map.
     * 
     * @return A const iterator to the end of the map.
     */
    auto end() const noexcept {
        return mMap.cend();
    }

private:
    std::map<Key, Batch, Compare> mMap{}; ///< The map storing batches, sorted by the key.
};

} // namespace r3d

#endif // R3D_DETAIL_BATCH_MAP_HPP
