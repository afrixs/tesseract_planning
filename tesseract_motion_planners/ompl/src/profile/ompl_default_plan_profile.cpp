/**
 * @file ompl_default_plan_profile.cpp
 * @brief
 *
 * @author Levi Armstrong
 * @date June 18, 2020
 * @version TODO
 * @bug No known bugs
 *
 * @copyright Copyright (c) 2020, Southwest Research Institute
 *
 * @par License
 * Software License Agreement (Apache License)
 * @par
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * http://www.apache.org/licenses/LICENSE-2.0
 * @par
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <tesseract_common/macros.h>
TESSERACT_COMMON_IGNORE_WARNINGS_PUSH
#include <ompl/geometric/SimpleSetup.h>
#include <ompl/base/spaces/RealVectorStateSpace.h>
#include <ompl/tools/multiplan/ParallelPlan.h>
#include <ompl/base/objectives/PathLengthOptimizationObjective.h>
#include <ompl/base/goals/GoalStates.h>
#include <boost/algorithm/string.hpp>
TESSERACT_COMMON_IGNORE_WARNINGS_POP

#include <tesseract_command_language/poly/move_instruction_poly.h>

#include <tesseract_motion_planners/ompl/profile/ompl_default_plan_profile.h>
#include <tesseract_motion_planners/ompl/utils.h>

#include <tesseract_motion_planners/ompl/continuous_motion_validator.h>
#include <tesseract_motion_planners/ompl/discrete_motion_validator.h>
#include <tesseract_motion_planners/ompl/state_collision_validator.h>
#include <tesseract_motion_planners/ompl/compound_state_validator.h>

#include <tesseract_kinematics/core/utils.h>

namespace tesseract_planning
{
OMPLDefaultPlanProfile::OMPLDefaultPlanProfile(const tinyxml2::XMLElement& xml_element)
{
  const tinyxml2::XMLElement* state_space_element = xml_element.FirstChildElement("StateSpace");
  const tinyxml2::XMLElement* planning_time_element = xml_element.FirstChildElement("PlanningTime");
  const tinyxml2::XMLElement* max_solutions_element = xml_element.FirstChildElement("MaxSolutions");
  const tinyxml2::XMLElement* simplify_element = xml_element.FirstChildElement("Simplify");
  const tinyxml2::XMLElement* optimize_element = xml_element.FirstChildElement("Optimize");
  const tinyxml2::XMLElement* planners_element = xml_element.FirstChildElement("Planners");
  //  const tinyxml2::XMLElement* collision_check_element = xml_element.FirstChildElement("CollisionCheck");
  //  const tinyxml2::XMLElement* collision_continuous_element = xml_element.FirstChildElement("CollisionContinuous");
  //  const tinyxml2::XMLElement* collision_safety_margin_element =
  //  xml_element.FirstChildElement("CollisionSafetyMargin");
  //  const tinyxml2::XMLElement* longest_valid_segment_fraction_element =
  //  xml_element.FirstChildElement("LongestValidSegme"
  //                                                                                                     "ntFraction");
  //  const tinyxml2::XMLElement* longest_valid_segment_length_element =
  //  xml_element.FirstChildElement("LongestValidSegment"
  //                                                                                                   "Length");

  tinyxml2::XMLError status{ tinyxml2::XMLError::XML_SUCCESS };

  if (state_space_element != nullptr)
  {
    auto type = static_cast<int>(OMPLProblemStateSpace::REAL_STATE_SPACE);
    status = state_space_element->QueryIntAttribute("type", &type);
    if (status != tinyxml2::XML_SUCCESS)
      throw std::runtime_error("OMPLPlanProfile: Error parsing StateSpace type attribute.");

    state_space = static_cast<OMPLProblemStateSpace>(type);
  }

  if (planning_time_element != nullptr)
  {
    std::string planning_time_string;
    status = tesseract_common::QueryStringText(planning_time_element, planning_time_string);
    if (status != tinyxml2::XML_NO_ATTRIBUTE && status != tinyxml2::XML_SUCCESS)
      throw std::runtime_error("OMPLPlanProfile: Error parsing PlanningTime string");

    if (!tesseract_common::isNumeric(planning_time_string))
      throw std::runtime_error("OMPLPlanProfile: PlanningTime is not a numeric values.");

    tesseract_common::toNumeric<double>(planning_time_string, planning_time);
  }

  if (max_solutions_element != nullptr)
  {
    std::string max_solutions_string;
    status = tesseract_common::QueryStringText(max_solutions_element, max_solutions_string);
    if (status != tinyxml2::XML_NO_ATTRIBUTE && status != tinyxml2::XML_SUCCESS)
      throw std::runtime_error("OMPLPlanProfile: Error parsing MaxSolutions string");

    if (!tesseract_common::isNumeric(max_solutions_string))
      throw std::runtime_error("OMPLPlanProfile: MaxSolutions is not a numeric values.");

    tesseract_common::toNumeric<int>(max_solutions_string, max_solutions);
  }

  if (simplify_element != nullptr)
  {
    status = simplify_element->QueryBoolText(&simplify);
    if (status != tinyxml2::XML_NO_ATTRIBUTE && status != tinyxml2::XML_SUCCESS)
      throw std::runtime_error("OMPLPlanProfile: Error parsing Simplify string");
  }

  if (optimize_element != nullptr)
  {
    status = optimize_element->QueryBoolText(&optimize);
    if (status != tinyxml2::XML_NO_ATTRIBUTE && status != tinyxml2::XML_SUCCESS)
      throw std::runtime_error("OMPLPlanProfile: Error parsing Optimize string");
  }

  if (planners_element != nullptr)
  {
    planners.clear();
    for (const tinyxml2::XMLElement* e = planners_element->FirstChildElement("Planner"); e != nullptr;
         e = e->NextSiblingElement("Planner"))
    {
      int type{ 0 };
      status = e->QueryIntAttribute("type", &type);
      if (status != tinyxml2::XML_SUCCESS)
        throw std::runtime_error("OMPLPlanProfile: Error parsing Planner type attribute.");

      switch (type)
      {
        case static_cast<int>(OMPLPlannerType::SBL):
        {
          SBLConfigurator::ConstPtr ompl_planner = std::make_shared<const SBLConfigurator>(*e);
          planners.push_back(ompl_planner);
          break;
        }
        case static_cast<int>(OMPLPlannerType::EST):
        {
          ESTConfigurator::ConstPtr ompl_planner = std::make_shared<const ESTConfigurator>(*e);
          planners.push_back(ompl_planner);
          break;
        }
        case static_cast<int>(OMPLPlannerType::LBKPIECE1):
        {
          LBKPIECE1Configurator::ConstPtr ompl_planner = std::make_shared<const LBKPIECE1Configurator>(*e);
          planners.push_back(ompl_planner);
          break;
        }
        case static_cast<int>(OMPLPlannerType::BKPIECE1):
        {
          BKPIECE1Configurator::ConstPtr ompl_planner = std::make_shared<const BKPIECE1Configurator>(*e);
          planners.push_back(ompl_planner);
          break;
        }
        case static_cast<int>(OMPLPlannerType::KPIECE1):
        {
          KPIECE1Configurator::ConstPtr ompl_planner = std::make_shared<const KPIECE1Configurator>(*e);
          planners.push_back(ompl_planner);
          break;
        }
        case static_cast<int>(OMPLPlannerType::BiTRRT):
        {
          BiTRRTConfigurator::ConstPtr ompl_planner = std::make_shared<const BiTRRTConfigurator>(*e);
          planners.push_back(ompl_planner);
          break;
        }
        case static_cast<int>(OMPLPlannerType::RRT):
        {
          RRTConfigurator::ConstPtr ompl_planner = std::make_shared<const RRTConfigurator>(*e);
          planners.push_back(ompl_planner);
          break;
        }
        case static_cast<int>(OMPLPlannerType::RRTConnect):
        {
          RRTConnectConfigurator::ConstPtr ompl_planner = std::make_shared<const RRTConnectConfigurator>(*e);
          planners.push_back(ompl_planner);
          break;
        }
        case static_cast<int>(OMPLPlannerType::RRTstar):
        {
          RRTstarConfigurator::ConstPtr ompl_planner = std::make_shared<const RRTstarConfigurator>(*e);
          planners.push_back(ompl_planner);
          break;
        }
        case static_cast<int>(OMPLPlannerType::TRRT):
        {
          TRRTConfigurator::ConstPtr ompl_planner = std::make_shared<const TRRTConfigurator>(*e);
          planners.push_back(ompl_planner);
          break;
        }
        case static_cast<int>(OMPLPlannerType::PRM):
        {
          PRMConfigurator::ConstPtr ompl_planner = std::make_shared<const PRMConfigurator>(*e);
          planners.push_back(ompl_planner);
          break;
        }
        case static_cast<int>(OMPLPlannerType::PRMstar):
        {
          PRMstarConfigurator::ConstPtr ompl_planner = std::make_shared<const PRMstarConfigurator>(*e);
          planners.push_back(ompl_planner);
          break;
        }
        case static_cast<int>(OMPLPlannerType::LazyPRMstar):
        {
          LazyPRMstarConfigurator::ConstPtr ompl_planner = std::make_shared<const LazyPRMstarConfigurator>(*e);
          planners.push_back(ompl_planner);
          break;
        }
        case static_cast<int>(OMPLPlannerType::SPARS):
        {
          SPARSConfigurator::ConstPtr ompl_planner = std::make_shared<const SPARSConfigurator>(*e);
          planners.push_back(ompl_planner);
          break;
        }
        default:
        {
          throw std::runtime_error("Unsupported OMPL Planner type");
        }
      }
    }
  }

  /// @todo Update XML
  //  if (collision_check_element)
  //  {
  //    status = collision_check_element->QueryBoolText(&collision_check);
  //    if (status != tinyxml2::XML_NO_ATTRIBUTE && status != tinyxml2::XML_SUCCESS)
  //      throw std::runtime_error("OMPLPlanProfile: Error parsing CollisionCheck string");
  //  }

  //  if (collision_continuous_element)
  //  {
  //    status = collision_continuous_element->QueryBoolText(&collision_continuous);
  //    if (status != tinyxml2::XML_NO_ATTRIBUTE && status != tinyxml2::XML_SUCCESS)
  //      throw std::runtime_error("OMPLPlanProfile: Error parsing CollisionContinuous string");
  //  }

  /// @todo Update XML
  //  if (collision_safety_margin_element)
  //  {
  //    std::string collision_safety_margin_string;
  //    status = tesseract_common::QueryStringText(collision_safety_margin_element, collision_safety_margin_string);
  //    if (status != tinyxml2::XML_NO_ATTRIBUTE && status != tinyxml2::XML_SUCCESS)
  //      throw std::runtime_error("OMPLPlanProfile: Error parsing CollisionSafetyMargin string");

  //    if (!tesseract_common::isNumeric(collision_safety_margin_string))
  //      throw std::runtime_error("OMPLPlanProfile: CollisionSafetyMargin is not a numeric values.");

  //    tesseract_common::toNumeric<double>(collision_safety_margin_string, collision_safety_margin);
  //  }

  //  if (longest_valid_segment_fraction_element)
  //  {
  //    std::string longest_valid_segment_fraction_string;
  //    status = tesseract_common::QueryStringText(longest_valid_segment_fraction_element,
  //                                               longest_valid_segment_fraction_string);
  //    if (status != tinyxml2::XML_NO_ATTRIBUTE && status != tinyxml2::XML_SUCCESS)
  //      throw std::runtime_error("OMPLPlanProfile: Error parsing LongestValidSegmentFraction string");

  //    if (!tesseract_common::isNumeric(longest_valid_segment_fraction_string))
  //      throw std::runtime_error("OMPLPlanProfile: LongestValidSegmentFraction is not a numeric values.");

  //    tesseract_common::toNumeric<double>(longest_valid_segment_fraction_string, longest_valid_segment_fraction);
  //  }

  //  if (longest_valid_segment_length_element)
  //  {
  //    std::string longest_valid_segment_length_string;
  //    status =
  //        tesseract_common::QueryStringText(longest_valid_segment_length_element,
  //        longest_valid_segment_length_string);
  //    if (status != tinyxml2::XML_NO_ATTRIBUTE && status != tinyxml2::XML_SUCCESS)
  //      throw std::runtime_error("OMPLPlanProfile: Error parsing LongestValidSegmentLength string");

  //    if (!tesseract_common::isNumeric(longest_valid_segment_length_string))
  //      throw std::runtime_error("OMPLPlanProfile: LongestValidSegmentLength is not a numeric values.");

  //    tesseract_common::toNumeric<double>(longest_valid_segment_length_string, longest_valid_segment_length);
  //  }
}

void OMPLDefaultPlanProfile::setup(OMPLProblem& prob) const
{
  prob.planners = planners;
  prob.planning_time = planning_time;
  prob.max_solutions = max_solutions;
  prob.simplify = simplify;
  prob.optimize = optimize;

  prob.contact_checker->applyContactManagerConfig(collision_check_config.contact_manager_config);

  std::vector<std::string> joint_names = prob.manip->getJointNames();
  auto dof = static_cast<unsigned>(prob.manip->numJoints());
  auto limits = prob.manip->getLimits().joint_limits;

  if (state_space == OMPLProblemStateSpace::REAL_STATE_SPACE)
    prob.extractor = [dof](const ompl::base::State* state) -> Eigen::Map<Eigen::VectorXd> {
      return tesseract_planning::RealVectorStateSpaceExtractor(state, dof);
    };

#ifndef OMPL_LESS_1_4_0
  else if (state_space == OMPLProblemStateSpace::REAL_CONSTRAINTED_STATE_SPACE)
    prob.extractor = tesseract_planning::ConstrainedStateSpaceExtractor;
#endif
  else
    throw std::runtime_error("OMPLMotionPlannerDefaultConfig: Unsupported configuration!");

  if (prob.state_space == OMPLProblemStateSpace::REAL_STATE_SPACE)
  {
    // Construct the OMPL state space for this manipulator
    ompl::base::StateSpacePtr state_space_ptr;

    auto rss = std::make_shared<ompl::base::RealVectorStateSpace>();
    for (unsigned i = 0; i < dof; ++i)
      rss->addDimension(joint_names[i], limits(i, 0), limits(i, 1));

    if (state_sampler_allocator)
    {
      rss->setStateSamplerAllocator(
          [=](const ompl::base::StateSpace* space) { return state_sampler_allocator(space, prob); });
    }
    else
    {
      Eigen::VectorXd weights = Eigen::VectorXd::Ones(dof);
      rss->setStateSamplerAllocator(
          [weights, limits](const ompl::base::StateSpace* state_space) -> ompl::base::StateSamplerPtr {
            return allocWeightedRealVectorStateSampler(state_space, weights, limits);
          });
    }

    state_space_ptr = rss;

    // Setup Longest Valid Segment
    processLongestValidSegment(state_space_ptr, collision_check_config);

    // Create Simple Setup from state space
    prob.simple_setup = std::make_shared<ompl::geometric::SimpleSetup>(state_space_ptr);

    // Setup state checking functionality
    ompl::base::StateValidityCheckerPtr svc_without_collision = processStateValidator(prob);

    // Setup motion validation (i.e. collision checking)
    processMotionValidator(prob, svc_without_collision);

    // make sure the planners run until the time limit, and get the best possible solution
    processOptimizationObjective(prob);
  }
}

void OMPLDefaultPlanProfile::applyGoalStates(OMPLProblem& prob,
                                             const Eigen::Isometry3d& cartesian_waypoint,
                                             const MoveInstructionPoly& parent_instruction,
                                             const tesseract_common::ManipulatorInfo& manip_info,
                                             const std::vector<std::string>& /*active_links*/,
                                             int /*index*/) const
{
  const auto dof = prob.manip->numJoints();
  tesseract_common::KinematicLimits limits = prob.manip->getLimits();

  assert(!(manip_info.empty() && parent_instruction.getManipulatorInfo().empty()));
  tesseract_common::ManipulatorInfo mi = manip_info.getCombined(parent_instruction.getManipulatorInfo());

  if (mi.manipulator.empty())
    throw std::runtime_error("OMPL, manipulator is empty!");

  if (mi.tcp_frame.empty())
    throw std::runtime_error("OMPL, tcp_frame is empty!");

  if (mi.working_frame.empty())
    throw std::runtime_error("OMPL, working_frame is empty!");

  Eigen::Isometry3d tcp_offset = prob.env->findTCPOffset(mi);

  Eigen::Isometry3d tcp_frame_cwp = cartesian_waypoint * tcp_offset.inverse();

  if (prob.state_space == OMPLProblemStateSpace::REAL_STATE_SPACE)
  {
    /** @todo Need to add Descartes pose sample to ompl profile */
    tesseract_kinematics::KinGroupIKInput ik_input(tcp_frame_cwp, mi.working_frame, mi.tcp_frame);
    tesseract_kinematics::IKSolutions joint_solutions =
        std::dynamic_pointer_cast<const tesseract_kinematics::KinematicGroup>(prob.manip)
            ->calcInvKin({ ik_input }, Eigen::VectorXd::Zero(dof));
    auto goal_states = std::make_shared<ompl::base::GoalStates>(prob.simple_setup->getSpaceInformation());
    std::vector<tesseract_collision::ContactResultMap> contact_map_vec(
        static_cast<std::size_t>(joint_solutions.size()));

    for (std::size_t i = 0; i < joint_solutions.size(); ++i)
    {
      Eigen::VectorXd& solution = joint_solutions[i];

      // Check limits
      if (tesseract_common::satisfiesPositionLimits<double>(solution, limits.joint_limits))
      {
        tesseract_common::enforcePositionLimits<double>(solution, limits.joint_limits);
      }
      else
      {
        CONSOLE_BRIDGE_logDebug("In OMPLDefaultPlanProfile: Goal state has invalid bounds");
      }

      // Get discrete contact manager for testing provided start and end position
      // This is required because collision checking happens in motion validators now
      // instead of the isValid function to avoid unnecessary collision checks.
      if (!checkStateInCollision(prob, solution, contact_map_vec[i]))
      {
        {
          ompl::base::ScopedState<> goal_state(prob.simple_setup->getStateSpace());
          for (unsigned j = 0; j < dof; ++j)
            goal_state[j] = solution[static_cast<Eigen::Index>(j)];

          goal_states->addState(goal_state);
        }

        auto redundant_solutions = tesseract_kinematics::getRedundantSolutions<double>(
            solution, limits.joint_limits, prob.manip->getRedundancyCapableJointIndices());
        for (const auto& rs : redundant_solutions)
        {
          ompl::base::ScopedState<> goal_state(prob.simple_setup->getStateSpace());
          for (unsigned j = 0; j < dof; ++j)
            goal_state[j] = rs[static_cast<Eigen::Index>(j)];

          goal_states->addState(goal_state);
        }
      }
    }

    if (!goal_states->hasStates())
    {
      for (std::size_t i = 0; i < contact_map_vec.size(); i++)
        for (const auto& contact_vec : contact_map_vec[i])
          for (const auto& contact : contact_vec.second)
            CONSOLE_BRIDGE_logError(("Solution: " + std::to_string(i) + "  Links: " + contact.link_names[0] + ", " +
                                     contact.link_names[1] + "  Distance: " + std::to_string(contact.distance))
                                        .c_str());
      throw std::runtime_error("In OMPLDefaultPlanProfile: All goal states are either in collision or outside limits");
    }
    prob.simple_setup->setGoal(goal_states);
  }
}

