/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) 1993-2002 Ken Martin, Will Schroeder, Bill Lorensen 
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkThreadedController.h"
#include "vtkObjectFactory.h"

#include "vtkDataSet.h"
#include "vtkImageData.h"
#include "vtkMultiThreader.h"
#include "vtkOutputWindow.h"
#include "vtkCriticalSection.h"
#include "vtkSharedMemoryCommunicator.h"

#ifdef VTK_USE_SPROC
#include <sys/prctl.h>
#endif

static vtkSimpleCriticalSection vtkOutputWindowCritSect;

// Output window which prints out the process id
// with the error or warning messages
class VTK_PARALLEL_EXPORT vtkThreadedControllerOutputWindow : public vtkOutputWindow
{
public:
  vtkTypeRevisionMacro(vtkThreadedControllerOutputWindow,vtkOutputWindow);

  void DisplayText(const char* t)
  {
    // Need to use critical section because the output window
    // is global. For the same reason, the process id has to
    // be obtained by calling GetGlobalController
    vtkOutputWindowCritSect.Lock();
    vtkMultiProcessController* cont = 
      vtkMultiProcessController::GetGlobalController();
    if (cont)
      {
      cout << "Process id: " << cont->GetLocalProcessId()
           << " >> ";
      }
    cout << t;
    cout.flush();
    vtkOutputWindowCritSect.Unlock();
  }

  vtkThreadedControllerOutputWindow()
  {
    vtkObject* ret = vtkObjectFactory::CreateInstance("vtkThreadedControllerOutputWindow");
    if (ret)
      ret->Delete();
  }

  friend class vtkThreadedController;

private:
  vtkThreadedControllerOutputWindow(const vtkThreadedControllerOutputWindow&);
  void operator=(const vtkThreadedControllerOutputWindow&);
};

vtkCxxRevisionMacro(vtkThreadedControllerOutputWindow, "$Revision$");

vtkCxxRevisionMacro(vtkThreadedController, "$Revision$");
vtkStandardNewMacro(vtkThreadedController);

void vtkThreadedController::CreateOutputWindow()
{
#if defined(VTK_USE_PTHREADS) || defined(VTK_USE_SPROC) || defined(VTK_USE_WIN32_THREADS)
  vtkThreadedControllerOutputWindow* window = new vtkThreadedControllerOutputWindow;
  this->OutputWindow = window;
  vtkOutputWindow::SetInstance(this->OutputWindow);
#endif
}

//----------------------------------------------------------------------------
vtkThreadedController::vtkThreadedController()
{
  this->LocalProcessId = 0;

  vtkMultiThreader::SetGlobalMaximumNumberOfThreads(0);

  this->MultiThreader = 0;
  this->NumberOfProcesses = 0;
  this->MultipleMethodFlag = 0;
    
  this->LastNumberOfProcesses = 0;
  this->Controllers = 0;

  this->OutputWindow = 0;
}

//----------------------------------------------------------------------------
vtkThreadedController::~vtkThreadedController()
{
  if (this->MultiThreader)
    {
    this->MultiThreader->Delete();
    }
  
   if(this->Communicator)
     {
     this->Communicator->Delete();
     }

   this->NumberOfProcesses = 0;
   this->ResetControllers();
}

//----------------------------------------------------------------------------
void vtkThreadedController::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
  if (this->MultiThreader)
    {
    os << indent << "MultiThreader:\n";
    this->MultiThreader->PrintSelf(os, indent.GetNextIndent());
    }
  else
    {
    os << indent << "MultiThreader: (none)\n";
    }
  os << indent << "LocalProcessId: " << this->LocalProcessId << endl;
  os << indent << "Barrier in progress: " 
     << (vtkThreadedController::IsBarrierInProgress ? "(yes)" : "(no)")
     << endl;
  os << indent << "Barrier counter: " << vtkThreadedController::Counter
     << endl;
  os << indent << "Last number of processes: " << this->LastNumberOfProcesses
     << endl;  
}

