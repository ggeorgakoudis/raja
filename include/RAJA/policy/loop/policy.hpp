/*!
 ******************************************************************************
 *
 * \file
 *
 * \brief   Header file containing RAJA sequential policy definitions.
 *
 ******************************************************************************
 */

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
// Copyright (c) 2016-19, Lawrence Livermore National Security, LLC
// and RAJA project contributors. See the RAJA/COPYRIGHT file for details.
//
// SPDX-License-Identifier: (BSD-3-Clause)
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

#ifndef policy_loop_HPP
#define policy_loop_HPP

#include "RAJA/policy/PolicyBase.hpp"

#include "RAJA/policy/sequential/policy.hpp"

namespace RAJA
{
namespace policy
{
namespace loop
{

//
//////////////////////////////////////////////////////////////////////
//
// Execution policies
//
//////////////////////////////////////////////////////////////////////
//

///
/// Segment execution policies
///

struct loop_exec : make_policy_pattern_launch_platform_t<Policy::loop,
                                                         Pattern::forall,
                                                         Launch::undefined,
                                                         Platform::host> {
};

///
/// Index set segment iteration policies
///
using loop_segit = loop_exec;

///
///////////////////////////////////////////////////////////////////////
///
/// Reduction execution policies
///
///////////////////////////////////////////////////////////////////////
///
using loop_reduce = seq_reduce;

}  // end namespace loop

}  // end namespace policy

using policy::loop::loop_exec;
using policy::loop::loop_reduce;
using policy::loop::loop_segit;

}  // namespace RAJA

#endif