void OMPLDefaultPlanProfile::applyGoalStates(OMPLProblem& prob,
                                             const Eigen::VectorXd& joint_waypoint,
                                             const MoveInstructionPoly& /*parent_instruction*/,
                                             const tesseract_common::ManipulatorInfo& /*manip_info*/,
                                             const std::vector<std::string>& /*active_links*/,
                                             int /*index*/) const
{
  const auto dof = prob.manip->numJoints();
  tesseract_common::KinematicLimits limits = prob.manip->getLimits();
  if (prob.state_space == OMPLProblemStateSpace::REAL_STATE_SPACE)
  {
    // Check limits
    Eigen::VectorXd solution = joint_waypoint;
    if (tesseract_common::satisfiesPositionLimits<double>(solution, limits.joint_limits))
    {
      tesseract_common::enforcePositionLimits<double>(solution, limits.joint_limits);
    }
    else
    {
      CONSOLE_BRIDGE_logDebug("In OMPLDefaultPlanProfile: Goal state has invalid bounds");
    }

    // Get discrete contact manager for testing provided start and end position
    // This is required because collision checking happens in motion validators now
    // instead of the isValid function to avoid unnecessary collision checks.
    tesseract_collision::ContactResultMap contact_map;
    if (checkStateInCollision(prob, solution, contact_map))
    {
      CONSOLE_BRIDGE_logError("In OMPLDefaultPlanProfile: Goal state is in collision");
      for (const auto& contact_vec : contact_map)
        for (const auto& contact : contact_vec.second)
          CONSOLE_BRIDGE_logError(("Links: " + contact.link_names[0] + ", " + contact.link_names[1] +
                                   "  Distance: " + std::to_string(contact.distance))
                                      .c_str());
    }

    ompl::base::ScopedState<> goal_state(prob.simple_setup->getStateSpace());
    for (unsigned i = 0; i < dof; ++i)
      goal_state[i] = joint_waypoint[i];

    prob.simple_setup->setGoalState(goal_state);
  }
}

