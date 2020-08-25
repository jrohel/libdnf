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


#include "../libdnf/utils/bgettext/bgettext-lib.h"
#include "../libdnf/rpm/solv/package_private.hpp"
#include "package_set_impl.hpp"
#include "repo_impl.hpp"
#include "solv_sack_impl.hpp"
#include "solv/id_queue.hpp"

#include "libdnf/rpm/transaction.hpp"

#include <fmt/format.h>

#include <filesystem>

#include <fcntl.h>

#include <rpm/rpmlib.h>
#include <rpm/rpmtag.h>
#include <rpm/rpmpgp.h>
#include <rpm/rpmdb.h>
#include <rpm/rpmbuild.h>

#include <rpm/rpmts.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <map>

namespace libdnf::rpm {

class RpmProblem::Impl {
public:
    Impl(rpmProblem problem) : problem(problem) {}

    rpmProblem problem{nullptr};
};

RpmProblem::RpmProblem(std::unique_ptr<Impl> && p_impl) : p_impl(std::move(p_impl)) {}

RpmProblem::~RpmProblem() = default;


class RpmProblemSet::Impl {
public:
    ~Impl() {
        rpmpsFree(problem_set);
    }

    rpmps problem_set{nullptr};
};

RpmProblemSet::~RpmProblemSet() = default;

class RpmProblemSet::iterator::Impl {
public:
    Impl() { iter = nullptr; }
    explicit Impl(rpmps problem_set) { iter = rpmpsInitIterator(problem_set); }
    //explicit Impl(rpmpsi iter) : iter(iter) {}
    ~Impl() { rpmpsFreeIterator(iter); }

    Impl& next() {
        if (!rpmpsiNext(iter)) {
            free();
        }
        return *this;
    }

    RpmProblem operator*() {
        auto problem = rpmpsGetProblem(iter);
        auto rpm_problem_impl = std::make_unique<RpmProblem::Impl>(problem);
        return RpmProblem(std::move(rpm_problem_impl));
    }

    void free() {
        rpmpsFreeIterator(iter);
        iter = nullptr;
    }

    rpmpsi iter;
};

RpmProblemSet::iterator::iterator() : p_impl(std::make_unique<Impl>()) {}

RpmProblemSet::iterator::iterator(std::unique_ptr<Impl> && p_impl) : p_impl(std::move(p_impl)) {}

RpmProblemSet::iterator RpmProblemSet::begin() {
    auto iter_impl = std::make_unique<iterator::Impl>(p_impl->problem_set);
    return iterator(std::move(iter_impl));
}

RpmProblemSet::iterator RpmProblemSet::end() {
    return iterator();
}

RpmProblemSet::iterator& RpmProblemSet::iterator::operator++() {
    p_impl->next();
    return *this;
};

/*RpmProblemSet::iterator RpmProblemSet::iterator::operator++(int) {
    auto ret = *this;
    ++*this;
    return ret;
};*/

bool RpmProblemSet::iterator::operator==(iterator & other) const {
    return rpmpsGetProblem(p_impl->iter) == rpmpsGetProblem(other.p_impl->iter);
}

RpmProblem RpmProblemSet::iterator::operator*() {
    return **p_impl;
}

int RpmProblemSet::size() noexcept {
    return rpmpsNumProblems(p_impl->problem_set);
}

class Transaction::Impl {
public:
/*    class CBKey {
    public:
        CBKey(const std::string & file_path) : file_path(file_path) {}
        CBKey(std::string && file_path) : file_path(std::move(file_path)) {}
        std::string file_path;
    };*/

    explicit Impl(Base & base);
    Impl(Base & base, rpmVSFlags vsflags);
    ~Impl();

    /// Set transaction script file handle, i.e. stdout/stderr on scriptlet execution
    /// @param script_fd  new script file handle (or NULL)
    void set_script_fd(FD_t script_fd) noexcept {
        rpmtsSetScriptFd(ts, script_fd);
        if (this->script_fd) {
            Fclose(this->script_fd);
        }
        this->script_fd = script_fd;
    }

    /// Set transaction root directory, i.e. path to chroot(2).
    /// @param root_dir  new transaction root directory (or NULL)
    /// @return  0 on success, -1 on error (invalid root directory)
    void set_root_dir(const char * root_dir) {
        auto rc = rpmtsSetRootDir(ts, root_dir);
        if (rc != 0) {
            throw Exception(std::string("Invalid root directory: ") + root_dir);
        }
    }

