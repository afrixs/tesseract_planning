/**
 * @file raster_waad_taskflow.h
 * @brief Plans raster paths with approach and departure
 *
 * @author Levi Armstrong
 * @date August 28, 2020
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
#ifndef TESSERACT_PROCESS_MANAGERS_RASTER_WAAD_TASKFLOW_H
#define TESSERACT_PROCESS_MANAGERS_RASTER_WAAD_TASKFLOW_H

#include <tesseract_common/macros.h>
TESSERACT_COMMON_IGNORE_WARNINGS_PUSH
#include <functional>
#include <vector>
#include <array>
#include <thread>
#include <taskflow/taskflow.hpp>
TESSERACT_COMMON_IGNORE_WARNINGS_POP

#include <tesseract_process_managers/taskflow_generator.h>

namespace tesseract_planning
{
/**
 * @brief This class provides a process manager for a raster process.
 *
 * Given a ProcessInput in the correct format, it handles the creation of the process dependencies and uses Taskflow to
 * execute them efficiently in a parallel based on those dependencies.
 *
 * The required format is below. Note that a transition is planned from both the start and end of each raster to allow
 * for skipping of rasters without replanning. This logic must be handled in the execute process.
 *
 * Composite
 * {
 *   Composite - from start
 *   Composite - Raster segment (e.g. approach, raster, departure)
 *   {
 *     Composite approach
 *     Composite process
 *     Composite departure
 *   }
 *   Composite - Transitions
 *   Composite - Raster segment
 *   {
 *     Composite approach
 *     Composite process
 *     Composite departure
 *   }
 *   Composite - Transitions
 *   Composite - Raster segment
 *   {
 *     Composite approach
 *     Composite process
 *     Composite departure
 *   }
 *   Composite - to end
 * }
 */
class RasterWAADTaskflow : public TaskflowGenerator
{
public:
  using UPtr = std::unique_ptr<RasterWAADTaskflow>;

  RasterWAADTaskflow(TaskflowGenerator::UPtr freespace_taskflow_generator,
                     TaskflowGenerator::UPtr transition_taskflow_generator,
                     TaskflowGenerator::UPtr raster_taskflow_generator,
                     std::string name = "RasterWAADTaskflow");
  ~RasterWAADTaskflow() override = default;
  RasterWAADTaskflow(const RasterWAADTaskflow&) = delete;
  RasterWAADTaskflow& operator=(const RasterWAADTaskflow&) = delete;
  RasterWAADTaskflow(RasterWAADTaskflow&&) = delete;
  RasterWAADTaskflow& operator=(RasterWAADTaskflow&&) = delete;

  const std::string& getName() const override;

  TaskflowContainer generateTaskflow(ProcessInput input,
                                     std::function<void()> done_cb,
                                     std::function<void()> error_cb) override;

private:
  TaskflowGenerator::UPtr freespace_taskflow_generator_;
  TaskflowGenerator::UPtr transition_taskflow_generator_;
  TaskflowGenerator::UPtr raster_taskflow_generator_;
  std::string name_;

  /**
   * @brief Checks that the ProcessInput is in the correct format.
   * @param input ProcessInput to be checked
   * @return True if in the correct format
   */
  bool checkProcessInput(const ProcessInput& input) const;
};

}  // namespace tesseract_planning

#endif  // TESSERACT_PROCESS_MANAGERS_RASTER_WAAD_TASKFLOW_H