void OMPLDefaultPlanProfile::applyStartStates(OMPLProblem& prob,
                                              const Eigen::Isometry3d& cartesian_waypoint,
                                              const MoveInstructionPoly& parent_instruction,
                                              const tesseract_common::ManipulatorInfo& manip_info,
                                              const std::vector<std::string>& /*active_links*/,
                                              int /*index*/) const
{
  const auto dof = prob.manip->numJoints();
  tesseract_common::KinematicLimits limits = prob.manip->getLimits();

  assert(!(manip_info.empty() && parent_instruction.getManipulatorInfo().empty()));
  tesseract_common::ManipulatorInfo mi = manip_info.getCombined(parent_instruction.getManipulatorInfo());

  if (mi.manipulator.empty())
    throw std::runtime_error("OMPL, manipulator is empty!");

  if (mi.tcp_frame.empty())
    throw std::runtime_error("OMPL, tcp_frame is empty!");

  if (mi.working_frame.empty())
    throw std::runtime_error("OMPL, working_frame is empty!");

  Eigen::Isometry3d tcp = prob.env->findTCPOffset(mi);

  Eigen::Isometry3d tcp_frame_cwp = cartesian_waypoint * tcp.inverse();

  if (prob.state_space == OMPLProblemStateSpace::REAL_STATE_SPACE)
  {
    /** @todo Need to add Descartes pose sampler to ompl profile */
    /** @todo Need to also provide the seed instruction to use here */
    tesseract_kinematics::KinGroupIKInput ik_input(tcp_frame_cwp, mi.working_frame, mi.tcp_frame);
    tesseract_kinematics::IKSolutions joint_solutions =
        std::dynamic_pointer_cast<const tesseract_kinematics::KinematicGroup>(prob.manip)
            ->calcInvKin({ ik_input }, Eigen::VectorXd::Zero(dof));
    bool found_start_state = false;
    std::vector<tesseract_collision::ContactResultMap> contact_map_vec(joint_solutions.size());

    for (std::size_t i = 0; i < joint_solutions.size(); ++i)
    {
      Eigen::VectorXd& solution = joint_solutions[i];

      // Check limits
      if (tesseract_common::satisfiesPositionLimits<double>(solution, limits.joint_limits))
      {
        tesseract_common::enforcePositionLimits<double>(solution, limits.joint_limits);
      }
      else
      {
        CONSOLE_BRIDGE_logDebug("In OMPLDefaultPlanProfile: Start state has invalid bounds");
      }

      // Get discrete contact manager for testing provided start and end position
      // This is required because collision checking happens in motion validators now
      // instead of the isValid function to avoid unnecessary collision checks.
      if (!checkStateInCollision(prob, solution, contact_map_vec[i]))
      {
        found_start_state = true;
        {
          ompl::base::ScopedState<> start_state(prob.simple_setup->getStateSpace());
          for (unsigned j = 0; j < dof; ++j)
            start_state[j] = solution[static_cast<Eigen::Index>(j)];

          prob.simple_setup->addStartState(start_state);
        }

        auto redundant_solutions = tesseract_kinematics::getRedundantSolutions<double>(
            solution, limits.joint_limits, prob.manip->getRedundancyCapableJointIndices());
        for (const auto& rs : redundant_solutions)
        {
          ompl::base::ScopedState<> start_state(prob.simple_setup->getStateSpace());
          for (unsigned j = 0; j < dof; ++j)
            start_state[j] = rs[static_cast<Eigen::Index>(j)];

          prob.simple_setup->addStartState(start_state);
        }
      }
    }

    if (!found_start_state)
    {
      for (std::size_t i = 0; i < contact_map_vec.size(); i++)
        for (const auto& contact_vec : contact_map_vec[i])
          for (const auto& contact : contact_vec.second)
            CONSOLE_BRIDGE_logError(("Solution: " + std::to_string(i) + "  Links: " + contact.link_names[0] + ", " +
                                     contact.link_names[1] + "  Distance: " + std::to_string(contact.distance))
                                        .c_str());
      throw std::runtime_error("In OMPLPlannerFreespaceConfig: All start states are either in collision or outside "
                               "limits");
    }
  }
}

