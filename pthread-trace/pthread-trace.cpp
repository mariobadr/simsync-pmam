#include "pin.H"

#include <iostream>
#include <fstream>

/**
 * A list of all the pthread function calls for synchronization.
 */
static const char *events[] = {"pthread_cond_init", "pthread_mutex_init", "pthread_rwlock_init",
    "pthread_spin_init", "pthread_barrier_destroy", "pthread_cond_destroy", "pthread_mutex_destroy",
    "pthread_rwlock_destroy", "pthread_spin_destroy", "pthread_exit", "pthread_barrier_wait",
    "pthread_cond_broadcast", "pthread_cond_signal", "pthread_cond_timedwait", "pthread_cond_wait",
    "pthread_mutex_lock", "pthread_mutex_unlock", "pthread_mutex_timedlock",
    "pthread_mutex_trylock", "pthread_rwlock_wrlock", "pthread_rwlock_timedwrlock",
    "pthread_rwlock_trywrlock", "pthread_rwlock_rdlock", "pthread_rwlock_timedrdlock",
    "pthread_rwlock_tryrdlock", "pthread_rwlock_unlock", "pthread_spin_lock", "pthread_spin_unlock",
    "pthread_spin_trylock", "thread_start", "thread_finish", "pthread_create", "pthread_join"};

/**
 * The number of pthread synchronization function calls.
 */
#define SYNC_CALLS_SIZE 29

/**
 * The total number of created threads.
 */
static UINT32 totalThreads = 0;

/**
 * A lock to increment totalThreads.
 */
PIN_LOCK totalThreadLock;

/**
 * The trace file to output to.
 */
FILE *trace;

/**
 * A lock to output to the trace file.
 */
PIN_LOCK traceLock;

/**
 * The pin tool accepts the trace file name as an argument.
 */
KNOB<std::string> KnobOutput(KNOB_MODE_WRITEONCE, "pintool", "o", "trace.out", "trace file name");

// NOTE: this is not portable...
typedef unsigned long int pthread_t;

/**
 * Statistics to track for a thread.
 */
struct threadData {
  /**
   * Constructor.
   */
  threadData() : instructions(0), currentBbl(0)
  {
  }

  /**
   * The number of instructions executed by the thread.
   */
  UINT64 instructions;

  /**
   * The identifier of the basic block currently (at run-time) being executed.
   */
  UINT32 currentBbl;

  /**
   * The pthread library's thread identifier for this thread.
   */
  pthread_t *pthreadHandle;
};

/**
 * The key used for thread-level storage.
 */
static TLS_KEY tlsKey;

/**
 * Get the ThreadData for a given thread.
 *
 * @param threadId The thread identifier to get the data for.
 * @return A pointer to the thread's ThreadData.
 */
threadData *GetThreadData(THREADID const threadId)
{
  return static_cast<threadData *>(PIN_GetThreadData(tlsKey, threadId));
}

/**
 * Write an item to the trace.
 *
 * @param threadId The thread identifier.
 * @param index The array index to the function call from the events array.
 * @param variable Other data, typically the address of a synchronization object.
 */
VOID DumpToTrace(THREADID const threadId, UINT32 const index, ADDRINT const variable)
{
  threadData *data = GetThreadData(threadId);

  // this should be optimized
  PIN_GetLock(&traceLock, threadId + 1);
  fprintf(trace, "%d %s %u %lu %lu\n", threadId, events[index], data->currentBbl, variable,
      data->instructions);
  PIN_ReleaseLock(&traceLock);
}

/**
 * Setup a thread that is about to be created.
 *
 * @param threadId The thread identifier.
 * @param pthreadHandle The thread's pthread identifier.
 */
VOID ThreadCreateBefore(THREADID const threadId, pthread_t *pthreadHandle)
{
  threadData *data = GetThreadData(threadId);
  data->pthreadHandle = pthreadHandle;
}

/**
 * Finalize the setup of a thread that has been created.
 *
 * @param threadId The thread identifier.
 */
VOID ThreadCreateAfter(THREADID const threadId)
{
  threadData *data = GetThreadData(threadId);
  DumpToTrace(threadId, SYNC_CALLS_SIZE + 2, *(data->pthreadHandle));
}

/**
 * Indicate that a join event has occured.
 *
 * @param threadId The identifier of the thread calling join.
 * @param pthreadHandle The pthread identifier of the thread to join with.
 */
VOID ThreadJoin(THREADID const threadId, pthread_t const pthreadHandle)
{
  DumpToTrace(threadId, SYNC_CALLS_SIZE + 3, pthreadHandle);
}

/**
 * Initialize a barrier object.
 *
 * @param threadId The thread identifier that is creating the barrier.
 * @param variable The memory address of the barrier object.
 * @param count The number of threads that must reach the barrier.
 */
VOID BarrierInit(THREADID const threadId, ADDRINT const variable, unsigned const count)
{
  threadData *data = GetThreadData(threadId);
  fprintf(trace, "%d pthread_barrier_init %u %lu %u %lu\n", threadId, data->currentBbl, variable,
      count, data->instructions);
}

/**
 * Add instrumentation calls to the pthread synchronization functions.
 *
 * @param image The image to check for function calls.
 */
