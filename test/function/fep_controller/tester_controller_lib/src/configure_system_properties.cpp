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
#include <fep3/components/configuration/configuration_service_intf.h>
#include <fep3/components/clock/clock_service_intf.h>
#include <fep3/components/clock_sync/clock_sync_service_intf.h>
#include <fep3/components/scheduler/scheduler_service_intf.h>
#include "fep_test_common.h"

#include <string>
#include <memory>

using namespace fep3;
using namespace fep3::core::arya;

class TestEventMonitor : public fep3::IEventMonitor
{
public:

    void onLog(std::chrono::milliseconds,
        fep3::logging::Severity severity_level,
        const std::string& participant_name,
        const std::string& logger_name,
        const std::string& message) override
    {
        if (severity_level <= logging::Severity::warning)
        {
            std::cerr << message.c_str() << std::endl;
        }
    }
};

class TesterControllerLibProperties : public ::testing::Test
{
protected:
    const std::string part_name_1 = "participant1";
    const std::string part_name_2 = "participant2";
    std::unique_ptr<fep3::System> system_to_test;
    std::shared_ptr<fep3::IProperties> props_part1{nullptr};
    std::shared_ptr<fep3::IProperties> props_part2{nullptr};
    a_util::filesystem::Path test_file_system{ TESTFILES_DIR };
    a_util::filesystem::Path test_file_properties{ TESTFILES_DIR };

private:
    TestParticipants lst_parts;
    TestEventMonitor tem;
   
protected:
    void SetUp()
    {
        const std::vector<std::string> participant_names{ part_name_1, part_name_2 };
        lst_parts = createTestParticipants(participant_names, "FEP_SYSTEM");

        test_file_system.append("files/2_participants.fep_sdk_system");
        ASSERT_NO_THROW(system_to_test = std::unique_ptr<fep3::System>(
            new System(
                std::move(controller::connectSystem(test_file_system)))));
        system_to_test->registerMonitoring(tem);
    }

    bool setupPropertiesInterfaces()
    {
        try
        {
            props_part1 = system_to_test->getParticipant(part_name_1).getRPCComponentProxyByIID<fep3::rpc::IRPCConfiguration>()->getProperties("/");
            props_part2 = system_to_test->getParticipant(part_name_2).getRPCComponentProxyByIID<fep3::rpc::IRPCConfiguration>()->getProperties("/");
            return (props_part1 && props_part2);
        }
        catch (...)
        {
            return false;
        }
    }
};

/**
 * Test base functionality
 * @req_id ""
 */
TEST_F(TesterControllerLibProperties, testConfigureSystem)
{
    // Configure System Properties
    test_file_properties.append("files/2_participants.fep_system_properties");
    ASSERT_NO_THROW(controller::configureSystemProperties(*system_to_test, test_file_properties));
    ASSERT_TRUE(setupPropertiesInterfaces());

    // Check participant1 properties that changed
    EXPECT_EQ(a_util::strings::toDouble(props_part1->getProperty("test_config/pos1")), a_util::strings::toDouble("1.234"));
    EXPECT_EQ(props_part1->getProperty("test_config/bool_value"), "true");
    EXPECT_EQ(a_util::strings::toDouble(props_part1->getProperty("test_config/double_pos2")), a_util::strings::toDouble("0.00"));
    EXPECT_EQ(props_part1->getProperty("test_config/string_test"), "this is a string");
    EXPECT_EQ(props_part1->getProperty("test_config/parameter1"), "3");
    // Check participant1 properties that did NOT change
    EXPECT_EQ(props_part1->getProperty("test_config/pos_X"), "11");
    // Check participant1 system property
    EXPECT_EQ(props_part1->getProperty("system/system_parameter"), "42");

    // Check participant2 properties that did NOT change
    EXPECT_EQ(a_util::strings::toDouble(props_part2->getProperty("test_config/pos1")), a_util::strings::toDouble("0.0"));
    EXPECT_EQ(props_part2->getProperty("test_config/bool_value"), "false");
    EXPECT_EQ(a_util::strings::toDouble(props_part2->getProperty("test_config/double_pos2")), a_util::strings::toDouble("1.1"));
    EXPECT_EQ(props_part2->getProperty("test_config/string_test"), "empty");
    EXPECT_EQ(props_part2->getProperty("test_config/parameter1"), "1");
    // Check participant2 properties that 
    EXPECT_EQ(props_part2->getProperty("test_config/pos_X"), "100");
    // Check participant2 system property
    EXPECT_EQ(props_part2->getProperty("system/system_parameter"), "42");
}

