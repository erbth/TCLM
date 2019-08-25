# Server

## Processes
  * When accessing a Process the Process\_map needs to be at least read locked.
    (shared_lock be acquired). Otherwise another thread within TCLM may destroy
    it in the mean time.

  * However a writable lock on the Process itself is acquired automatically by
    the member functions.

## Lock\_Requests
  * Like with Processes the list must be locked here, too.
