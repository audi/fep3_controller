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

 /**
 * Test Case:   TestSystemLibrary
 * Test ID:     1.0
 * Test Title:  FEP System Library base test
 * Description: Test if controlling a test system is possible
 * Strategy:    Invoke Controller and issue commands
 * Passed If:   no errors occur
 * Ticket:      -
 * Requirement: -
 */

#include <gtest/gtest.h>
#include <fep_controller/fep_controller.h>
#include <fep_system/fep_system.h>
#include <a_util/system.h>
#include <a_util/xml.h>
#include <a_util/filesystem.h>

#include <fep3/core/participant.h>
#include <fep3/components/configuration/propertynode_helper.h>
#include <fep3/components/configuration/propertynode.h>

#include <string>
#include <memory>

#include "fep_test_common.h"

/**
 * Test base functionality
 * @req_id ""
 */
TEST(TesterControllerLib, testConnectSystem)
{
    using namespace fep3::core::arya;

    const std::vector<std::string> participant_names{ "participant1", "participant2" };
    const auto system_name{ "FEP_SYSTEM" };
    auto lst_parts = createTestParticipants(participant_names, system_name);

    a_util::filesystem::Path test_file(TESTFILES_DIR);
    test_file.append("files/2_participants.fep_sdk_system");

    std::unique_ptr<fep3::System> system_to_test;
    ASSERT_NO_THROW(system_to_test = std::make_unique<fep3::System>(
        fep3::controller::connectSystem(test_file)));

    ASSERT_EQ(system_to_test->getSystemName(), "FEP_SYSTEM");

    size_t counter = 1;
    for (auto part_proxy : system_to_test->getParticipants())
    {
        ASSERT_EQ(part_proxy.getName(), a_util::strings::format("participant%d", counter).c_str());
        ASSERT_EQ(part_proxy.getStartPriority(), counter-1);
        ASSERT_EQ(part_proxy.getInitPriority(), counter-1);
        auto stm = part_proxy.getRPCComponentProxyByIID<fep3::rpc::IRPCParticipantStateMachine>();
        ASSERT_TRUE(static_cast<bool>(stm));
        ASSERT_EQ(stm->getState(), fep3::rpc::IRPCParticipantStateMachine::State::unloaded);
        counter++;
    }

    system_to_test->shutdown();
}

/**
 * @brief Test whether incorrect file name/path is reported as error
 * @req_id ""
 */
TEST(TesterControllerLib, testConnectSystemFileNotExisting)
{
    try 
    {
        fep3::controller::connectSystem("file_does_not_exist");
        FAIL() << "Expected std::runtime_error";
    }
    catch (std::runtime_error const & err)
    {
        std::string error_what = err.what();
        EXPECT_NE(error_what.find(std::string("file_does_not_exist"), 0), std::string::npos);
        EXPECT_NE(error_what.find(std::string("does not exist"), 0), std::string::npos);
    }
    catch (...)
    {
        FAIL() << "Expected std::runtime_error";
    }
}

/**
 * @brief Test whether incorrect XML syntax is reported as error
 * @req_id ""
 */
TEST(TesterControllerLib, testConnectSystemXmlParseError)
{
    a_util::filesystem::Path test_file(TESTFILES_DIR);
    test_file.append("files/xml_parse_error.fep_sdk_system");

    try
    {
        fep3::controller::connectSystem(test_file);
        FAIL() << "Expected std::runtime_error";
    }
    catch (std::runtime_error const & err)
    {
        std::string error_what = err.what();
        EXPECT_NE(error_what.find(std::string("xml_parse_error.fep_sdk_system")), std::string::npos);
        EXPECT_NE(error_what.find(std::string("xml parse error")), std::string::npos);
        EXPECT_NE(error_what.find(std::string("Start-end tags mismatch")), std::string::npos);
    }
    catch (...)
    {
        FAIL() << "Expected std::runtime_error";
    }
}

/**
 * @brief Test whether a file that does not validate the XSD is reported as error
 * @req_id ""
 */
TEST(TesterControllerLib, testConnectSystemDataModelParseError)
{
    a_util::filesystem::Path test_file(TESTFILES_DIR);
    test_file.append("files/data_model_parse_error.fep_sdk_system");

    try
    {
        fep3::controller::connectSystem(test_file);
        FAIL() << "Expected std::runtime_error";
    }
    catch (std::runtime_error const & err)
    {
        std::string error_what = err.what();
        EXPECT_NE(error_what.find(std::string("data_model_parse_error.fep_sdk_system")), std::string::npos);
        EXPECT_NE(error_what.find(std::string("fep sdk system data model parse error")), std::string::npos);
        EXPECT_NE(error_what.find(std::string("element \"file_reference\" not found")), std::string::npos);
    }
    catch (...)
    {
        FAIL() << "Expected std::runtime_error";
    }
}