    /// Get transaction root directory, i.e. path to chroot(2).
    /// @return  transaction root directory
    const char * get_root_dir() const {
        return rpmtsRootDir(ts);
    }

    /// Retrieve color bits of transaction set.
    /// @return  color bits
    rpm_color_t get_color() const {
        return rpmtsColor(ts);
    }

    /// Set color bits of transaction set.
    /// @param color  new color bits
    /// @return  previous color bits
    rpm_color_t set_color(rpm_color_t color) {
        return rpmtsSetColor(ts, color);
    }

    /// Retrieve preferred file color
    /// @return  color bits
    rpm_color_t get_pref_color() const {
        return rpmtsPrefColor(ts);
    }

    /// Set preferred file color
    /// @param color  new color bits
    /// @return  previous color bits
    rpm_color_t set_pref_color(rpm_color_t color) {
        return rpmtsSetPrefColor(ts, color);
    }

    /// Get transaction flags, i.e. bits that control rpmtsRun().
    /// @return  transaction flags
    rpmtransFlags get_flags() const { return rpmtsFlags(ts); }

    /// Set transaction flags, i.e. bits that control rpmtsRun().
    /// @param flags  new transaction flags
    /// @return  previous transaction flags
    rpmtransFlags set_flags(rpmtransFlags flags) {
        return rpmtsSetFlags(ts, flags);
    }

    /// Get verify signatures flag(s).
    /// @return  verify signatures flags
    rpmVSFlags get_signature_verify_flags() const {
        return rpmtsVSFlags(ts);
    }

    /// Set verify signatures flag(s).
    /// @param verify_flags  new verify signatures flags
    /// @return         previous value
    rpmVSFlags set_signature_verify_flags(rpmVSFlags verify_flags) {
        return rpmtsSetVSFlags(ts, verify_flags);
    }

    /// Get package verify flag(s).
    /// @return  verify signatures flags
    rpmVSFlags get_pkg_verify_flags() const {
        return rpmtsVfyFlags(ts);
    }

    /// Set package verify flag(s).
    /// @param verify_flags  new package verify flags
    /// @return  old package verify flags
    rpmVSFlags set_pkg_verify_flags(rpmVSFlags verify_flags) {
        return rpmtsSetVfyFlags(ts, verify_flags);
    }

    /// Get enforced package verify level
    /// @return  package verify level
    int get_pkg_verify_level() const {
        return rpmtsVfyLevel(ts);
    }

    /// Set enforced package verify level
    /// @param verify_level  new package verify level
    /// @return  old package verify level
    int set_pkg_verify_level(int verify_level) {
        return rpmtsSetVfyLevel(ts, verify_level);
    }

    /// Get transaction id, i.e. transaction time stamp.
    /// @return  transaction id
    rpm_tid_t get_id() const {
        return rpmtsGetTid(ts);
    }

    /// Return header from package.
    /// @param path  file path
    /// @retval hdrp  header (NULL on failure)
    /// @return  package header
    Header read_pkg_header(const std::string & file_path) const {
        FD_t fd = Fopen(file_path.c_str(), "r.ufdio");

        if (!fd) {
            throw libdnf::RuntimeError("read_pkg_header: Can't open file: " + file_path);
        }

        Header h;
        const char * descr = file_path.c_str();
        rpmRC rpmrc = rpmReadPackageFile(ts, fd, descr, &h);
        Fclose(fd);

        switch (rpmrc) {
        case RPMRC_NOTFOUND:
            /* XXX Read a package manifest. Restart ftswalk on success. */
        case RPMRC_FAIL:
        default:
            h = headerFree(h);
            throw libdnf::RuntimeError("read_pkg_header: failed");
            break;
        case RPMRC_NOTTRUSTED:
        case RPMRC_NOKEY:
        case RPMRC_OK:
            break;
        }

        return h;
    }

    /// Add package to be installed to transaction set.
    /// The transaction set is checked for duplicate package names.
    /// If found, the package with the "newest" EVR will be replaced.
    /// @param item  item to be installed
    void install(Item & item) {
        install_or_upgrade(item, false);
    }

    /// Add package to be upgraded to transaction set.
    /// The transaction set is checked for duplicate package names.
    /// If found, the package with the "newest" EVR will be replaced.
    /// @param item  item to be upgraded
    void upgrade(Item & item) {
        install_or_upgrade(item, true);
    }

