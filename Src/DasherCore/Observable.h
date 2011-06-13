#ifndef __eventhandler_h__
#define __eventhandler_h__

#include <deque>
#include <algorithm>

template <typename  T> class Observable;

///Thing that listens to events - parameterized by the type of event.
template <typename T> class Observer {
public:
  ///Called to indicate an event has occurred! Subclasses must implement.
  virtual void HandleEvent(T evt)=0;
};

///An Event handler for a single type of event: maintains a list of listeners,
/// allows listeners to (un/)register, and allows dispatching of events to all
/// listeners.
template <typename T> class Observable {
public:
  void Register(Observer<T> *pLstnr, bool bLast=false);
  void Unregister(Observer<T> *pLstnr);
protected:
  void DispatchEvent(T t);
private:
  typedef typename std::deque< Observer<T>* > ListenerList;
  typedef typename ListenerList::iterator L_it;
  ListenerList m_vListeners;
  int m_iInHandler;
};

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

template <typename T> void Observable<T>::Register(Observer<T> *pListener, bool bLast) {
  L_it it = std::find(m_vListeners.begin(), m_vListeners.end(), pListener);
  if (it != m_vListeners.end()) {
    //already in list...in ok place?
    if (it == (bLast ? m_vListeners.end() : m_vListeners.begin())) return;
    //put it first or last, these are the only places we can guarantee satisfy the constraint of bLast
    if (m_iInHandler) *it=NULL; else m_vListeners.erase(it);
  }
  if (bLast) m_vListeners.push_back(pListener); else m_vListeners.insert(m_vListeners.begin(),pListener);
}

template <typename T> void Observable<T>::Unregister(Observer<T> *pListener) {
  L_it it = std::find(m_vListeners.begin(), m_vListeners.end(), pListener);
  if (it==m_vListeners.end()) return;
  if (m_iInHandler) {
    //remove listener, but leave behind its slot, so as not to upset the in-progress iteration
    *it=NULL;
  } else {
    m_vListeners.erase(it);
  }
}

template <typename T> void Observable<T>::DispatchEvent(T evt) {
  
  // We may end up here recursively, so keep track of how far down we
  // are, and only permit new handlers to be registered after all
  // messages are processed.
  
  // An alternative approach would be a message queue - this might actually be a bit more sensible
  ++m_iInHandler;
  // Loop through components and notify them of the event
  for(L_it I=m_vListeners.begin(), E=m_vListeners.end(); I!=E; I++) {
    if (*I) //don't dispatch to NULLs (slots remaining from listeners removed during iteration)
      (*I)->HandleEvent(evt);
  }
  
  --m_iInHandler;
  
  if(m_iInHandler == 0) {
    for (L_it it=m_vListeners.begin(); it!=m_vListeners.end(); it++)
      if (!(*it)) //slot remaining from listener removed during iteration. Shuffle up...
        m_vListeners.erase(it--);
  }
}
#endif
