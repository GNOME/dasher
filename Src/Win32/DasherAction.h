#ifndef __DasherAction_h__
#define __DasherAction_h__

#include <string>

class CDasherAction {
public:
  // Return the human readable name for this action
  virtual std::string GetName() = 0;

  // Activate the action (eg load any required resources)
  virtual bool Activate() = 0;

  // Deactivate the action (deallocate resources etc)
  virtual bool Deactivate() = 0;

  // Return whether or not the action is active
  virtual bool GetActive() = 0;

  // Execute the action
  virtual bool Execue(const std::wstring &strText) = 0;
};

#endif
