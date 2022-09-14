/**
 * @file task_composer_server.h
 * @brief A task server
 *
 * @author Levi Armstrong
 * @date August 27, 2022
 * @version TODO
 * @bug No known bugs
 *
 * @copyright Copyright (c) 2022, Levi Armstrong
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
#ifndef TESSERACT_TASK_COMPOSER_TASK_COMPOSER_SERVER_H
#define TESSERACT_TASK_COMPOSER_TASK_COMPOSER_SERVER_H

#include <tesseract_common/macros.h>
TESSERACT_COMMON_IGNORE_WARNINGS_PUSH
#include <memory>
TESSERACT_COMMON_IGNORE_WARNINGS_POP

#include <tesseract_task_composer/task_composer_executor.h>

namespace tesseract_planning
{
class TaskComposerServer
{
public:
  using Ptr = std::shared_ptr<TaskComposerServer>;
  using ConstPtr = std::shared_ptr<const TaskComposerServer>;
  using UPtr = std::unique_ptr<TaskComposerServer>;
  using ConstUPtr = std::unique_ptr<const TaskComposerServer>;

  /**
   * @brief Add a executors (thread pool)
   * @param executor The executor to add
   */
  void addExecutor(const TaskComposerExecutor::Ptr& executor);

  /**
   * @brief Get an executor by name
   * @param name The name of the executor
   * @return The exector, if not found nullptr is returned
   */
  TaskComposerExecutor::Ptr getExecutor(const std::string& name);

  /**
   * @brief Check if executors (thread pool) exists with the provided name
   * @param name The name to search
   * @return True if it exists, otherwise false
   */
  bool hasExecutor(const std::string& name) const;

  /**
   * @brief Get the available executors (thread pool) names
   * @return A vector of names
   */
  std::vector<std::string> getAvailableExecutors() const;

  /**
   * @brief Add a task
   * @param task The task to add
   */
  void addTask(TaskComposerNode::UPtr task);

  /**
   * @brief Check if task exists with the provided name
   * @param name The name to search
   * @return True if it exists, otherwise false
   */
  bool hasTask(const std::string& name) const;

  /**
   * @brief Get the available task names
   * @return A vector of names
   */
  std::vector<std::string> getAvailableTask() const;

  /**
   * @brief Execute the provided task graph
   * @param task_input The task input provided to every task
   * @param name The name of the executor to use
   * @return The future associated with execution
   */
  TaskComposerFuture::UPtr run(TaskComposerInput& task_input, const std::string& name);

  /**
   * @brief Execute the provided node
   * @details It will call one of the methods below based on the node type
   * @param node The node to execute
   * @param task_input The task input provided to every task
   * @return The future associated with execution
   */
  TaskComposerFuture::UPtr run(const TaskComposerNode& node, TaskComposerInput& task_input, const std::string& name);

  /**
   * @brief Execute the provided task graph
   * @param task_graph The task graph to execute
   * @param task_input The task input provided to every task
   * @return The future associated with execution
   */
  TaskComposerFuture::UPtr run(const TaskComposerGraph& task_graph,
                               TaskComposerInput& task_input,
                               const std::string& name);

  /**
   * @brief Execute the provided task
   * @param task_graph The task to execute
   * @param task_input The task input provided to task
   * @return The future associated with execution
   */
  TaskComposerFuture::UPtr run(const TaskComposerTask& task, TaskComposerInput& task_input, const std::string& name);

  /** @brief Queries the number of workers (example: number of threads) */
  long getWorkerCount(const std::string& name) const;

  /** @brief Queries the number of running tasks at the time of this call */
  long getTaskCount(const std::string& name) const;

protected:
  std::unordered_map<std::string, TaskComposerExecutor::Ptr> executors_;
  std::unordered_map<std::string, TaskComposerNode::UPtr> tasks_;
};
}  // namespace tesseract_planning

#endif  // TASK_COMPOSER_SERVER_H