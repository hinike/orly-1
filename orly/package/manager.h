/* <orly/package/manager.h>

   Package management.

   Copyright 2010-2014 OrlyAtomics, Inc.

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

     http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License. */

#pragma once

#include <memory>
#include <shared_mutex>
#include <unordered_map>
#include <unordered_set>

#include <base/thrower.h>
#include <orly/package/loaded.h>
#include <orly/package/name.h>

namespace Orly {

  namespace Package {

    /* Co-ordinates the loading, upgrading, and uninstalling of packages, as well as getting a package to do work
       with it. Also does proper graph upgrades of packages and the like. */
    class TManager {
      public:

      DEFINE_ERROR(TError, std::runtime_error, "package manager error");

      DEFINE_ERROR(TPackageDirError, std::runtime_error, "package dir error");

      typedef std::unordered_map<TName, TLoaded::TPtr> TInstalled;
      typedef std::unordered_set<TName> TNames;
      typedef std::unordered_set<TVersionedName> TVersionedNames;

      /* Make a new package manager which will load packages from the given directory. */
      TManager(const Jhm::TTree &package_dir);

      /* Unload (But don't uninstall) all currently used packages. There is no chance of failure. */
      ~TManager();

      /* Get a package given a name, throw if it is not around. Note that eventually this will be a construction of
         a package handle, but since we don't need the read/write lock logic, we're good for now. */
      TLoaded::TPtr Get(const TName &package) const;

      /* Install or upgrade a set of packages atomically. Packages can't have dependnecies, installers, uninstallers,
         etc, so currently this is identical to load. */
      void Install(const TVersionedNames &packages,
                   const std::function<void(TLoaded::TPtr, bool is_new_version)> &pre_install_step = [](TLoaded::TPtr,
                                                                                                        bool) {});

      /* Load a package. Packages don't have dependencies, so this is simply check that exists, build new installed
         map, and swap in. */
      void Load(const TVersionedNames &packages,
                const std::function<void(TLoaded::TPtr, bool is_new_version)> &pre_install_step = [](TLoaded::TPtr,
                                                                                                     bool) {});

      /* Get the package directory. */
      const Jhm::TTree &GetPackageDir() const;

      /* Set the package directory. An explicit call, because TService statically constructs... */
      void SetPackageDir(const Jhm::TTree &package_dir);

      /* Uninstall a package. Simply removes it from the installed package set, which means when the last reference
         goes away, the package will be dlclosed. This is identical to Unload as packages can't have uninstallers at
         the moment. */
      void Uninstall(const TVersionedNames &packages);

      /* Yield each installed package */
      void YieldInstalled(std::function<bool (const TVersionedName &name)> cb) const;

      protected:

      private:
      Jhm::TTree PackageDir;

      //TODO: Engineer the lock out of existence as much as possible.
      mutable std::shared_timed_mutex InstallLock;

      /* Map of all the installed packages. We work aside to build/change the package map, then swap it out. */
      TInstalled Installed;

      friend class TPackageHandle;
    }; // TManager

  }  // Package

}  // Orly