/**
 * @brief Test whether properties having a preceeding '/' may be set and retrieved.
 * @req_id FEPSDK-2171
 */
TEST_F(TesterControllerLibProperties, testConfigureSystemRootProperties)
{
    // Configure System Properties
    test_file_properties.append("files/root_properties.fep_system_properties");
    ASSERT_NO_THROW(controller::configureSystemProperties(*system_to_test, test_file_properties));
    ASSERT_TRUE(setupPropertiesInterfaces());

    // Check participant1 properties that changed
    EXPECT_EQ(a_util::strings::toDouble(props_part1->getProperty("/test_config/pos1")), a_util::strings::toDouble("1.234"));
    EXPECT_EQ(props_part1->getProperty("/test_config/bool_value"), "true");
    EXPECT_EQ(a_util::strings::toDouble(props_part1->getProperty("/test_config/double_pos2")), a_util::strings::toDouble("0.00"));
    EXPECT_EQ(props_part1->getProperty("/test_config/string_test"), "this is a string");
    EXPECT_EQ(props_part1->getProperty("/test_config/parameter1"), "3");
    // Check participant1 properties that did NOT change
    EXPECT_EQ(props_part1->getProperty("/test_config/pos_X"), "11");
    // Check participant1 system property
    EXPECT_EQ(props_part1->getProperty("/system/system_parameter"), "42");

    // Check participant2 properties that did NOT change
    EXPECT_EQ(a_util::strings::toDouble(props_part2->getProperty("/test_config/pos1")), a_util::strings::toDouble("0.0"));
    EXPECT_EQ(props_part2->getProperty("/test_config/bool_value"), "false");
    EXPECT_EQ(a_util::strings::toDouble(props_part2->getProperty("/test_config/double_pos2")), a_util::strings::toDouble("1.1"));
    EXPECT_EQ(props_part2->getProperty("/test_config/string_test"), "empty");
    EXPECT_EQ(props_part2->getProperty("/test_config/parameter1"), "1");
    // Check participant2 properties that 
    EXPECT_EQ(props_part2->getProperty("/test_config/pos_X"), "100");
    // Check participant2 system property
    EXPECT_EQ(props_part2->getProperty("/system/system_parameter"), "42");
}

/**
 * @brief Test whether setting a system property file which contains
 *        invalid formatted properties is refused.
 *        Properties have to use a '/' delimiter instead of '.'.
 * @req_id FEPSDK-2171
 */
TEST_F(TesterControllerLibProperties, testConfigureSystemInvalidPropertyFormat)
{
    test_file_properties.append("files/invalid_property_format.fep_system_properties");

    try
    {
        fep3::controller::configureSystemProperties(*system_to_test, test_file_properties);
        FAIL() << "Expected std::runtime_error";
    }
    catch (std::runtime_error const & err)
    {
        std::string error_what = err.what();
        EXPECT_NE(error_what.find(std::string("Error setting property 'test_config.parameter1' of participant 'participant1'.")), std::string::npos);
    }
    catch (...)
    {
        FAIL() << "Expected std::runtime_error";
    }
}

/**
 * @brief Test whether incorrect file name/path is reported as error
 * @req_id ""
 */
