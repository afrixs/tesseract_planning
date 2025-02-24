/**
 * @file taskflow_task_composer_future.h
 * @brief A taskflow task composer future implementation
 *
 * @author Levi Armstrong
 * @date August 18, 2020
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
#ifndef TESSERACT_TASK_COMPOSER_TASKFLOW_TASK_COMPOSER_FUTURE_H
#define TESSERACT_TASK_COMPOSER_TASKFLOW_TASK_COMPOSER_FUTURE_H

#include <tesseract_task_composer/task_composer_future.h>

namespace tf
{
class Taskflow;
}

namespace tesseract_planning
{
struct TaskComposerTaskflowContainer;
class TaskflowTaskComposerFuture : public TaskComposerFuture
{
public:
  TaskflowTaskComposerFuture() = default;
  TaskflowTaskComposerFuture(std::shared_future<void> future,
                             std::shared_ptr<const std::vector<std::unique_ptr<tf::Taskflow>>> container);
  ~TaskflowTaskComposerFuture() override;
  TaskflowTaskComposerFuture(const TaskflowTaskComposerFuture&) = default;
  TaskflowTaskComposerFuture& operator=(const TaskflowTaskComposerFuture&) = default;
  TaskflowTaskComposerFuture(TaskflowTaskComposerFuture&&) = default;
  TaskflowTaskComposerFuture& operator=(TaskflowTaskComposerFuture&&) = default;

  void clear() override final;

  bool valid() const override final;

  bool ready() const override final;

  void wait() const override final;

  std::future_status waitFor(const std::chrono::duration<double>& duration) const override final;

  std::future_status
  waitUntil(const std::chrono::time_point<std::chrono::high_resolution_clock>& abs) const override final;

  TaskComposerFuture::UPtr copy() const override final;

  bool operator==(const TaskflowTaskComposerFuture& rhs) const;
  bool operator!=(const TaskflowTaskComposerFuture& rhs) const;

private:
  friend struct tesseract_common::Serialization;
  friend class boost::serialization::access;
  template <class Archive>
  void serialize(Archive& ar, const unsigned int version);  // NOLINT

  /** @brief This is the future return from taskflow executor.run */
  std::shared_future<void> future_;

  /** @brief Hold objects that must not go out of scope during execution */
  std::shared_ptr<const std::vector<std::unique_ptr<tf::Taskflow>>> container_;
};
}  // namespace tesseract_planning

#include <boost/serialization/export.hpp>
#include <boost/serialization/tracking.hpp>
BOOST_CLASS_EXPORT_KEY2(tesseract_planning::TaskflowTaskComposerFuture, "TaskflowTaskComposerFuture")
#endif  // TESSERACT_TASK_COMPOSER_TASKFLOW_TASK_COMPOSER_FUTURE_H
