/**

   @copyright
   @verbatim
   Copyright @ 2019 Audi AG. All rights reserved.
   
       This Source Code Form is subject to the terms of the Mozilla
       Public License, v. 2.0. If a copy of the MPL was not distributed
       with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
   
   If it is not possible or desirable to put the notice in a particular file, then
   You may include the notice in a location (such as a LICENSE file in a
   relevant directory) where a recipient would be likely to look for such a notice.
   
   You may add additional accurate notices of copyright ownership.
   @endverbatim
 */

#pragma once

#include <a_util/system.h>
#include <a_util/logging.h>
#include <a_util/process.h>
#include <a_util/strings.h>
#include <thread>
#include <fep3/core.h>
#include <fep3/core/participant_executor.hpp>
#include <fep3/components/configuration/propertynode.h>

#ifdef WIN32
    #pragma warning( push )
    #pragma warning( disable : 4250 )
#endif

static const std::string makePlatformDepName(const char* strOrigName)
{
    std::string strModuleNameDep(strOrigName);

    //strModuleNameDep.append(cTime::GetCurrentSystemTime().format("_%H%M%S"));

#if (_MSC_VER == 1600)
    strModuleNameDep.append("_win64_vc100");
#elif (_MSC_VER == 1900)
    strModuleNameDep.append("_win64_vc140");
#elif (_MSC_VER == 1910 || _MSC_VER == 1911)
    strModuleNameDep.append("_win64_vc141");
#elif (defined (__linux))
    strModuleNameDep.append("_linux");
#else
    // this goes for vc90, vc120 or apple or arm or whatever.
#error "Platform currently not supported";
#endif // Version check

    std::stringstream ss;
    ss << std::this_thread::get_id();

    strModuleNameDep += "_" + a_util::strings::toString(a_util::process::getCurrentProcessId());
    strModuleNameDep += "_" + ss.str();
    return strModuleNameDep;
}

struct TestElement : public fep3::core::ElementBase
{
    TestElement() 
        : fep3::core::ElementBase("Testelement", "3.0")
    {
    }

    fep3::Result load() override
    {
        auto config_service = getComponents()->getComponent<fep3::IConfigurationService>();
        if (config_service)
        {
            // System property
            config_service->createSystemProperty("system_parameter", fep3::PropertyType<int32_t>::getTypeName(), "1");

            // Participant properties
            auto test_config_node = std::make_shared<fep3::NativePropertyNode>("test_config", "", "node");
            test_config_node->setChild(fep3::makeNativePropertyNode<double>("pos1", 0.0));
            test_config_node->setChild(fep3::makeNativePropertyNode<bool>("bool_value", false));
            test_config_node->setChild(fep3::makeNativePropertyNode<double>("double_pos2", 1.1));
            test_config_node->setChild(fep3::makeNativePropertyNode<std::string>("string_test", "empty"));
            test_config_node->setChild(fep3::makeNativePropertyNode<int32_t>("parameter1", 1));
            test_config_node->setChild(fep3::makeNativePropertyNode<int32_t>("pos_X", 11));
            config_service->registerNode(test_config_node);
            return{};
        }
        RETURN_ERROR_DESCRIPTION(fep3::ERR_FAILED, "Unable to initialize test element: Configuration service not reachable.");
    }
};


struct PartStruct
{
    PartStruct(PartStruct&&) = default;
    ~PartStruct() = default;
    PartStruct(fep3::core::Participant&& part) 
        : _part(std::move(part)), _part_executor(_part)
    {
    }
    fep3::core::Participant _part;
    fep3::core::ParticipantExecutor _part_executor;
};

using TestParticipants = std::map<std::string, std::unique_ptr<PartStruct>>;

/**
* @brief Creates participants from the incoming list of names
*
*/
inline TestParticipants createTestParticipants(
    const std::vector<std::string>& participant_names,
    const std::string& system_name)
{
    using namespace fep3::core;
    TestParticipants test_parts;
    std::for_each
        (participant_names.begin()
        , participant_names.end()
        , [&](const std::string& name)
            {
                auto part = createParticipant<ElementFactory<TestElement>>(name, "1.0", system_name);
                auto part_exec = std::make_unique<PartStruct>(std::move(part));
                part_exec->_part_executor.exec();
                test_parts[name].reset(part_exec.release());
            }
        );

    return std::move(test_parts);
}



#ifdef WIN32
    #pragma warning( pop )
#endif