//----------------------------------------------------------------------------
void vtkThreadedController::Initialize(int* vtkNotUsed(argc), 
                                       char*** vtkNotUsed(argv))
{
  if ( !vtkThreadedController::BarrierLock )
    {
    vtkThreadedController::BarrierLock = new vtkSimpleCriticalSection(1);
    }
  if ( !vtkThreadedController::BarrierInProgress)
    {
    vtkThreadedController::BarrierInProgress = new vtkSimpleCriticalSection;
    }
}

void vtkThreadedController::Finalize()
{
  if (vtkThreadedController::BarrierLock)
    {
    vtkThreadedController::BarrierLock->Unlock();
    }
  delete vtkThreadedController::BarrierLock;
  vtkThreadedController::BarrierLock = 0;
  delete vtkThreadedController::BarrierInProgress;
  vtkThreadedController::BarrierInProgress = 0;
}
  
void vtkThreadedController::ResetControllers()
{
  int i;

  for(i=1; i < this->LastNumberOfProcesses; i++)
    {
    this->Controllers[i]->Delete();
    }

  if (this->NumberOfProcesses == this->LastNumberOfProcesses)
    {
    return;
    }
  
  delete[] this->Controllers;

  if (this->NumberOfProcesses > 0 )
    {
    this->Controllers = new vtkThreadedController*[this->NumberOfProcesses];
    }
}


//----------------------------------------------------------------------------
// Called before threads are spawned to create the "process objecs".
void vtkThreadedController::CreateProcessControllers()
{

  // Delete previous controllers.
  this->ResetControllers();

  // Create the controllers.
  // The original controller will be assigned thread 0.
  this->Controllers[0] = this;
  this->LocalProcessId = 0;

  // Create a new communicator.
  if (this->Communicator)
    {
    this->Communicator->Delete();
    }
  this->Communicator = vtkSharedMemoryCommunicator::New();
  ((vtkSharedMemoryCommunicator*)this->Communicator)->Initialize(
    this->NumberOfProcesses, 
    this->ForceDeepCopy);
  this->RMICommunicator = this->Communicator;

  // Initialize the new controllers.
  for (int i = 1; i < this->NumberOfProcesses; ++i)
    {
    this->Controllers[i] = vtkThreadedController::New();
    this->Controllers[i]->LocalProcessId = i;
    this->Controllers[i]->NumberOfProcesses = this->NumberOfProcesses;
    this->Controllers[i]->Communicator = 
      ((vtkSharedMemoryCommunicator*)this->Communicator)->Communicators[i];
    this->Controllers[i]->RMICommunicator = 
      ((vtkSharedMemoryCommunicator*)this->RMICommunicator)->Communicators[i];
    }

  // Stored in case someone changes the number of processes.
  // Needed to delete the controllers properly.
  this->LastNumberOfProcesses = this->NumberOfProcesses;
}

vtkSimpleCriticalSection vtkThreadedController::CounterLock;
int vtkThreadedController::Counter;

#ifdef VTK_USE_WIN32_THREADS
HANDLE vtkThreadedController::BarrierEndedEvent = 0;
HANDLE vtkThreadedController::NextThread = 0;
#else
//vtkSimpleCriticalSection vtkThreadedController::BarrierLock(1);
//vtkSimpleCriticalSection vtkThreadedController::BarrierInProgress;
vtkSimpleCriticalSection* vtkThreadedController::BarrierLock = 0;
vtkSimpleCriticalSection* vtkThreadedController::BarrierInProgress = 0;
#endif
int vtkThreadedController::IsBarrierInProgress=0;


