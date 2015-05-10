/*
 * Copyright 2015 Cloudius Systems
 */

#define BOOST_TEST_DYN_LINK

#include <boost/test/unit_test.hpp>
#include <stdlib.h>
#include <iostream>
#include <unordered_map>

#include "tests/test-utils.hh"
#include "core/future-util.hh"
#include "utils/UUID_gen.hh"
#include "db/serializer.hh"
#include "database.hh"

using namespace db;

template<typename T>
static T serialize_deserialize(const T& t) {
    db::serializer<T> sz(t);
    bytes tmp(bytes::initialized_later(), sz.size());
    data_output out(tmp);
    sz(out);
    data_input in(tmp);
    T n = db::serializer<T>::read(in);
    if (in.avail() > 0) {
        throw std::runtime_error("Did not consume all bytes");
    }
    return std::move(n);
}

SEASTAR_TEST_CASE(test_sstring){
    std::initializer_list<sstring> values = {
            "kow", "abcdefghIJKL78&%\"\r", "\xff\xff"
    };
    for (auto& v : values) {
        auto nv = serialize_deserialize(v);
        BOOST_CHECK_EQUAL(nv, v);
    }
    return make_ready_future<>();
}

namespace utils {
inline std::ostream& operator<<(std::ostream& os, const utils::UUID& uuid) {
    return os << uuid.to_bytes();
}
}

SEASTAR_TEST_CASE(test_uuid){
    std::initializer_list<utils::UUID> values = {
            utils::UUID_gen::get_time_UUID(), utils::make_random_uuid()
    };
    for (auto& v : values) {
        auto nv = serialize_deserialize(v);
        BOOST_CHECK_EQUAL(nv, v);
    }
    return make_ready_future<>();
}

SEASTAR_TEST_CASE(test_tombstone){
    std::initializer_list<tombstone> values = {
            //tombstone(),
            tombstone(12, gc_clock::now())
    };
    for (auto& v : values) {
        auto nv = serialize_deserialize(v);
        BOOST_CHECK_EQUAL(nv, v);
    }
    return make_ready_future<>();
}

template<typename... Args>
inline std::ostream& operator<<(std::ostream& os, const std::map<Args...>& v) {
    os << "{ ";
    int n = 0;
    for (auto& p : v) {
        if (n > 0) {
            os << ", ";
        }
        os << "{ " << p.first << ", " << p.second << " }";
        ++n;
    }
    return os << " }";
}
