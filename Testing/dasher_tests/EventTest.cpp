#include "gtest/gtest.h"
#include "../../Src/TestPlatform/MockInterfaceBase.h"
#include "../../Src/TestPlatform/MockSettingsStore.h"
#include "../../Src/DasherCore/EventHandler.h"
#include "../../Src/DasherCore/SettingsStore.h"

//A Dasher component that allows tests to easily determine whether
//or not it was notified of an event which it subscribed to.
class DummyComponent : public Dasher::CDasherComponent {

  public:
    
    int evtCount;
    std::vector<Dasher::CEvent*> receivedEvents;
    
    DummyComponent(Dasher::CEventHandler* pEventHandler, CSettingsStore* pSettingsStore) :
             Dasher::CDasherComponent(pEventHandler, pSettingsStore, std::vector<int>()) {
      evtCount = 0;
    }
    
    virtual void HandleEvent(Dasher::CEvent *pEvent) {
      evtCount++;
      receivedEvents.push_back(pEvent);
    }
};

//An extension of DummyComponent that fires a CStartEvent every time its HandleEvent is called
class RecursionComponent : public DummyComponent {
  
  public:
  
    RecursionComponent(Dasher::CEventHandler* pEventHandler, CSettingsStore* pSettingsStore) :
             DummyComponent(pEventHandler, pSettingsStore) {
    }
    
    void HandleEvent(Dasher::CEvent *pEvent) {
      
      evtCount++;
      m_pEventHandler->InsertEvent(new Dasher::CStartEvent());
    }
};

//Test fixture - sets up the EventHandler, SettingsStore, and InterfaceBase instances.
class EventTest : public ::testing::Test {
  
  public:
  
    EventTest() {
      
      interfaceBase = new CMockInterfaceBase();
      evtHandler = new Dasher::CEventHandler(interfaceBase);
      settingsStore = new CMockSettingsStore(evtHandler);
    }
    
  protected:
  
    Dasher::CEventHandler *evtHandler;
    CMockInterfaceBase *interfaceBase;
    CMockSettingsStore *settingsStore;
};

//very basic test - subscribe 5 components, fire one event, and assert that they all were notified
TEST_F(EventTest, BasicTest) {  
  
  std::vector<DummyComponent*> components;
  
  for(int c = 0; c < 5; c++) {
    
    DummyComponent *newComponent = new DummyComponent(evtHandler, settingsStore);
    components.push_back(newComponent);
    evtHandler->RegisterListener(newComponent, EV_PARAM_NOTIFY);
  }
  
  evtHandler->InsertEvent(new CParameterNotificationEvent(0));
  
  for(std::vector<DummyComponent*>::iterator iter = components.begin(); iter != components.end(); ++iter) {
    EXPECT_EQ((*iter)->evtCount, 1);
  }

}

//stress test - register a lot of listeners, fire a lot of events, and EXPECT that 
//everything was notified without crashing
TEST_F(EventTest, StressTest) {
  
  std::vector<DummyComponent*> components;
  
  for(int c = 0; c < 1000; c++) {
    
    DummyComponent *newComponent = new DummyComponent(evtHandler, settingsStore);
    components.push_back(newComponent);
    evtHandler->RegisterListener(newComponent, EV_PARAM_NOTIFY);
  }
  
  for(int c = 0; c < 1000; c++) {
    evtHandler->InsertEvent(new CParameterNotificationEvent(0));
  }
  
  for(std::vector<DummyComponent*>::iterator iter = components.begin(); iter != components.end(); ++iter) {
    EXPECT_EQ((*iter)->evtCount, 1000);
  }
}

//test for proper behavior when components call InsertEvent in their HandleEvent
TEST_F(EventTest, RecursionTest) {
  
  RecursionComponent *rec1 = new RecursionComponent(evtHandler, settingsStore);
  RecursionComponent *rec2 = new RecursionComponent(evtHandler, settingsStore);
  DummyComponent *listener = new DummyComponent(evtHandler, settingsStore);
  
  evtHandler->RegisterListener(rec1, EV_PARAM_NOTIFY);
  evtHandler->RegisterListener(rec2, EV_PARAM_NOTIFY);
  evtHandler->RegisterListener(listener, EV_START);
  
  evtHandler->InsertEvent(new Dasher::CParameterNotificationEvent(0));
  
  EXPECT_EQ(rec1->evtCount, 1);
  EXPECT_EQ(rec2->evtCount, 1);
  EXPECT_EQ(listener->evtCount, 2);
}

//register listeners for one event, fire a different event, and
//check that they DON'T receieve that event
TEST_F(EventTest, StaysQuiet) {
  
  DummyComponent *listener1 = new DummyComponent(evtHandler, settingsStore);
  DummyComponent *listener2 = new DummyComponent(evtHandler, settingsStore);
  
  evtHandler->RegisterListener(listener1, EV_GAME_NODE_DRAWN);
  evtHandler->RegisterListener(listener2, EV_COMMAND);
  
  evtHandler->InsertEvent(new Dasher::CParameterNotificationEvent(0));
  
  EXPECT_EQ(listener1->evtCount, 0);
  EXPECT_EQ(listener2->evtCount, 0);
}

//verify that events are fired in the order we claim them to be
TEST_F(EventTest, EventOrdering) {
  
  std::vector<CEvent*> events;
  
  DummyComponent *listener = new DummyComponent(evtHandler, settingsStore);
  
  evtHandler->RegisterListener(listener, EV_START);
  
  for(int c = 0; c < 5; c++) {
    
    Dasher::CStartEvent *curEvt = new Dasher::CStartEvent();
    events.push_back(curEvt);
    evtHandler->InsertEvent(curEvt);
  }
  
  for(int c = 0; c < 5; c++) {
    EXPECT_EQ(events[c], listener->receivedEvents[c]); 
  }
}
