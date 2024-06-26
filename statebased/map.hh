#ifndef CRDTS_MAP_HH
#define CRDTS_MAP_HH

#include <unordered_map>
#include "orset.hh"
#include "lwwregister.hh"
#include <cpprest/json.h>

using namespace web;

/// Map is a convergent map with the ``add wins'' policy for keys and the ``last writer wins'' policy for values.
/// Map maintains keys in an ORSet to implement the former policy. By keeping each value in a LWWRegister, Map
/// implements the latter policy.
template<typename KeyType, typename ValueType>
class Map {
private:
    ORSet<KeyType> _keys;
    std::unordered_map<KeyType, LWWRegister<ValueType>> _registers;

public:
    /// Creates a map object with a given replica id
    /// \param replica_id the given replica id
    explicit Map(uint64_t replica_id) : _keys(replica_id) { }

    Map(const json::value& json_value): _keys(0) {
        if (!json_value.has_field(U("_keys")) || !json_value.has_field(U("_registers"))) {
            throw std::invalid_argument("Invalid JSON format for Map");
        }

        // Deserialize _keys
        _keys = ORSet<KeyType>(json_value.at(U("_keys")));

        // Deserialize _registers
        const auto& registers_json = json_value.at(U("_registers"));
        if (!registers_json.is_null()) {
            for (const auto& pair : registers_json.as_object()) {
                KeyType key = pair.first; // Convert key to KeyType
                LWWRegister<ValueType> lww_register(pair.second); // Deserialize LWWRegister object
                _registers[key] = lww_register; // Add LWWRegister to _registers
            }
        }else{
            _registers.clear();
        }
    }

    /// Puts a given key and value pair to the map
    /// \param key the given key
    /// \param val the given value
    void put(const KeyType& key, const ValueType& val)  {
        _keys.add(key);
        if (_registers.count(key) == 0) {
            // initialize a register for the first time
            _registers[key].replica_id(_keys.replica_id());
        }//if

        _registers[key].assign(val);
    }

    /// Gets the value of a given key
    /// \param key the given key
    /// \return the value
    ValueType get(const KeyType& key) {
        if (!this->contains(key)) {
            return ValueType();
        }//if

        return _registers[key].value();
    }

    /// Removes a given key from the map
    /// \param key the given key
    void remove(const KeyType& key) {
        _keys.remove(key);
        _registers.erase(key);
    }

    /// Merges a given map with the local map
    /// \param map the given map
    void merge(const Map<KeyType, ValueType>& map) {
        ORSet<KeyType> keys_copy = this->_keys;

        // Merge keys
        this->_keys.merge(map._keys);

        // Remove keys deleted in merging keys (above)
        for (const auto& k: keys_copy.elements()) {
            if (!this->_keys.contains(k)) {
                this->_registers.erase(k);
            }//if
        }//for

        // Merge registers associated with remaining keys
        for (const auto& remote_reg: map._registers) {
            auto local_reg = this->_registers.find(remote_reg.first);
            if (local_reg != this->_registers.end()) {
                // The associated register locally exists, merge it with the remote register
                local_reg->second.merge(remote_reg.second);
            }//if
            else if (this->_keys.contains(remote_reg.first)) {
                // The register associate to the remote key does not locally exist, add the associated register.
                // We initialize the local register with the remote value, then merge two registers.
                this->_registers[remote_reg.first].replica_id(this->replica_id());
                this->_registers[remote_reg.first].assign(remote_reg.second.value());
                this->_registers[remote_reg.first].merge(remote_reg.second);
            }//else if
        }//for
    }

    /// Checks the existence of a given key
    /// \param key the given key
    /// \return true if the key exists, otherwise false
    bool contains(const KeyType& key) {
        return _keys.contains(key);
    }

    /// Gets the number of key value pairs in the map
    /// \return the number of key value pairs
    size_t size() {
        return _keys.size();
    }

    /// Gets the replica id
    /// \return the replica id
    uint64_t replica_id() {
        return _keys.replica_id();
    }

    /// Gets the key value pairs stored in the map
    /// \return all key value pairs
    std::unordered_map<KeyType, ValueType> key_value_pairs() {
        std::unordered_map<KeyType, ValueType> res;
        for (const auto& kv: _registers)
            res[kv.first] = kv.second.value();

        return res;
    }

    json::value to_json() const {
        json::value serialized_map;
        // // Serialize replica id
        serialized_map[U("_keys")] = _keys.to_json();
        
        // Serialize _registers
        json::value registers_json;
        for (const auto& pair : _registers) {
            // Serialize LWWRegister for each key
            registers_json[U(pair.first)] = pair.second.to_json();
        }
        serialized_map[U("_registers")] = registers_json;

     return serialized_map;
    }

    void print() const {
        std::cout << "Keys:" << std::endl;
        _keys.print(); // Assuming ORSet has a print function

        std::cout << "Registers:" << std::endl;
        for (const auto& pair : _registers) {
            std::cout << "Key: " << pair.first << ", Value: ";
            pair.second.print(); // Assuming LWWRegister has a print function
        }
    }

};

#endif //CRDTS_MAP_HH