void vtkThreadedController::Barrier()
{
  if (this->NumberOfProcesses == 0)
    {
    return;
    }

#ifndef VTK_USE_WIN32_THREADS
  if (!vtkThreadedController::BarrierLock || 
      !vtkThreadedController::BarrierInProgress)
    {
    vtkErrorMacro("Barrier was called without initializing threads. "
                  "Please call Initialize first. Skipping barrier.");
    return;
    }
#endif

  vtkThreadedController::InitializeBarrier();

  // If there was a barrier before this one, we need to
  // wait until that is cleaned up
  if (vtkThreadedController::IsBarrierInProgress)
    {
    vtkThreadedController::WaitForPreviousBarrierToEnd();
    }
#ifdef VTK_USE_WIN32_THREADS
  else
    {
    ResetEvent(vtkThreadedController::BarrierEndedEvent);
    }
#endif

  // All processes increment the counter (which is initially 0) by 1
  vtkThreadedController::CounterLock.Lock();
  int count = ++vtkThreadedController::Counter;
  vtkThreadedController::CounterLock.Unlock();

  if (count == this->NumberOfProcesses)
    {
    // If you are the last process, unlock the barrier
    vtkThreadedController::BarrierStarted();
    vtkThreadedController::SignalNextThread();
    }
  else
    {
    // If you are not the last process, wait until someone unlocks 
    // the barrier
    vtkThreadedController::WaitForNextThread();
    vtkThreadedController::CounterLock.Lock();
    vtkThreadedController::Counter--;
    vtkThreadedController::CounterLock.Unlock();
    if (vtkThreadedController::Counter == 1)
      {
      // If you are the last process to pass the barrier
      // Set the counter to 0 and leave the barrier locked
      vtkThreadedController::Counter = 0;
      // Barrier is over, another one can start
      vtkThreadedController::BarrierEnded();
      }
    else
      {
      //  unlock the barrier for the next guy
      vtkThreadedController::SignalNextThread();
      }

    }
}

//----------------------------------------------------------------------------
VTK_THREAD_RETURN_TYPE vtkThreadedController::vtkThreadedControllerStart( 
  void *arg )
{
  vtkMultiThreader::ThreadInfo* info = (vtkMultiThreader::ThreadInfo*)(arg);
  int threadId = info->ThreadID;
  vtkThreadedController *controller0 =(vtkThreadedController*)(info->UserData);

  controller0->Start(threadId);
  return VTK_THREAD_RETURN_VALUE;
}

//----------------------------------------------------------------------------
// We are going to try something new.  We will pass the local controller
// as the argument.
void vtkThreadedController::Start(int threadId)
{
  vtkThreadedController* localController = this->Controllers[threadId];

    // Store threadId in a table.
#ifdef VTK_USE_PTHREADS  
  localController->ThreadId = pthread_self();
#elif defined VTK_USE_SPROC
  localController->ThreadId = PRDA->sys_prda.prda_sys.t_pid;
#elif defined VTK_USE_WIN32_THREADS
  localController->ThreadId = GetCurrentThreadId();
#endif

  this->Barrier();

  if (this->MultipleMethodFlag)
    {
    if (this->MultipleMethod[threadId])
      {
      (this->MultipleMethod[threadId])(localController, 
                                       this->MultipleData[threadId]);
      }
    else
      {
      vtkWarningMacro("MultipleMethod " << threadId << " not set");
      }
    }
  else
    {
    if (this->SingleMethod)
      {
      (this->SingleMethod)(localController, this->SingleData);
      }
    else
      {
      vtkErrorMacro("SingleMethod not set");
      } 
    }
}

//----------------------------------------------------------------------------
// Execute the method set as the SingleMethod on NumberOfThreads threads.
void vtkThreadedController::SingleMethodExecute()
{
  if (!this->MultiThreader)
    {
    this->MultiThreader = vtkMultiThreader::New();
    }
  this->CreateProcessControllers();
  this->MultipleMethodFlag = 0;

  this->MultiThreader->SetSingleMethod(vtkThreadedControllerStart, 
                                       (void*)this);
  this->MultiThreader->SetNumberOfThreads(this->NumberOfProcesses);

  // GLOBAL_CONTROLLER will be from thread0 always.
  // GetLocalController will translate to the local controller.
  vtkMultiProcessController::SetGlobalController(this);
  
  this->MultiThreader->SingleMethodExecute();
}
//----------------------------------------------------------------------------
// Execute the methods set as the MultipleMethods.
void vtkThreadedController::MultipleMethodExecute()
{
  if (!this->MultiThreader)
    {
    this->MultiThreader = vtkMultiThreader::New();
    }
  this->CreateProcessControllers();
  this->MultipleMethodFlag = 1;

  this->MultiThreader->SetSingleMethod(vtkThreadedControllerStart, 
                                       (void*)this);
  this->MultiThreader->SetNumberOfThreads(this->NumberOfProcesses);

  // GLOBAL_CONTROLLER will be from thread0 always.
  // GetLocalController will translate to the local controller.
  vtkMultiProcessController::SetGlobalController(this);

  this->MultiThreader->SingleMethodExecute();
}

