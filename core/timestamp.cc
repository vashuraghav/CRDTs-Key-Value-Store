#include "timestamp.hh"

Timestamp::Timestamp(){

}

Timestamp::Timestamp(const json::value& json_value) {
        // Check if JSON value has the required fields
        if (!json_value.has_field(U("sequence_number")) ||
            !json_value.has_field(U("uid")) ||
            !json_value.has_field(U("replica_id"))) {
            throw std::invalid_argument("Invalid JSON format for Timestamp");
        }

        // Parse sequence number
        _seq_number = json_value.at(U("sequence_number")).as_integer();

        // Parse uid
        _uid = json_value.at(U("uid")).as_integer();

        // Parse replica id
        _replica_id = json_value.at(U("replica_id")).as_integer();
}

void Timestamp::replica_id(uint64_t replica_id) {
    this->_replica_id = replica_id;
    this->_uid = replica_id;
}

bool operator < (const Timestamp &t1, const Timestamp &t2) {
    // The comparison concatenates sequence numbers with uids to avoid ties.
    return t1._seq_number < t2._seq_number or
           (t1._seq_number == t2._seq_number and t1._uid < t2._uid);
}

bool operator == (const Timestamp& t1, const Timestamp& t2) {
    return t1._seq_number == t2._seq_number and t1._uid == t2._uid;
}

bool operator != (const Timestamp& t1, const Timestamp& t2) {
    return !(t1 == t2);
}

void Timestamp::update() {
    _seq_number ++;
    // TODO: replace replica_id with an external call that returns the replica's identifier.
    _uid = _replica_id;
}

uint64_t Timestamp::replica_id() const {
    return _replica_id;
}

void Timestamp::copy(const Timestamp &t) {
    this->_uid = t._uid;
    this->_seq_number = t._seq_number;
}

uint64_t Timestamp::sequence_number() {
    return _seq_number;
}

 // Serialization method to convert Timestamp object to JSON
json::value Timestamp::to_json() const {
    json::value json_value;

    json_value[U("sequence_number")] = json::value::number(_seq_number);
    json_value[U("uid")] = json::value::number(_uid);
    json_value[U("replica_id")] = json::value::number(_replica_id);

    return json_value;
}

void Timestamp::print() const {
    std::cout << "Sequence Number: " << _seq_number << std::endl;
    std::cout << "UID: " << _uid << std::endl;
    std::cout << "Replica ID: " << _replica_id << std::endl;
}