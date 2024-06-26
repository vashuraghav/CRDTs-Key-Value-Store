#ifndef CRDTS_ORSET_HH
#define CRDTS_ORSET_HH

#include <unordered_map>
#include <unordered_set>
#include "../core/timestamp.hh"
#include <cpprest/json.h>

using namespace web;
/// ORSet implements an "observed remove set" based on "optimized observed removed set" [1].
/// [1] Bieniusa A, Zawirski M, Preguiça N, Shapiro M, Baquero C, Balegas V, Duarte S. (2012).
/// An optimized conflict-free replicated set, arXiv preprint arXiv:1210.3368.
template<typename ValueType>
class ORSet {
private:
    std::unordered_map<ValueType, std::unordered_map<uint64_t, Timestamp>> _elements;
    std::unordered_map<uint64_t, Timestamp> _versions;
    uint64_t _replica_id;

    /// Merges the local version vector with a given set
    /// \param remote_set the given set
    void _merge_versions(const ORSet<ValueType>& remote_set) {
        for (const auto& v: remote_set._versions) {
            auto local_version = this->_versions.find(v.first);

            if (local_version != this->_versions.end()) {
                // Local replica has observed the version of this remote replica
                local_version->second = std::max(local_version->second, v.second);
            }//if
            else {
                // Local replica has not observed the version of this remote replica
                this->_versions[v.first] = v.second;
            }//else
        }//for
    }

    /// Applies remove operations from a given remote set, only operations that are more recent than
    /// observed add operations are effective.
    /// \param remote_set the remote set object
    void _apply_remote_removes(const ORSet<ValueType>& remote_set) {
        // Remove elements that have been removed remotely.
        // Add wins policy is applied for concurrent add and remove operations
        for (auto local_elem = this->_elements.begin(); local_elem != this->_elements.end(); /* no increment here */) {
            // An element has been removed remotely if it does not exist in the remote set.
            if (remote_set._elements.count(local_elem->first) == 0) {
                // Applying the add wins policy: find a concurrent or newer add
                bool newer_remote_remove = true;
                for (const auto& local_add: local_elem->second) {
                    auto remote_remove = remote_set._versions.find(local_add.first);

                    if (remote_remove == remote_set._versions.end() or remote_remove->second < local_add.second) {
                        // A concurrent or newer local add exists, so the remote remove is not newer
                        // based on ``add wins'' policy
                        newer_remote_remove = false;
                        break;
                    }//if
                }//for

                if (newer_remote_remove) {
                    // Find an evidence that there has been a remote remove by examining the versions
                    // of replicas observed in the remote set
                    newer_remote_remove = false;
                    for (const auto &remote_remove: remote_set._versions) {
                        auto local_add = local_elem->second.find(remote_remove.first);
                        // An evidence would be that a timestamp associated to a replica id
                        // has not been seen locally or this timestamp is newer than an locally observed timestamp
                        if (local_add == local_elem->second.end() or local_add->second < remote_remove.second) {
                            // An evidence is found
                            newer_remote_remove = true;
                            break;
                        }//if
                    }//for
                }//if

                if (newer_remote_remove) {
                    // Remove the element that has been remotely removed, and move to next local element
                    local_elem = this->_elements.erase(local_elem);
                }//if
                else {
                    ++local_elem;
                }//else
            }//if
            else {
                ++local_elem;
            }//else
        }//for
    }

    /// Applies add operations from a given remote set with add-wins policy
    /// \param remote_set the given remote set
    void _apply_remote_adds(const ORSet<ValueType>& remote_set) {
        // Add remote elements locally. A remote element is not added if the local replica has received and then
        // removed the element earlier.
        for (const auto& remote_elem: remote_set._elements) {
            auto local_elem = this->_elements.find(remote_elem.first);

            // Check if the remote_set element locally exists
            if (local_elem != this->_elements.end()) {
                // The element already exists, update the timestamps
                for (auto remote_timestamp: remote_elem.second) {
                    // Update a local timestamp with a remote timestamp that is more recent
                    if (this->_versions.count(remote_timestamp.first) == 0 or
                        this->_versions[remote_timestamp.first] < remote_timestamp.second) {
                        local_elem->second[remote_timestamp.first] = remote_timestamp.second;
                    }//if
                }//for
            }//if
            else {
                // The element does not exist locally
                bool more_recent = false;
                for (auto remote_timestamp: remote_elem.second) {
                    // Check if a remote timestamp is newer than the local timestamp
                    // A single newer remote timestamp is sufficient due to the ``add wins'' policy
                    if (this->_versions.count(remote_timestamp.first) == 0 or
                        this->_versions[remote_timestamp.first] < remote_timestamp.second) {
                        more_recent = true;
                        break;
                    }//if
                }//for

                // Add a new element that was added remotely
                if (more_recent) {
                    this->_elements[remote_elem.first] = remote_elem.second;
                }//if
            }//else
        }//for
    }

public:
    /// Creates an ORSet object at a replica identified with a given id
    /// \param replica_id the given replica id
    explicit ORSet(uint64_t replica_id) {
        this->_replica_id = replica_id;
        // Initialize the local version
        this->_versions[_replica_id].replica_id(this->_replica_id);
    }