TEST_F(TesterControllerLibProperties, testConfigureSystemFileNotExisting)
{
    try 
    {
        fep3::controller::configureSystemProperties(*system_to_test, "file_does_not_exist");
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
TEST_F(TesterControllerLibProperties, testConfigureSystemXmlParseError)
{
    test_file_properties.append("files/xml_parse_error.fep_system_properties");

    try
    {
        fep3::controller::configureSystemProperties(*system_to_test, test_file_properties);
        FAIL() << "Expected std::runtime_error";
    }
    catch (std::runtime_error const & err)
    {
        std::string error_what = err.what();
        EXPECT_NE(error_what.find(std::string("xml_parse_error.fep_system_properties")), std::string::npos);
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
TEST_F(TesterControllerLibProperties, testConfigureSystemDataModelParseError)
{
    test_file_properties.append("files/data_model_parse_error.fep_system_properties");

    try
    {
        fep3::controller::configureSystemProperties(*system_to_test, test_file_properties);
        FAIL() << "Expected std::runtime_error";
    }
    catch (std::runtime_error const & err)
    {
        std::string error_what = err.what();
        EXPECT_NE(error_what.find(std::string("data_model_parse_error.fep_system_properties")), std::string::npos);
        EXPECT_NE(error_what.find(std::string("fep sdk system properties data model parse error")), std::string::npos);
        EXPECT_NE(error_what.find(std::string("element \"properties\" not found")), std::string::npos);
    }
    catch (...)
    {
        FAIL() << "Expected std::runtime_error";
    }
}

 /**
  * @req_id ""
  */
 TEST_F(TesterControllerLibProperties, testConfigureTiming3NoMaster)
{
    test_file_properties.append("files/2_participants_Timing3NoMaster.fep_system_properties");
    ASSERT_NO_THROW(controller::configureSystemProperties(*system_to_test, test_file_properties));
    ASSERT_TRUE(setupPropertiesInterfaces());

    // participant1
    EXPECT_EQ(props_part1->getProperty(FEP3_CLOCKSYNC_SERVICE_CONFIG_TIMING_MASTER), "");
    EXPECT_EQ(props_part1->getProperty(FEP3_CLOCK_SERVICE_MAIN_CLOCK), FEP3_CLOCK_LOCAL_SYSTEM_REAL_TIME);
    EXPECT_EQ(props_part1->getProperty(FEP3_SCHEDULER_SERVICE_SCHEDULER), FEP3_SCHEDULER_CLOCK_BASED);

    // participant2
    EXPECT_EQ(props_part2->getProperty(FEP3_CLOCKSYNC_SERVICE_CONFIG_TIMING_MASTER), "");
    EXPECT_EQ(props_part2->getProperty(FEP3_CLOCK_SERVICE_MAIN_CLOCK), FEP3_CLOCK_LOCAL_SYSTEM_REAL_TIME);
    EXPECT_EQ(props_part2->getProperty(FEP3_SCHEDULER_SERVICE_SCHEDULER), FEP3_SCHEDULER_CLOCK_BASED);
}

 /**
  * @req_id ""
  */
 TEST_F(TesterControllerLibProperties, testConfigureTiming3AFAP)
{   
    test_file_properties.append("files/2_participants_Timing3AFAP.fep_system_properties");
    ASSERT_NO_THROW(controller::configureSystemProperties(*system_to_test, test_file_properties));
    ASSERT_TRUE(setupPropertiesInterfaces());

    // participant1
    //expect to have the timing master set to participant2
    EXPECT_EQ(props_part1->getProperty(FEP3_CLOCKSYNC_SERVICE_CONFIG_TIMING_MASTER), "participant2");
    EXPECT_EQ(props_part1->getProperty(FEP3_CLOCK_SERVICE_MAIN_CLOCK), FEP3_CLOCK_SLAVE_MASTER_ONDEMAND_DISCRETE);
    EXPECT_EQ(props_part1->getProperty(FEP3_SCHEDULER_SERVICE_SCHEDULER), FEP3_SCHEDULER_CLOCK_BASED);
    
    // participant2
    EXPECT_EQ(props_part2->getProperty(FEP3_CLOCKSYNC_SERVICE_CONFIG_TIMING_MASTER), "participant2");
    EXPECT_EQ(props_part2->getProperty(FEP3_CLOCK_SERVICE_MAIN_CLOCK), FEP3_CLOCK_LOCAL_SYSTEM_SIM_TIME);
    EXPECT_EQ(props_part2->getProperty(FEP3_SCHEDULER_SERVICE_SCHEDULER), FEP3_SCHEDULER_CLOCK_BASED);
    EXPECT_EQ(a_util::strings::toDouble(props_part2->getProperty(FEP3_CLOCK_SERVICE_CLOCK_SIM_TIME_TIME_FACTOR)),
        FEP3_CLOCK_SIM_TIME_TIME_FACTOR_AFAP_VALUE);
    EXPECT_EQ(props_part2->getProperty(FEP3_CLOCK_SERVICE_CLOCK_SIM_TIME_CYCLE_TIME), "50");
}

 /**
  * @req_id ""
  */
 TEST_F(TesterControllerLibProperties, testConfigureTiming3ClockSyncOnlyInterpolation)
{  
    test_file_properties.append("files/2_participants_Timing3ClockSyncOnlyInterpolation.fep_system_properties");
    ASSERT_NO_THROW(controller::configureSystemProperties(*system_to_test, test_file_properties));
    ASSERT_TRUE(setupPropertiesInterfaces());

    // participant1
    //expect to have the timing master set to participant2
    EXPECT_EQ(props_part1->getProperty(FEP3_CLOCKSYNC_SERVICE_CONFIG_TIMING_MASTER), "participant2");
    EXPECT_EQ(props_part1->getProperty(FEP3_CLOCK_SERVICE_MAIN_CLOCK), FEP3_CLOCK_SLAVE_MASTER_ONDEMAND);
    EXPECT_EQ(props_part1->getProperty(FEP3_SCHEDULER_SERVICE_SCHEDULER), FEP3_SCHEDULER_CLOCK_BASED);
    EXPECT_EQ(props_part1->getProperty(FEP3_CLOCKSYNC_SERVICE_CONFIG_SLAVE_SYNC_CYCLE_TIME), "10");

    // participant2
    EXPECT_EQ(props_part2->getProperty(FEP3_CLOCKSYNC_SERVICE_CONFIG_TIMING_MASTER), "participant2");
    EXPECT_EQ(props_part2->getProperty(FEP3_CLOCK_SERVICE_MAIN_CLOCK), FEP3_CLOCK_LOCAL_SYSTEM_REAL_TIME);
    EXPECT_EQ(props_part2->getProperty(FEP3_SCHEDULER_SERVICE_SCHEDULER), FEP3_SCHEDULER_CLOCK_BASED);
}

 /**
  * @req_id ""
  */
 TEST_F(TesterControllerLibProperties, testConfigureTiming3ClockSyncOnlyDiscrete)
{
    test_file_properties.append("files/2_participants_Timing3ClockSyncOnlyDiscrete.fep_system_properties");
    ASSERT_NO_THROW(controller::configureSystemProperties(*system_to_test, test_file_properties));
    ASSERT_TRUE(setupPropertiesInterfaces());

    // participant1
    //expect to have the timing master set to participant2
    EXPECT_EQ(props_part1->getProperty(FEP3_CLOCKSYNC_SERVICE_CONFIG_TIMING_MASTER), "participant2");
    EXPECT_EQ(props_part1->getProperty(FEP3_CLOCK_SERVICE_MAIN_CLOCK), FEP3_CLOCK_SLAVE_MASTER_ONDEMAND_DISCRETE);
    EXPECT_EQ(props_part1->getProperty(FEP3_SCHEDULER_SERVICE_SCHEDULER), FEP3_SCHEDULER_CLOCK_BASED);
    EXPECT_EQ(props_part1->getProperty(FEP3_CLOCKSYNC_SERVICE_CONFIG_SLAVE_SYNC_CYCLE_TIME), "10");

    // participant2
    EXPECT_EQ(props_part2->getProperty(FEP3_CLOCKSYNC_SERVICE_CONFIG_TIMING_MASTER), "participant2");
    EXPECT_EQ(props_part2->getProperty(FEP3_CLOCK_SERVICE_MAIN_CLOCK), FEP3_CLOCK_LOCAL_SYSTEM_REAL_TIME);
    EXPECT_EQ(props_part2->getProperty(FEP3_SCHEDULER_SERVICE_SCHEDULER), FEP3_SCHEDULER_CLOCK_BASED);
}

 /**
  * @req_id ""
  */
 TEST_F(TesterControllerLibProperties, testConfigureTiming3DiscreteSteps)
{
    test_file_properties.append("files/2_participants_Timing3DiscreteSteps.fep_system_properties");
    ASSERT_NO_THROW(controller::configureSystemProperties(*system_to_test, test_file_properties));
    ASSERT_TRUE(setupPropertiesInterfaces());

    // participant1
    //expect to have the timing master set to participant2
    EXPECT_EQ(props_part1->getProperty(FEP3_CLOCKSYNC_SERVICE_CONFIG_TIMING_MASTER), "participant2");
    EXPECT_EQ(props_part1->getProperty(FEP3_CLOCK_SERVICE_MAIN_CLOCK), FEP3_CLOCK_SLAVE_MASTER_ONDEMAND_DISCRETE);
    EXPECT_EQ(props_part1->getProperty(FEP3_SCHEDULER_SERVICE_SCHEDULER), FEP3_SCHEDULER_CLOCK_BASED);

    // participant2
    EXPECT_EQ(props_part2->getProperty(FEP3_CLOCKSYNC_SERVICE_CONFIG_TIMING_MASTER), "participant2");
    EXPECT_EQ(props_part2->getProperty(FEP3_CLOCK_SERVICE_MAIN_CLOCK), FEP3_CLOCK_LOCAL_SYSTEM_SIM_TIME);
    EXPECT_EQ(props_part2->getProperty(FEP3_SCHEDULER_SERVICE_SCHEDULER), FEP3_SCHEDULER_CLOCK_BASED);
    EXPECT_EQ(a_util::strings::toDouble(props_part2->getProperty(FEP3_CLOCK_SERVICE_CLOCK_SIM_TIME_TIME_FACTOR)), 
        a_util::strings::toDouble("0.5"));
    EXPECT_EQ(props_part2->getProperty(FEP3_CLOCK_SERVICE_CLOCK_SIM_TIME_CYCLE_TIME), "50");
}