void OMPLDefaultPlanProfile::applyStartStates(OMPLProblem& prob,
                                              const Eigen::VectorXd& joint_waypoint,
                                              const MoveInstructionPoly& /*parent_instruction*/,
                                              const tesseract_common::ManipulatorInfo& /*manip_info*/,
                                              const std::vector<std::string>& /*active_links*/,
                                              int /*index*/) const
{
  const auto dof = prob.manip->numJoints();
  tesseract_common::KinematicLimits limits = prob.manip->getLimits();

  if (prob.state_space == OMPLProblemStateSpace::REAL_STATE_SPACE)
  {
    Eigen::VectorXd solution = joint_waypoint;

    if (tesseract_common::satisfiesPositionLimits<double>(solution, limits.joint_limits))
    {
      tesseract_common::enforcePositionLimits<double>(solution, limits.joint_limits);
    }
    else
    {
      CONSOLE_BRIDGE_logDebug("In OMPLDefaultPlanProfile: Start state is outside limits");
    }

    // Get discrete contact manager for testing provided start and end position
    // This is required because collision checking happens in motion validators now
    // instead of the isValid function to avoid unnecessary collision checks.
    tesseract_collision::ContactResultMap contact_map;
    if (checkStateInCollision(prob, joint_waypoint, contact_map))
    {
      CONSOLE_BRIDGE_logError("In OMPLPlannerFreespaceConfig: Start state is in collision");
      for (const auto& contact_vec : contact_map)
        for (const auto& contact : contact_vec.second)
          CONSOLE_BRIDGE_logError(("Links: " + contact.link_names[0] + ", " + contact.link_names[1] +
                                   "  Distance: " + std::to_string(contact.distance))
                                      .c_str());
    }

    ompl::base::ScopedState<> start_state(prob.simple_setup->getStateSpace());
    for (unsigned i = 0; i < dof; ++i)
      start_state[i] = joint_waypoint[i];

    prob.simple_setup->addStartState(start_state);
  }
}

