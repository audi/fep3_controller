## ℹ️ This repository is archived 

It is now maintained at https://github.com/cariad-tech


---
<!---
  Copyright @ 2019 Audi AG. All rights reserved.
  
      This Source Code Form is subject to the terms of the Mozilla
      Public License, v. 2.0. If a copy of the MPL was not distributed
      with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
  
  If it is not possible or desirable to put the notice in a particular file, then
  You may include the notice in a location (such as a LICENSE file in a
  relevant directory) where a recipient would be likely to look for such a notice.
  
  You may add additional accurate notices of copyright ownership.
  -->
# FEP 3 SDK Controller Library  {#mainpage}

# Description

## FEP 3 SDK Controller Library functionality in a nutshell 

### The possibility to create a fep::System with the help of the FEP SDK Metamodel Library

* see [fep_controller API](include/fep_controller/fep_controller.h)

### The possibility to confgure a fep::System with the help of the FEP SDK Metamodel Library

* see [fep_controller API](include/fep_controller/fep_controller.h)

# Dependencies

* fep_sdk_system package (i.e. fep_sdk_system/3.x.y@aev25/stable)
* internally it uses fep_sdk_metamodel package (i.e. fep_sdk_metamodel/a.b.c@aev25/stable)

## How to use ###

The FEP Controller Library provides a CMake >= 3.5 configuration. Here's how to use it from your own CMake projects:

    find_package(fep3_controller_lib REQUIRED)

You can append the FEP Controller Library target to your existing targets to add a dependency:

    target_link_libraries(my_existing_target PRIVATE fep3_controller_lib)

Note that the fep_controller_lib target will transitively pull in all required include directories and libraries.

### Build Environment ####

The libraries are build and tested only under following compilers and operating systems: 
* Windows 10 x64 with Visual Studio C++ 2015 Update 3.1 (Update 3 and KB3165756)
* Linux Ubuntu 16.04 LTS x64 with GCC 5.4 and libstdc++14 (C++14 ABI)




