#ifndef LIBDNF_UTILS_SACK_QUERY_HPP
#define LIBDNF_UTILS_SACK_QUERY_HPP


#include "QueryCmp.hpp"
#include "Set.hpp"
#include "match_int64.hpp"
#include "match_string.hpp"

#include <cstdint>
#include <map>
#include <set>
#include <stdexcept>
#include <string>
#include <vector>


namespace libdnf::utils::sack {


/// Query is a Set with filtering capabilities.
// TODO(dmach): consider merging Query and Set? or keep them separate for better user understanding?
template <typename T>
class Query : public Set<T> {
public:

    using FilterFunctionBool = bool(const T & obj);
    using FilterFunctionCString = char * (const T & obj);
    using FilterFunctionInt64 = int64_t(const T & obj);
    using FilterFunctionString = std::string(const T & obj);
    using FilterFunctionVectorInt64 = std::vector<int64_t>(const T & obj);
    using FilterFunctionVectorString = std::vector<std::string>(const T & obj);

    std::size_t filter(Query<T>::FilterFunctionString & getter, QueryCmp cmp, const std::string & pattern);
    std::size_t filter(Query<T>::FilterFunctionVectorString & getter, QueryCmp cmp, const std::string & pattern);
    std::size_t filter(Query<T>::FilterFunctionString & getter, QueryCmp cmp, const std::vector<std::string> & patterns);
    std::size_t filter(Query<T>::FilterFunctionVectorString & getter, QueryCmp cmp, const std::vector<std::string> & patterns);

    std::size_t filter(Query<T>::FilterFunctionInt64 & getter, QueryCmp cmp, int64_t pattern);
    std::size_t filter(Query<T>::FilterFunctionVectorInt64 & getter, QueryCmp cmp, int64_t pattern);
    std::size_t filter(Query<T>::FilterFunctionInt64 & getter, QueryCmp cmp, const std::vector<int64_t> & patterns);
    std::size_t filter(Query<T>::FilterFunctionVectorInt64 & getter, QueryCmp cmp, const std::vector<int64_t> & patterns);

    std::size_t filter(Query<T>::FilterFunctionBool & getter, QueryCmp cmp, bool pattern);


    std::size_t filter(Query<T>::FilterFunctionCString & getter, QueryCmp cmp, const std::string & pattern);

    /// Get a single object. Raise an exception if none or multiple objects match the query.
    const T & get() const {
        if (get_data().size() == 1) {
            return *get_data().begin();
        }
        throw std::runtime_error("Query must contain exactly one object.");
    }

    /// List all objects matching the query.
    const std::set<T> & list() const noexcept { return get_data(); }

    // operators; OR at least
    // copy()
    using Set<T>::get_data;
};


template <typename T>
inline std::size_t Query<T>::filter(Query<T>::FilterFunctionString & getter, QueryCmp cmp, const std::string & pattern) {
    std::size_t filtered = 0;

    for (auto it = get_data().begin(); it != get_data().end(); ) {
        auto value = getter(*it);
        if (match_string(value, cmp, pattern)) {
            ++it;
        } else {
            it = get_data().erase(it);
            ++filtered;
        }
    }
    return filtered;
}


template <typename T>
inline std::size_t Query<T>::filter(Query<T>::FilterFunctionVectorString & getter, QueryCmp cmp, const std::string & pattern) {
    std::size_t filtered = 0;

    for (auto it = get_data().begin(); it != get_data().end(); ) {
        auto values = getter(*it);
        if (match_string(values, cmp, pattern)) {
            ++it;
        } else {
            it = get_data().erase(it);
            ++filtered;
        }
    }
    return filtered;
}

template <typename T>
inline std::size_t Query<T>::filter(Query<T>::FilterFunctionString & getter, QueryCmp cmp, const std::vector<std::string> & patterns) {
    std::size_t filtered = 0;
    for (auto it = get_data().begin(); it != get_data().end(); ) {
        auto value = getter(*it);
        if (match_string(value, cmp, patterns)) {
            ++it;
        } else {
            it = get_data().erase(it);
            ++filtered;
        }
    }
    return filtered;
}

template <typename T>
inline std::size_t Query<T>::filter(Query<T>::FilterFunctionVectorString & getter, QueryCmp cmp, const std::vector<std::string> & patterns) {
    std::size_t filtered = 0;
    for (auto it = get_data().begin(); it != get_data().end(); ) {
        auto values = getter(*it);
        if (match_string(values, cmp, patterns)) {
            ++it;
        } else {
            it = get_data().erase(it);
            ++filtered;
        }
    }
    return filtered;
}

template <typename T>
inline std::size_t Query<T>::filter(FilterFunctionInt64 & getter, QueryCmp cmp, int64_t pattern) {
    std::size_t filtered = 0;

    for (auto it = get_data().begin(); it != get_data().end(); ) {
        auto value = getter(*it);
        if (match_int64(value, cmp, pattern)) {
            ++it;
        } else {
            it = get_data().erase(it);
            ++filtered;
        }
    }
    return filtered;
}

template <typename T>
inline std::size_t Query<T>::filter(FilterFunctionVectorInt64 & getter, QueryCmp cmp, int64_t pattern) {
    std::size_t filtered = 0;

    for (auto it = get_data().begin(); it != get_data().end(); ) {
        auto values = getter(*it);
        if (match_int64(values, cmp, pattern)) {
            ++it;
        } else {
            it = get_data().erase(it);
            ++filtered;
        }
    }
    return filtered;
}

template <typename T>
inline std::size_t Query<T>::filter(FilterFunctionInt64 & getter, QueryCmp cmp, const std::vector<int64_t> & patterns) {
    std::size_t filtered = 0;

    for (auto it = get_data().begin(); it != get_data().end(); ) {
        auto value = getter(*it);
        if (match_int64(value, cmp, patterns)) {
            ++it;
        } else {
            it = get_data().erase(it);
            ++filtered;
        }
    }
    return filtered;
}

template <typename T>
inline std::size_t Query<T>::filter(FilterFunctionVectorInt64 & getter, QueryCmp cmp, const std::vector<int64_t> & patterns) {
    std::size_t filtered = 0;

    for (auto it = get_data().begin(); it != get_data().end(); ) {
        auto values = getter(*it);
        if (match_int64(values, cmp, patterns)) {
            ++it;
        } else {
            it = get_data().erase(it);
            ++filtered;
        }
    }
    return filtered;
}

// TODO: other cmp
template <typename T>
inline std::size_t Query<T>::filter(Query<T>::FilterFunctionBool & getter, QueryCmp cmp, bool pattern) {
    std::size_t filtered = 0;

    for (auto it = get_data().begin(); it != get_data().end(); ) {
        auto value = getter(*it);
        if (cmp == QueryCmp::EQ && value == pattern) {
            ++it;
        } else {
            it = get_data().erase(it);
            ++filtered;
        }
    }
    return filtered;
}

template <typename T>
inline std::size_t Query<T>::filter(Query<T>::FilterFunctionCString & getter, QueryCmp cmp, const std::string & pattern) {
    std::size_t filtered = 0;

    for (auto it = get_data().begin(); it != get_data().end(); ) {
        auto value = getter(*it);
        if (match_string(value, cmp, pattern)) {
            ++it;
        } else {
            it = get_data().erase(it);
            ++filtered;
        }
    }
    return filtered;
}


}  // namespace libdnf::utils::sack


#endif