tinyxml2::XMLElement* OMPLDefaultPlanProfile::toXML(tinyxml2::XMLDocument& doc) const
{
  Eigen::IOFormat eigen_format(Eigen::StreamPrecision, Eigen::DontAlignCols, " ", " ");

  tinyxml2::XMLElement* xml_planner = doc.NewElement("Planner");
  xml_planner->SetAttribute("type", std::to_string(2).c_str());

  tinyxml2::XMLElement* xml_ompl = doc.NewElement("OMPLPlanProfile");

  tinyxml2::XMLElement* xml_ompl_planners = doc.NewElement("Planners");

  for (const auto& planner : planners)
  {
    tinyxml2::XMLElement* xml_ompl_planner = doc.NewElement("Planner");
    xml_ompl_planner->SetAttribute("type", std::to_string(static_cast<int>(planner->getType())).c_str());
    tinyxml2::XMLElement* xml_planner = planner->toXML(doc);
    xml_ompl_planner->InsertEndChild(xml_planner);
    xml_ompl_planners->InsertEndChild(xml_ompl_planner);
  }

  xml_ompl->InsertEndChild(xml_ompl_planners);

  tinyxml2::XMLElement* xml_state_space = doc.NewElement("StateSpace");
  xml_state_space->SetAttribute("type", std::to_string(static_cast<int>(state_space)).c_str());
  xml_ompl->InsertEndChild(xml_state_space);

  tinyxml2::XMLElement* xml_planning_time = doc.NewElement("PlanningTime");
  xml_planning_time->SetText(planning_time);
  xml_ompl->InsertEndChild(xml_planning_time);

  tinyxml2::XMLElement* xml_max_solutions = doc.NewElement("MaxSolutions");
  xml_max_solutions->SetText(max_solutions);
  xml_ompl->InsertEndChild(xml_max_solutions);

  tinyxml2::XMLElement* xml_simplify = doc.NewElement("Simplify");
  xml_simplify->SetText(simplify);
  xml_ompl->InsertEndChild(xml_simplify);

  tinyxml2::XMLElement* xml_optimize = doc.NewElement("Optimize");
  xml_optimize->SetText(optimize);
  xml_ompl->InsertEndChild(xml_optimize);

  /// @todo Update XML
  //  tinyxml2::XMLElement* xml_collision_check = doc.NewElement("CollisionCheck");
  //  xml_collision_check->SetText(collision_check);
  //  xml_ompl->InsertEndChild(xml_collision_check);

  //  tinyxml2::XMLElement* xml_collision_continuous = doc.NewElement("CollisionContinuous");
  //  xml_collision_continuous->SetText(collision_continuous);
  //  xml_ompl->InsertEndChild(xml_collision_continuous);

  //  tinyxml2::XMLElement* xml_collision_safety_margin = doc.NewElement("CollisionSafetyMargin");
  //  xml_collision_safety_margin->SetText(collision_safety_margin);
  //  xml_ompl->InsertEndChild(xml_collision_safety_margin);

  //  tinyxml2::XMLElement* xml_long_valid_seg_frac = doc.NewElement("LongestValidSegmentFraction");
  //  xml_long_valid_seg_frac->SetText(longest_valid_segment_fraction);
  //  xml_ompl->InsertEndChild(xml_long_valid_seg_frac);

  //  tinyxml2::XMLElement* xml_long_valid_seg_len = doc.NewElement("LongestValidSegmentLength");
  //  xml_long_valid_seg_len->SetText(longest_valid_segment_length);
  //  xml_ompl->InsertEndChild(xml_long_valid_seg_len);

  // TODO: Add plugins for state_sampler_allocator, optimization_objective_allocator, svc_allocator,
  // mv_allocator

  xml_planner->InsertEndChild(xml_ompl);

  return xml_planner;
}