    /// Add package to be reinstalled to transaction set.
    /// The transaction set is checked for duplicate package names.
    /// If found, the package with the "newest" EVR will be replaced.
    /// @param item  item to be reinstalled
    void reinstall(Item & item) {
        auto file_path = item.pkg.get_local_filepath();
        auto header = read_pkg_header(file_path);
        auto rc = rpmtsAddReinstallElement(ts, header, &item);
        if (rc != 0) {
            std::string msg = "Can't reinstall package \"" + file_path + "\"";
            throw libdnf::RuntimeError(msg);
        }
        auto [iter, inserted] = items.insert({header, &item});
        if (!inserted) {
            throw libdnf::RuntimeError("The package already exists in rpm::Transaction");
        }
    }

    /// Add package to be erased to transaction set.
    /// @param item  item to be erased
    void erase(Item & item) {
        auto rpmdb_id = static_cast<unsigned int>(item.pkg.get_rpmdbid());
        auto header = get_header(rpmdb_id);
        int unused = -1;
        int rc = rpmtsAddEraseElement(ts, header, unused);
        headerFree(header);
        if (rc != 0) {
            throw libdnf::RuntimeError("Can't remove package");
        }
        auto [iter, inserted] = items.insert({header, &item});
        if (!inserted) {
            throw libdnf::RuntimeError("The package already exists in rpm::Transaction");
        }
    };

    void register_cb(std::unique_ptr<TransactionCB> && cb) {
        cb_info.cb = std::move(cb);
    }

    /// Perform dependency resolution on the transaction set.
    /// Any problems found by rpmtsCheck() can be examined by retrieving the problem set with rpmtsProblems(),
    /// success here only means that the resolution was successfully attempted for all packages in the set.
    /// @return  true on dependencies are ok
    bool check() noexcept {
        return rpmtsCheck(ts) == 0;
    }

    /// Return current transaction set problems.
    /// @return  current problem set (or NULL if no problems)
    rpmps get_problems() {
        return rpmtsProblems(ts);
    }

    rpmdbMatchIterator matchPackages(unsigned int value) {
        return rpmtsInitIterator(ts, RPMDBI_PACKAGES, &value, sizeof(value));
    }

    rpmdbMatchIterator matchTag(rpmDbiTagVal tag, const char * value) {
        if (tag == RPMDBI_PACKAGES) {
            throw libdnf::LogicError("rpm::Transaction::match(): not allowed tag RPMDBI_PACKAGES");
        }
        return rpmtsInitIterator(ts, tag, value, 0);
    }

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
    int run() {
        rpmprobFilterFlags ignore_set = RPMPROB_FILTER_NONE;
        if (cb_info.cb) {
            rpmtsSetNotifyCallback(ts, ts_callback, &cb_info);
        }
        auto rc = rpmtsRun(ts, nullptr, ignore_set);
        if (cb_info.cb) {
            rpmtsSetNotifyCallback(ts, nullptr, nullptr);
        }
        return rc;
    }

/**
 * dnf_rpmts_find_package:
 **/
Header
get_header(unsigned int rec_offset)
{
    Header hdr = NULL;
//    unsigned int recOffset;
//    g_autoptr(GString) rpm_error = NULL;

    /* XXX If not already opened, open the database O_RDONLY now. */
    /* XXX FIXME: lazy default rdonly open also done by rpmtsInitIterator(). */
    /*if (rpmtsGetRdb(ts) == NULL) {
        int rc = rpmtsOpenDB(ts, O_RDONLY);
        if (rc || rpmtsGetRdb(ts) == NULL) {
            throw libdnf::RuntimeError("rpmdb open failed");
        }
    }*/

    /* find package by db-id */
//    rpmlogSetCallback(dnf_rpmts_log_handler_cb, &rpm_error);
    // rec_offset must be unsigned int
    auto iter = rpmtsInitIterator(ts, RPMDBI_PACKAGES, &rec_offset, sizeof(rec_offset));
    if (!iter) {
        throw libdnf::RuntimeError(_("Fatal error, run database recovery"));
/*        if (rpm_error != NULL) {
            g_set_error_literal(error,
                                DNF_ERROR,
                                DNF_ERROR_UNFINISHED_TRANSACTION,
                                rpm_error->str);
        } else {
            g_set_error_literal(error,
                                DNF_ERROR,
                                DNF_ERROR_UNFINISHED_TRANSACTION,
                                _("Fatal error, run database recovery"));
        }*/
    }
    hdr = rpmdbNextIterator(iter);
    if (!hdr) {
        throw libdnf::RuntimeError(_("failed to find package"));
/*        g_set_error(error,
                    DNF_ERROR,
                    DNF_ERROR_FILE_NOT_FOUND,
                    _("failed to find package %s"),
                    dnf_package_get_name(pkg));*/
    }
    headerLink(hdr);
//    std::cout << "Header test: " << headerGetString(hdr, RPMTAG_NAME) << std::endl;
//    rpmlogSetCallback(NULL, NULL);
    rpmdbFreeIterator(iter);
    return hdr;
}

private:
    friend class Transaction;

