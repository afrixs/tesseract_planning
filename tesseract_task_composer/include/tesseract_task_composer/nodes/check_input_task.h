/**
 * @file check_input_task.h
 * @brief Task for checking input data structure
 *
 * @author Levi Armstrong
 * @date November 2. 2021
 * @version TODO
 * @bug No known bugs
 *
 * @copyright Copyright (c) 2021, Southwest Research Institute
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
#ifndef TESSERACT_TASK_COMPOSER_CHECK_INPUT_TASK_H
#define TESSERACT_TASK_COMPOSER_CHECK_INPUT_TASK_H

#include <tesseract_common/macros.h>
TESSERACT_COMMON_IGNORE_WARNINGS_PUSH
#include <boost/serialization/access.hpp>
#include <functional>
TESSERACT_COMMON_IGNORE_WARNINGS_POP

#include <tesseract_task_composer/task_composer_task.h>

namespace tesseract_planning
{
class TaskComposerPluginFactory;
class CheckInputTask : public TaskComposerTask
{
public:
  using Ptr = std::shared_ptr<CheckInputTask>;
  using ConstPtr = std::shared_ptr<const CheckInputTask>;
  using UPtr = std::unique_ptr<CheckInputTask>;
  using ConstUPtr = std::unique_ptr<const CheckInputTask>;

  CheckInputTask();
  explicit CheckInputTask(std::string name, std::string input_key, bool is_conditional = true);
  explicit CheckInputTask(std::string name, std::vector<std::string> input_keys, bool is_conditional = true);
  explicit CheckInputTask(std::string name, const YAML::Node& config, const TaskComposerPluginFactory& plugin_factory);

  ~CheckInputTask() override = default;
  CheckInputTask(const CheckInputTask&) = delete;
  CheckInputTask& operator=(const CheckInputTask&) = delete;
  CheckInputTask(CheckInputTask&&) = delete;
  CheckInputTask& operator=(CheckInputTask&&) = delete;

  bool operator==(const CheckInputTask& rhs) const;
  bool operator!=(const CheckInputTask& rhs) const;

protected:
  friend struct tesseract_common::Serialization;
  friend class boost::serialization::access;
  template <class Archive>
  void serialize(Archive& ar, const unsigned int version);  // NOLINT

  TaskComposerNodeInfo::UPtr runImpl(TaskComposerInput& input,
                                     OptionalTaskComposerExecutor executor = std::nullopt) const override final;
};

}  // namespace tesseract_planning

#include <boost/serialization/export.hpp>
BOOST_CLASS_EXPORT_KEY2(tesseract_planning::CheckInputTask, "CheckInputTask")

#endif  // TESSERACT_TASK_COMPOSER_CHECK_INPUT_TASK_H
