/**
 * \file    driver.cpp
 * \brief   Provides the configured CAN driver instance.
 *
 * DriverInterface::getInstance() owns and returns the single driver instance
 * used by the controllers. The build configuration selects MockDriver when
 * __USE_MOCK_DRIVER is defined; otherwise it selects the AVR hardware driver.
 * The returned pointer is non-owning, and the function-local static instance
 * remains available for the lifetime of the program.
 *
 * \copyright Copyright (C) 2026 Luca Hesselbrock
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <MercuryCAN/driver/driverInterface.hpp>

#ifdef __USE_MOCK_DRIVER
    #include <MercuryCAN/driver/mockDriver.hpp>
#else
    #include <MercuryCAN/driver/avrDriver.hpp>
#endif

Can::Driver::DriverInterface* Can::Driver::DriverInterface::getInstance(void) {
#ifdef __USE_MOCK_DRIVER
    static Can::Driver::MockDriver driver;
#else
    static Can::Driver::AvrDriver driver;
#endif
    return &driver;
}