VOID ImageLoad(IMG const image, VOID *)
{
  for(int i = 0; i < SYNC_CALLS_SIZE; ++i) {
    RTN routine = RTN_FindByName(image, events[i]);

    if(RTN_Valid(routine)) {
      RTN_Open(routine);

      RTN_InsertCall(routine, IPOINT_BEFORE, AFUNPTR(DumpToTrace), IARG_THREAD_ID, IARG_UINT32, i,
          IARG_FUNCARG_ENTRYPOINT_VALUE, 0, IARG_END);

      RTN_Close(routine);
    }
  }

  RTN create_routine = RTN_FindByName(image, "pthread_create");
  if(RTN_Valid(create_routine)) {
    RTN_Open(create_routine);

    RTN_InsertCall(create_routine, IPOINT_BEFORE, AFUNPTR(ThreadCreateBefore), IARG_THREAD_ID,
        IARG_FUNCARG_ENTRYPOINT_VALUE, 0, IARG_END);

    RTN_InsertCall(
        create_routine, IPOINT_AFTER, AFUNPTR(ThreadCreateAfter), IARG_THREAD_ID, IARG_END);

    RTN_Close(create_routine);
  }

  RTN join_routine = RTN_FindByName(image, "pthread_join");
  if(RTN_Valid(join_routine)) {
    RTN_Open(join_routine);

    RTN_InsertCall(join_routine, IPOINT_BEFORE, AFUNPTR(ThreadJoin), IARG_THREAD_ID,
        IARG_FUNCARG_ENTRYPOINT_VALUE, 0, IARG_END);

    RTN_Close(join_routine);
  }

  RTN barrier_init_routine = RTN_FindByName(image, "pthread_barrier_init");
  if(RTN_Valid(barrier_init_routine)) {
    RTN_Open(barrier_init_routine);

    RTN_InsertCall(barrier_init_routine, IPOINT_BEFORE, AFUNPTR(BarrierInit), IARG_THREAD_ID,
        IARG_FUNCARG_ENTRYPOINT_VALUE, 0, IARG_FUNCARG_ENTRYPOINT_VALUE, 2, IARG_END);

    RTN_Close(barrier_init_routine);
  }
}

/**
 * Update thread data based on the basic block executed.
 */
VOID PIN_FAST_ANALYSIS_CALL BasicBlockExecuted(THREADID const threadId,
    UINT32 const bblId,
    UINT32 const numInstructions)
{
  threadData *data = GetThreadData(threadId);
  data->instructions += numInstructions;
  data->currentBbl = bblId;
}

/**
 * Instrument basic blocks to track executed instructions.
 *
 * @param trace The trace to instrument basic blocks from.
 */
VOID Trace(TRACE trace, VOID *)
{
  static UINT32 next_bblId = 0;

  for(BBL bbl = TRACE_BblHead(trace); BBL_Valid(bbl); bbl = BBL_Next(bbl)) {
    UINT32 bblId = next_bblId++;

    BBL_InsertCall(bbl, IPOINT_ANYWHERE, AFUNPTR(BasicBlockExecuted), IARG_FAST_ANALYSIS_CALL,
        IARG_THREAD_ID, IARG_UINT32, bblId, IARG_UINT32, BBL_NumIns(bbl), IARG_END);
  }
}

/**
 * Setup when a thread has begun execution (i.e., after it has been created).
 *
 * @param threadId The thread identifier.
 */
VOID ThreadStart(THREADID const threadId, CONTEXT *, INT32, VOID *)
{
  // setup thread local storage
  threadData *data = new threadData;
  PIN_SetThreadData(tlsKey, data, threadId);

  DumpToTrace(threadId, SYNC_CALLS_SIZE, 0);

  PIN_GetLock(&totalThreadLock, threadId + 1);
  totalThreads++;
  PIN_ReleaseLock(&totalThreadLock);
}

/**
 * Indiciate that a thread has finished execution.
 *
 * @param threadId The thread identifier.
 */
VOID ThreadFini(THREADID const threadId, CONTEXT const *, INT32, VOID *)
{
  DumpToTrace(threadId, SYNC_CALLS_SIZE + 1, 0);
}

/**
 * Close the trace file.
 */
VOID Fini(INT32, VOID *)
{
  fclose(trace);
}

/**
 * Print some help on how to use the pin tool.
 */
VOID PrintUsage()
{
  std::cerr << KNOB_BASE::StringKnobSummary() << "\n";
}

int main(int argc, char *argv[])
{
  PIN_InitSymbols();
  if(PIN_Init(argc, argv)) {
    PrintUsage();

    return EXIT_FAILURE;
  }

  trace = fopen(KnobOutput.Value().c_str(), "w");

  tlsKey = PIN_CreateThreadDataKey(0);
  PIN_InitLock(&totalThreadLock);
  PIN_InitLock(&traceLock);

  PIN_AddThreadStartFunction(ThreadStart, NULL);
  PIN_AddThreadFiniFunction(ThreadFini, NULL);

  TRACE_AddInstrumentFunction(Trace, NULL);
  IMG_AddInstrumentFunction(ImageLoad, NULL);
  PIN_AddFiniFunction(Fini, NULL);

  // never returns
  PIN_StartProgram();

  return EXIT_SUCCESS;
}
