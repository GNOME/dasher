#ifndef __eventhandler_h__
#define __eventhandler_h__

#include <list>
#include <algorithm>

template <typename  T> class Observable;

///Thing that listens to events - parameterized by the type of event.
template <typename T> class Observer {
public:
  virtual ~Observer() {};
  ///Called to indicate an event has occurred! Subclasses must implement.
  virtual void HandleEvent(T evt)=0;
};

///An Event handler for a single type of event: maintains a list of listeners,
/// allows listeners to (un/)register, and allows dispatching of events to all
/// listeners.
template <typename T> class Observable {
public:
  Observable();
  void Register(Observer<T> *pLstnr);
  void Unregister(Observer<T> *pLstnr);
  void DispatchEvent(T t);
private:
  typedef typename std::list< Observer<T>* > ListenerList;
  typedef typename ListenerList::iterator L_it;
  ListenerList m_vListeners;
  ListenerList m_vListenersToAdd;
  int m_iInHandler;
};

template <typename T> Observable<T>::Observable()
{
  m_iInHandler = 0;
}

///Utility class for Observers which register with an Observable at construction
/// and deregister at destruction. (I.e. which are strictly shorter-lived, than the
/// Observable they listen to!)
template <typename T> class TransientObserver : public Observer<T> {
public:
  TransientObserver(Observable<T> *pObservable) : m_pEventHandler(pObservable) {
    m_pEventHandler->Register(this);
  }
  virtual ~TransientObserver() {
    m_pEventHandler->Unregister(this);
  }
protected:
  Observable<T> *m_pEventHandler;
};

template <typename T> void Observable<T>::Register(Observer<T> *pListener) {
  if (m_iInHandler == 0)
    m_vListeners.push_back(pListener);
  else
    m_vListenersToAdd.push_back(pListener);
}

template <typename T> void Observable<T>::Unregister(Observer<T> *pListener) {
  if (m_iInHandler == 0)
    m_vListeners.remove(pListener);
  else {
    L_it it = std::find(m_vListeners.begin(), m_vListeners.end(), pListener);
    if (it != m_vListeners.end())
	  *it = NULL;
  }
}

template <typename T> void Observable<T>::DispatchEvent(T evt) {

  // Speed up start-up before any listeners are registered
  if (m_vListeners.empty()) return;

  // Just in case the same event handler was registered twice.
  if (m_iInHandler == 0)
    m_vListeners.unique();

  // We may end up here recursively, so keep track of how far down we
  // are, and only permit new handlers to be registered after all
  // messages are processed.

  // An alternative approach would be a message queue - this might actually be a bit more sensible
  ++m_iInHandler;

  // Loop through components and notify them of the event
  for(L_it I=m_vListeners.begin(), E=m_vListeners.end(); I!=E; I++) {
    if (*I != NULL) { // Listener not removed during iteration
      (*I)->HandleEvent(evt);
    }
  }

  --m_iInHandler;

  if (m_iInHandler == 0) {
    m_vListeners.remove(NULL);
    m_vListeners.splice(m_vListeners.end(), m_vListenersToAdd);
  }
}
#endif