ompl::base::StateValidityCheckerPtr OMPLDefaultPlanProfile::processStateValidator(OMPLProblem& prob) const
{
  ompl::base::StateValidityCheckerPtr svc_without_collision;
  auto csvc = std::make_shared<CompoundStateValidator>();
  if (svc_allocator != nullptr)
  {
    svc_without_collision = svc_allocator(prob.simple_setup->getSpaceInformation(), prob);
    csvc->addStateValidator(svc_without_collision);
  }

  if (collision_check_config.type == tesseract_collision::CollisionEvaluatorType::DISCRETE ||
      collision_check_config.type == tesseract_collision::CollisionEvaluatorType::LVS_DISCRETE)
  {
    auto svc = std::make_shared<StateCollisionValidator>(
        prob.simple_setup->getSpaceInformation(), *prob.env, prob.manip, collision_check_config, prob.extractor);
    csvc->addStateValidator(svc);
  }
  prob.simple_setup->setStateValidityChecker(csvc);

  return svc_without_collision;
}

void OMPLDefaultPlanProfile::processMotionValidator(
    OMPLProblem& prob,
    const ompl::base::StateValidityCheckerPtr& svc_without_collision) const
{
  if (mv_allocator != nullptr)
  {
    auto mv = mv_allocator(prob.simple_setup->getSpaceInformation(), prob);
    prob.simple_setup->getSpaceInformation()->setMotionValidator(mv);
  }
  else
  {
    if (collision_check_config.type != tesseract_collision::CollisionEvaluatorType::NONE)
    {
      ompl::base::MotionValidatorPtr mv;
      if (collision_check_config.type == tesseract_collision::CollisionEvaluatorType::CONTINUOUS ||
          collision_check_config.type == tesseract_collision::CollisionEvaluatorType::LVS_CONTINUOUS)
      {
        mv = std::make_shared<ContinuousMotionValidator>(prob.simple_setup->getSpaceInformation(),
                                                         svc_without_collision,
                                                         *prob.env,
                                                         prob.manip,
                                                         collision_check_config,
                                                         prob.extractor);
      }
      else
      {
        // Collision checking is preformed using the state validator which this calls.
        mv = std::make_shared<DiscreteMotionValidator>(prob.simple_setup->getSpaceInformation());
      }
      prob.simple_setup->getSpaceInformation()->setMotionValidator(mv);
    }
  }
}

void OMPLDefaultPlanProfile::processOptimizationObjective(OMPLProblem& prob) const
{
  if (optimization_objective_allocator)
  {
    prob.simple_setup->getProblemDefinition()->setOptimizationObjective(
        optimization_objective_allocator(prob.simple_setup->getSpaceInformation(), prob));
  }
  else if (prob.optimize)
  {
    // Add default optimization function to minimize path length
    prob.simple_setup->getProblemDefinition()->setOptimizationObjective(
        std::make_shared<ompl::base::PathLengthOptimizationObjective>(prob.simple_setup->getSpaceInformation()));
  }
}

}  // namespace tesseract_planning