vtkMultiProcessController *vtkThreadedController::GetLocalController()
{
#ifdef VTK_USE_PTHREADS  
  int idx;
  pthread_t pid = pthread_self();
  for (idx = 0; idx < this->NumberOfProcesses; ++idx)
    {
    if (pthread_equal(pid, this->Controllers[idx]->ThreadId))
      {
      return this->Controllers[idx];
      }
    }

  // Need to use cerr instead of error macro here to prevent
  // recursion (the controller's output window calls GetLocalController)
  cerr << "Could Not Find my process id." << endl;
  return NULL;
#elif defined VTK_USE_SPROC
  int idx;
  pid_t pid = PRDA->sys_prda.prda_sys.t_pid;
  for (idx = 0; idx < this->NumberOfProcesses; ++idx)
    {
    if (pid == this->Controllers[idx]->ThreadId)
      {
      return this->Controllers[idx];
      }
    }
  
  // Need to use cerr instead of error macro here to prevent
  // recursion (the controller's output window calls GetLocalController)
  cerr << "Could Not Find my process id." << endl;
  return NULL;
#elif defined VTK_USE_WIN32_THREADS

  int idx;
  DWORD pid = GetCurrentThreadId();
  for (idx = 0; idx < this->NumberOfProcesses; ++idx)
    {
    if (pid == this->Controllers[idx]->ThreadId)
      {
      return this->Controllers[idx];
      }
    }
  
  // Need to use cerr instead of error macro here to prevent
  // recursion (the controller's output window calls GetLocalController)
  cerr << "Could Not Find my process id." << endl;
  return NULL;
  
#else

  vtkErrorMacro(
    "ThreadedController only works with windows api, pthreads or sproc");
  return this;
  
#endif  
}

// Note that the Windows and Unix implementations of
// these methods are completely different. This is because,
// in Windows, if the same thread locks the same mutex/critical
// section twice, it will not block. Therefore, this method
// can not be used to make the threads wait until all of them
// reach the barrier


// If there was a barrier before this one, we need to
// wait until that is cleaned up or bad things happen.
void vtkThreadedController::WaitForPreviousBarrierToEnd()
{
#ifdef VTK_USE_WIN32_THREADS
  WaitForSingleObject(vtkThreadedController::BarrierEndedEvent, INFINITE);
#else
  vtkThreadedController::BarrierInProgress->Lock();
  vtkThreadedController::BarrierInProgress->Unlock();
#endif
}

void vtkThreadedController::BarrierStarted()
{
  vtkThreadedController::IsBarrierInProgress = 1;
#ifdef VTK_USE_WIN32_THREADS

#else
  vtkThreadedController::BarrierInProgress->Lock();
#endif
}

// A new barrier can now start
void vtkThreadedController::BarrierEnded()
{
  vtkThreadedController::IsBarrierInProgress = 0;
#ifdef VTK_USE_WIN32_THREADS
  SetEvent(vtkThreadedController::BarrierEndedEvent);
#else
  vtkThreadedController::BarrierInProgress->Unlock();
#endif
}

// Tell the next guy that it is ok to continue with the barrier
void vtkThreadedController::SignalNextThread()
{
#ifdef VTK_USE_WIN32_THREADS
  SetEvent(vtkThreadedController::NextThread);
#else
  vtkThreadedController::BarrierLock->Unlock();
#endif
}

// Create the windows event necessary for waiting  
void vtkThreadedController::InitializeBarrier()
{
#ifdef VTK_USE_WIN32_THREADS
  if (!BarrierEndedEvent)
    {
    vtkThreadedController::BarrierEndedEvent = CreateEvent(0,FALSE,FALSE,0);
    vtkThreadedController::NextThread = CreateEvent(0,FALSE,FALSE,0);
    }
#endif
}

// Wait until the previous thread says it's ok to continue
void vtkThreadedController::WaitForNextThread()
{
#ifdef VTK_USE_WIN32_THREADS
  WaitForSingleObject(vtkThreadedController::NextThread,INFINITE);
#else
  vtkThreadedController::BarrierLock->Lock();
#endif
}
