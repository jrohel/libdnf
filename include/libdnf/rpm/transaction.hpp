/*
Copyright (C) 2020 Red Hat, Inc.

This file is part of libdnf: https://github.com/rpm-software-management/libdnf/

Libdnf is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

Libdnf is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with libdnf.  If not, see <https://www.gnu.org/licenses/>.
*/


#ifndef LIBDNF_RPM_TRANSACTION_HPP
#define LIBDNF_RPM_TRANSACTION_HPP

#include "libdnf/utils/exception.hpp"
#include "libdnf/utils/weak_ptr.hpp"
#include "libdnf/rpm/package.hpp"

#include <memory>

namespace libdnf {

class Base;

}  // namespace libdnf

namespace libdnf::rpm {

class Transaction;
class TransactionCB;
class RpmProblemSet;

class RpmProblem {
public:
    ~RpmProblem();

private:
    friend RpmProblemSet;
    class Impl;
    RpmProblem(std::unique_ptr<Impl> && p_impl);
    std::unique_ptr<Impl> p_impl;
};

class RpmProblemSet {
public:
    class iterator {
    public:
        // iterator traits
        using difference_type = void;
        using value_type = RpmProblem;
        using pointer = const RpmProblem *;
        using reference = const RpmProblem &;
        using iterator_category = std::forward_iterator_tag;

        iterator();
        iterator& operator++();
//        iterator operator++(int);
        bool operator==(iterator & other) const;
        bool operator!=(iterator other) const { return !(*this == other); }
        RpmProblem operator*();

    private:
        friend RpmProblemSet;
        class Impl;
        iterator(std::unique_ptr<Impl> && p_impl);
        std::unique_ptr<Impl> p_impl;
    };

    iterator begin();
    iterator end();

    ~RpmProblemSet();

    /// Return number of problems in set.
    /// @return  number of problems
    int size() noexcept;

    bool empty() noexcept { return size() == 0; }

private:
    class Impl;
    std::unique_ptr<Impl> p_impl;
};


class Transaction {
public:
    class Exception : public RuntimeError {
    public:
        using RuntimeError::RuntimeError;
        const char * get_domain_name() const noexcept override { return "libdnf::rpm::Transaction"; }
        const char * get_name() const noexcept override { return "Exception"; }
        const char * get_description() const noexcept override { return "rpm::Transaction exception"; }
    };

    struct Item {
        Item(Package pkg) : pkg(pkg) {}
        Package pkg;
    };

    explicit Transaction(Base & base);
    ~Transaction();

    void register_cb(std::unique_ptr<TransactionCB> && cb);

    void install(Item & item);
    void upgrade(Item & item);
    void reinstall(Item & item);
    void erase(Item & item);

    /// Perform a dependency check on the transaction set.
    /// After headers have been added to a transaction set,
    /// a dependency check can be performed to make sure that all package dependencies are satisfied.
    /// Any found problems can be examined by retrieving the problem set with rpmtsProblems().
    /// @return  true on dependencies are ok
    bool check();

    /// Process all package elements in a transaction set.
    /// Before calling rpmtsRun be sure to have:
    ///
    ///    - setup the rpm root dir via rpmtsSetRoot().
    ///    - setup the rpm notify callback via rpmtsSetNotifyCallback().
    ///    - setup the rpm transaction flags via rpmtsSetFlags().
    ///
    /// Additionally, though not required you may want to:
    ///
    ///    - setup the rpm verify signature flags via rpmtsSetVSFlags().
    ///
    /// @param okProbs  unused
    /// @param ignoreSet	bits to filter problem types
    /// @return		0 on success, -1 on error, >0 with newProbs set
    int run();

    /// Set transaction script file descriptor, i.e. stdout/stderr on scriptlet execution.
    /// The file descriptor is copied using dup().
    /// @param fd  new script file descriptor (or NULL)
    void set_script_out_fd(int fd);

    /// Set transaction script file, i.e. stdout/stderr on scriptlet execution.
    /// The set_script_fd() can be used to pass file handle.
    /// @param file_path  new file path
    void set_script_out_file(const std::string & file_path);

private:
    class Impl;
    std::unique_ptr<Impl> p_impl;
};

class TransactionCB {
public:
    virtual ~TransactionCB() = default;

    virtual void install_start(const Transaction::Item * /*item*/, const std::string & /*name*/, uint64_t /*total*/) {}
    virtual void install_progress(const Transaction::Item * /*item*/, const std::string & /*name*/, uint64_t /*amount*/, uint64_t /*total*/) {};

};


}  // namespace libdnf::rpm

#endif  // LIBDNF_RPM_TRANSACTION_HPP
