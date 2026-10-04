/**
 * \file    MercuryCAN.hpp
 * \brief   MercuryCAN main header file for the MercuryCAN library. 
 *
 * This file includes all necessary headers for using the MercuryCAN library. 
 * You can include this file in your project to access the full functionality of 
 * the library. Please keep in mind that including this file will also include 
 * all dependencies, so if you only need specific components, consider including 
 * the individual headers instead.
 * 
 * \copyright Copyright (C) 2026 Luca Hesselbrock
 * 
 * SPDX-License-Identifier: Apache-2.0 
 */

#ifndef __MERCURY_CAN_HPP__
#define __MERCURY_CAN_HPP__

#include <MercuryCAN/model/signal.hpp>
#include <MercuryCAN/model/message.hpp>
#include <MercuryCAN/model/cyclicMessage.hpp>

#include <MercuryCAN/driver/driver.hpp>

#include <MercuryCAN/controller/transmitter.hpp>
#include <MercuryCAN/controller/receiver.hpp>

#endif //__MERCURY_CAN_HPP__