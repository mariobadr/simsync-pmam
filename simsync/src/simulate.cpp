#include "simsync/simulate.hpp"

#include "simsync/application.hpp"
#include "simsync/core.hpp"
#include "simsync/reports/report.hpp"
#include "simsync/system.hpp"

#include <cmath>

namespace simsync {

using std::chrono::nanoseconds;

nanoseconds estimate_time(uint64_t const instructions, double const cpi, int64_t const frequency)
{
  auto const cycles = instructions * cpi;
  auto const period = 1 / static_cast<double>(frequency);

  return nanoseconds(static_cast<uint64_t>(std::ceil(1e9 * cycles * period)));
}

uint64_t estimate_instructions(nanoseconds const time, double const cpi, int64_t const frequency)
{
  return static_cast<uint64_t>((time.count() / cpi) * frequency * 1e-9);
}

/**
 * Tracks the progress of threads.
 */
class thread_tracker {
public:
  explicit thread_tracker(std::map<int32_t, thread> const &threads) : m_threads(threads)
  {
    for(auto const &t : threads) {
      m_instructions_remaining[t.first] = t.second.get_computation(0);
      m_current_index[t.first] = 0;
    }
  }

  uint64_t instructions_remaining(int32_t const thread_id) const
  {
    return m_instructions_remaining.at(thread_id);
  }

  size_t current_index(int32_t const thread_id) const
  {
    return m_current_index.at(thread_id);
  }

  /**
   * Progress a thread by a number of instructions.
   *
   * @param thread_id The thread to progress.
   * @param instructions The number of instructions to progress by.
   */
  void progress(int32_t const thread_id, uint64_t const instructions)
  {
    auto &thread_instructions = m_instructions_remaining[thread_id];

    if(thread_instructions >= instructions) {
      thread_instructions -= instructions;
    } else {
      // due to floating point, estimating instructions from time can cause some off-by-a-little errors
      // so we ensure that thread_instructions remains non-negative
      thread_instructions = 0;
    }
  }

  /**
   * Progress a thread to its next synchronization event.
   *
   * @param thread_id The thread to progress.
   */
  void next(int32_t const thread_id)
  {
    auto &index = m_current_index[thread_id];

    // progress by one index
    ++index;
    // reset the instructions remaining to the new index
    m_instructions_remaining[thread_id] = m_threads.at(thread_id).get_computation(index);
  }

private:
  std::map<int32_t, thread> const &m_threads;
  // for each thread, track the instructions remaining until the next event
  std::map<int32_t, uint64_t> m_instructions_remaining;
  // for each thread, track the interval index we are presently at
  std::map<int32_t, size_t> m_current_index;
};

/**
 * Determine the thread that should progress to the next synchronization event.
 *
 * @param threads The current progress of the threads.
 * @param sys Information about the architecture
 * @param[out] elapsed_time The minimum time taken to reach the next event.
 *
 * @return The thread ID that will reach the next event first.
 */
int32_t next_thread(thread_tracker const &threads, system const &sys, nanoseconds *elapsed_time)
{
  *elapsed_time = nanoseconds::max();
  int32_t next_id = *sys.executing_threads().begin();

  for(auto const &t : sys.executing_threads()) {
    auto const instructions = threads.instructions_remaining(t);
    auto const &c = sys.get_thread_core(t);

    auto const thread_time = estimate_time(instructions, c.get_cpi(t), c.frequency());
    if(thread_time < *elapsed_time) {
      next_id = t;
      *elapsed_time = thread_time;
    }
  }

  return next_id;
}

/**
 * Progress all threads forward in time.
 *
 * @param next_thread The thread that will reach its event next.
 * @param time The time the next_thread will take to reach its event.
 * @param threads The current progress of all threads.
 * @param sys Information about the architecture.
 */
void progress(int32_t const next_thread,
    nanoseconds const time,
    thread_tracker &threads,
    system const &sys)
{
  auto executing_threads = sys.executing_threads();
  executing_threads.erase(next_thread);

  for(auto const &t : executing_threads) {
    auto const &c = sys.get_thread_core(t);

    auto const instructions = estimate_instructions(time, c.get_cpi(t), c.frequency());
    threads.progress(t, instructions);
  }

  threads.next(next_thread);
}

nanoseconds
simulate(application const &app, system &sys, std::deque<std::unique_ptr<report>> const &reports)
{
  thread_tracker threads(app.threads());

  // schedule master thread (assumed to have ID 0) for execution
  sys.schedule(0);

  nanoseconds total_time = nanoseconds(0);
  while(!sys.executing_threads().empty()) {
    nanoseconds elapsed_time = nanoseconds(0);

    // determine the next thread that will complete
    auto const current_thread = next_thread(threads, sys, &elapsed_time);
    auto const current_index = threads.current_index(current_thread);
    auto current_event = app.at(current_thread).get_event(current_index);

    // progress time of all currently executing threads
    progress(current_thread, elapsed_time, threads, sys);
    total_time += elapsed_time;

    for(auto & report : reports) {
      report->update(total_time, current_event);
    }

    // update synchronization state
    auto const state_changes = current_event->synchronize();

    // schedule threads based on synchronization state changes
    sys.sleep(state_changes.to_sleep);
    sys.schedule(state_changes.to_wake);
    if(state_changes.finished != -1) {
      sys.erase(state_changes.finished);
    }
  }

  return total_time;
}
}