    struct CallbackInfo {
        std::unique_ptr<TransactionCB> cb;
        Impl * transaction;
    };

    Base * base;
    rpmts ts;
    FD_t script_fd{nullptr};
//    std::vector<std::unique_ptr<CBKey>> cb_keys;
    CallbackInfo cb_info{nullptr, this};
    FD_t fd_in_cb; // file descriptor used by transaction in callback (install/reinstall package)
    std::map<Header, Item *> items;

    /// Add package to be installed to transaction set.
    /// The transaction set is checked for duplicate package names.
    /// If found, the package with the "newest" EVR will be replaced.
    /// @param item  item to be erased
    /// @param upgrade  operation: true - upgrade, false - install
    void install_or_upgrade(Item & item, bool upgrade) {
        auto file_path = item.pkg.get_local_filepath();
        auto header = read_pkg_header(file_path);
        auto rc = rpmtsAddInstallElement(ts, header, &item, upgrade ? 1 : 0, nullptr);
        if (rc != 0) {
            std::string msg = "Can't ";
            msg += upgrade ? "upgrade" : "install";
            msg += " package \"" + file_path + "\"";
            throw Exception(msg);
        }
        auto [iter, inserted] = items.insert({header, &item});
        if (!inserted) {
            throw libdnf::RuntimeError("The package already exists in rpm::Transaction");
        }
    }