    ORSet(const json::value& json_value) {
        if (!json_value.has_field(U("elements")) ||
            !json_value.has_field(U("versions")) ||
            !json_value.has_field(U("replica_id"))) {
            throw std::invalid_argument("Invalid JSON format for ORSet");
        }

        // Deserialize _elements
        const auto& elements_json = json_value.at(U("elements"));
        if (!elements_json.is_null()) {
            for (const auto& elem : elements_json.as_object()) {
                ValueType key = elem.first; // Remove extra quotes from key
                const auto& timestamp_map_json = elem.second;
                for (const auto& timestamp_pair : timestamp_map_json.as_object()) {
                    uint64_t timestamp_key = std::stoull(timestamp_pair.first); // Convert timestamp key to uint64_t
                    Timestamp timestamp(timestamp_pair.second); // Deserialize Timestamp object
                    _elements[key][timestamp_key] = timestamp; // Add element to _elements
                }
            }
        }else{
            _elements.clear();
        }

        // Deserialize _versions
        const auto& versions_json = json_value.at(U("versions"));
        for (const auto& version_pair : versions_json.as_object()) {
            uint64_t version_key = std::stoull(version_pair.first); // Convert version key to uint64_t
            Timestamp timestamp(version_pair.second); // Deserialize Timestamp object
            _versions[version_key] = timestamp; // Add version to _versions
        }

        // Deserialize _replica_id
        _replica_id = json_value.at(U("replica_id")).as_integer(); // Get replica id
    }

    /// Adds a given element to the set
    /// \param e the given element
    void add(const ValueType& e) {
        _versions[_replica_id].update();
        _elements[e][_replica_id] = _versions[_replica_id];
    }
    void add(const ValueType&& e) { add(e); }

    /// Removes a given element from the set
    /// \param e the given element
    /// \return true if the given element existed and was removed, otherwise false
    bool remove(const ValueType& e) {
        return _elements.erase(e) > 0;
    }
    bool remove(const ValueType&& e) { return remove(e); }

    /// Check if the given element exists in the set
    /// \param e the given element
    /// \return true if the given element exists, otherwise false
    bool contains(const ValueType& e) {
        return _elements.count(e) > 0;
    }
    bool contains(const ValueType&& e) { return contains(e); }

    /// Merges the local set with a given remote set
    /// \param remote_set the given remote set
    void merge(const ORSet<ValueType>& remote_set) {
        // apply remote remove operations
        _apply_remote_removes(remote_set);

        // apply remote add operations
        _apply_remote_adds(remote_set);

        // merge versions
        _merge_versions(remote_set);
    }

    /// Gets elements stored in the local replica
    /// \return the set of elements
    std::unordered_set<ValueType> elements() const {
        std::unordered_set<ValueType> result;

        for (const auto& elem: _elements)
            result.insert(elem.first);

        return result;
    }

    /// Gets the local replica id
    /// \return the local replica id
    uint64_t replica_id() {
        return _replica_id;
    }

    /// Gets the number of elements in the set
    /// \return the number of elements
    size_t size() {
        return _elements.size();
    }

     // Serialization method to convert orset object to JSON
    json::value to_json() const {
        json::value json_value;
        // Serialize _elements
        json::value elements_json;
        for (const auto& elem : _elements) {
            json::value timestamp_map_json;
            for (const auto& timestamp_pair : elem.second) {
                timestamp_map_json[json::value::number(timestamp_pair.first).serialize()] = timestamp_pair.second.to_json();
            }
            elements_json[U(elem.first)] = timestamp_map_json;
        }

        // Serialize _versions
        json::value versions_json;
        for (const auto& version : _versions) {
            versions_json[json::value::number(version.first).serialize()] = version.second.to_json();
        }

        json_value[U("elements")] = elements_json;
        json_value[U("versions")] = versions_json;
        json_value[U("replica_id")] = json::value::number(_replica_id);

        return json_value;
    }

    void print() const {
        // Print elements
        std::cout << "Elements:" << std::endl;
        for (const auto& elem : _elements) {
            std::cout << "  " << elem.first << ":" << std::endl;
            for (const auto& timestamp : elem.second) {
                std::cout << "    " << timestamp.first << ": ";
                timestamp.second.print(); // Assuming the Timestamp class has a print function
                std::cout << std::endl;
            }
        }

        // Print versions
        std::cout << "Versions:" << std::endl;
        for (const auto& version : _versions) {
            std::cout << "  " << version.first << ": ";
            version.second.print(); // Assuming the Timestamp class has a print function
            std::cout << std::endl;
        }

        // Print replica ID
        std::cout << "Replica ID: " << _replica_id << std::endl;
    }

};

#endif //CRDTS_ORSET_HH