    /// Function triggered by rpmtsNotify()
    ///
    /// @param hd  related header or NULL
    /// @param what  kind of notification
    /// @param amount  number of bytes/packages already processed or tag of the scriptlet involved
    ///                or 0 or some other number
    /// @param total  total number of bytes/packages to be processed or return code of the scriptlet or 0
    /// @param key  result of rpmteKey() of related rpmte or 0
    /// @param data  user data as passed to rpmtsSetNotifyCallback()
    static void * ts_callback(const void * hd, const rpmCallbackType what,
        const rpm_loff_t amount, const rpm_loff_t total, const void * pkg_key, rpmCallbackData data) {
        void * rc = nullptr;
        auto cb_info = static_cast<CallbackInfo *>(data);
std::cout << "what: " << what << std::endl;
std::cout << "CBData: " << cb_info << std::endl;

        auto transaction = cb_info->transaction;
        //auto & Log = transaction->base->get_logger();
        auto & cb = *cb_info->cb;
//        auto cb_key = static_cast<const CBKey *>(pkg_key);
        auto item = static_cast<const Item *>(pkg_key);

        auto hdr = const_cast<headerToken_s *>(static_cast<const headerToken_s *>(hd));
        const char * name = nullptr;
        if (hdr) {
            name = headerGetString(hdr, RPMTAG_NAME);
        }
        switch (what) {
            case RPMCALLBACK_INST_PROGRESS:
                //rpmtd td;
                //headerGet(hdr, "NAME", td, HEADERGET_DEFAULT);
                cb.install_progress(item, name, amount, total);
                break;
            case RPMCALLBACK_INST_START:
                /*// find pkg
                pkg = dnf_find_pkg_from_filename_suffix(priv->install, filename);
                if (pkg == NULL)
                    g_assert_not_reached();

                // map to correct action code (install/upgrade/downgrade)
                action = dnf_package_get_action(pkg);
                if (action == DNF_STATE_ACTION_UNKNOWN)
                    action = DNF_STATE_ACTION_INSTALL;

                // set the pkgid if not already set
                if (dnf_package_get_pkgid(pkg) == NULL) {
                    const gchar *pkgid;
                    pkgid = headerGetString(hdr, RPMTAG_SHA1HEADER);
                    if (pkgid != NULL) {
                        g_debug("setting %s pkgid %s", name, pkgid);
                        dnf_package_set_pkgid(pkg, pkgid);
                    }
                }

                // install start
                priv->step = DNF_TRANSACTION_STEP_WRITING;
                priv->child = dnf_state_get_child(priv->state);
                dnf_state_action_start(priv->child, action, dnf_package_get_package_id(pkg));
                g_debug("install start: %s size=%i", filename, (gint32)total);*/

                // Install? Maybe upgrade/downgrade/...obsolete?
                cb.install_start(item, name, total);
                break;
            case RPMCALLBACK_INST_OPEN_FILE:
                {
                    auto file_path = item->pkg.get_local_filepath();
                    if (file_path.empty()) {
                        return nullptr;
                    }
                    transaction->fd_in_cb = Fopen(file_path.c_str(), "r.ufdio");
                    rc = transaction->fd_in_cb;
                }
                break;
            case RPMCALLBACK_INST_CLOSE_FILE:
                // just close the file
                if (transaction->fd_in_cb) {
                    Fclose(transaction->fd_in_cb);
                    transaction->fd_in_cb = nullptr;
                }
                break;
            case RPMCALLBACK_TRANS_PROGRESS:
            case RPMCALLBACK_TRANS_START:
            case RPMCALLBACK_TRANS_STOP:
            case RPMCALLBACK_UNINST_PROGRESS:
            case RPMCALLBACK_UNINST_START:
            case RPMCALLBACK_UNINST_STOP:
            case RPMCALLBACK_REPACKAGE_PROGRESS:/* obsolete, unused */
            case RPMCALLBACK_REPACKAGE_START: /* obsolete, unused */
            case RPMCALLBACK_REPACKAGE_STOP: /* obsolete, unused */
            case RPMCALLBACK_UNPACK_ERROR:
            case RPMCALLBACK_CPIO_ERROR:
            case RPMCALLBACK_SCRIPT_ERROR:
            case RPMCALLBACK_SCRIPT_START:
            case RPMCALLBACK_SCRIPT_STOP:
            case RPMCALLBACK_INST_STOP:
            case RPMCALLBACK_ELEM_PROGRESS:
            case RPMCALLBACK_VERIFY_PROGRESS:
            case RPMCALLBACK_VERIFY_START:
            case RPMCALLBACK_VERIFY_STOP:
            case RPMCALLBACK_UNKNOWN:
                return nullptr;
        }

        return rc;
    }
};


Transaction::Impl::Impl(Base & base, rpmVSFlags vsflags) : base(&base) {
    ts = rpmtsCreate();
    auto & config = base.get_config();
    //rpmtsSetRootDir(ts, config.installroot().get_value().c_str());
    //rpmtsSetVSFlags(ts, vsflags);
    set_root_dir(config.installroot().get_value().c_str());
    set_signature_verify_flags(vsflags);
}

Transaction::Impl::Impl(Base & base) : Impl(base, static_cast<rpmVSFlags>(rpmExpandNumeric("%{?__vsflags}"))) {}

Transaction::Impl::~Impl() {
    rpmtsFree(ts);
    if (script_fd) {
        Fclose(script_fd);
    }
}

Transaction::Transaction(Base & base) : p_impl(new Impl(base)) {}

Transaction::~Transaction() = default;

void Transaction::register_cb(std::unique_ptr<TransactionCB> && cb) {
    p_impl->register_cb(std::move(cb));
}

void Transaction::install(Item & item) {
    p_impl->install(item);
}

void Transaction::upgrade(Item & item) {
    p_impl->upgrade(item);
}

void Transaction::reinstall(Item & item) {
    p_impl->reinstall(item);
}

void Transaction::erase(Item & item) {
    p_impl->erase(item);
}

bool Transaction::check() {
    return p_impl->check();
}

int Transaction::run() {
    return p_impl->run();
}

void Transaction::set_script_out_fd(int fd) {
    auto script_fd = fdDup(fd);
    if (!script_fd) {
        throw Exception("fdDup()");
    }
    p_impl->set_script_fd(script_fd);
}

void Transaction::set_script_out_file(const std::string & file_path) {
    auto script_fd = Fopen(file_path.c_str(), "w+b");
    if (!script_fd) {
        throw Exception("Fopen(): " + file_path);
    }
    p_impl->set_script_fd(script_fd);
}

}  // namespace libdnf::rpm
