#pragma once

#include <typeindex>
#include <unordered_map>
#include <vector>
#include <queue>

#include "Event.h"
#include "Timer.h"

constexpr size_t MAX_ADDED_EVENTS = 1000;

#ifdef _DEBUG
//#define ENABLE_EVENT_DEBUGGING
//#define ENABLE_EVENT_PRINTING
#endif

// ONE MUST BE DEFINED!
#define USE_MAX_TIME
//#define USE_MAX_EVENTS
//#define USE_MAX_CYCLES

#ifdef USE_MAX_TIME
constexpr float MAX_TIME = 0.005f;
#elif USE_MAX_EVENTS
constexpr unsigned int MAX_EVENTS = 500000;
#elif USE_MAX_CYCLES
constexpr size_t MAX_CLOCK_CYCLES = 50000;
#endif

#ifdef USE_MAX_TIME
#define CHECK_CONDITION() (timer.GetTotalTime() >= MAX_TIME)
#elif defined(USE_MAX_EVENTS)
#define CHECK_CONDITION() (handledEventsCount >= MAX_EVENTS)
#elif defined(USE_MAX_CYCLES)
#define CHECK_CONDITION() (timer.GetElapsedCycles() >= MAX_CLOCK_CYCLES)
#else
#error "Must use at least one measure mode!"
#endif

/// EventSystem
namespace ES
{
	/* Event System by Anton Eriksson
	* -----------------------------------------
	* EventSystem is a Singleton class that handles events and observers.
	* Events are stored as pointers to the base class Event, in order to
	* be able to store different types of events.
	*/

	// Observer interface for the class that wants to receive events
	class IObserver
	{
	public:
		virtual ~IObserver() = default;

		virtual void OnReceiveEvent(Event& aEvent) = 0;
		virtual void OnInit() = 0;
		virtual void OnDestroy() = 0;

		template <class EventClass>
		void OnReceiveEvent(EventClass& aEvent)
		{
			OnReceiveEvent(static_cast<Event&>(aEvent));
			// HOW TO USE:
			// const ExampleEvent* exampleEvent = dynamic_cast<ExampleEvent*>(&aEvent);
		}

	//protected:
	//	bool myIsActive = false;
	};

	// Event system that handles distribution of events
	class EventSystem
	{
	public:
		static EventSystem& GetInstance()
		{
			static EventSystem instance;
			return instance;
		}

		// Attach an Observer to an Event (a subclass to Event)
		// The Observer will be notified when this Event is called
		template <class EventClass>
		void Attach(IObserver* aObserver)
		{
			// Get the vector in myObservers at the given key
			// _typeid_ determines the type that the Observer expects
			std::vector<IObserver*>& observers = myObservers[typeid(EventClass)];
			// Check if Observer is already attached to this event
			const std::vector<IObserver*>::iterator it = std::ranges::find(observers, aObserver);
			if (it != observers.end())
			{
#ifdef _DEBUG
				printf("\n-------------------------------------------------------------------");
				printf("\nObserver is already attached to event in " __FUNCSIG__);
				printf("\nThis means you have probably missed calling Detach on the observer!");
				printf("\n-------------------------------------------------------------------\n");
#endif
				return;
			}
			// Adds observer to the vector for the type is expects
			observers.push_back(aObserver);
		}

		// Detach an Observer from an Event, it will no longer be notified
		template <class EventClass>
		void Detach(IObserver* aObserver)
		{
			// Get the vector in myObservers at the given key
			// _typeid_ determines the type that the Observer expects
			std::vector<IObserver*>& observers = myObservers[typeid(EventClass)];
			// Check if Observer is attached to this event
			const std::vector<IObserver*>::iterator it = std::remove(observers.begin(), observers.end(), aObserver);
			if (it != observers.end())
			{
				observers.erase(it);
			}
			else
			{
#ifdef _DEBUG
				printf("\n-------------------------------------");
				printf("\nObserver of type %s", typeid(EventClass).name());
				printf(" could not be detached in " __FUNCSIG__);
				printf("\nThis may result in memory leaks!");
				printf("\n-------------------------------------\n");
#endif
			}

			// If the vector is empty, we remove its key from myObservers
			if (observers.empty())
			{
				myObservers.erase(typeid(EventClass));
			}
		}

		// Send event directly to all Observers (if any) that are attached to the Event
		template <class EventClass>
		void SendEvent(EventClass& aEvent)
		{
			// Get the type from anEvent
			const type_info& type = typeid(aEvent);
			// Check if anEvent is a key in myObservers
			const std::unordered_map<std::type_index, std::vector<IObserver*>>::iterator it = myObservers.find(type);
			if (it != myObservers.end())
			{
				// Get the value from key-value pair (key = first, value = second)
				const std::vector<IObserver*>& observers = it->second;
				// Notify all Observers for this event type
				for (int i = 0; i < observers.size(); i++)
				{
					observers[i]->OnReceiveEvent(aEvent);
				}
			}
			else
			{
#ifdef ENABLE_EVENT_DEBUGGING
				printf("\nNo observers attached to this event in " __FUNCSIG__);
#endif
			}
		}

		// Add events to queue to be called at a later stage
		template <class EventClass>
		void QueueEvent(EventClass* aEvent)
		{
			myEventQueue.push(aEvent);

			assert(
				myEventQueue.size() < MAX_ADDED_EVENTS &&
				"Event queue is full! Increase MAX_ADDED_EVENTS in EventSystem.h");
		}

		// Handle each queued event to notify Observers
		void HandleQueuedEvents()
		{
			if (myEventQueue.empty())
			{
#ifdef ENABLE_EVENT_PRINTING
				if (myDoOnce)
				{
					for (EventMetadata& metadata : myEventMetadatas)
					{
						std::string text = "\n\nReturn reason: " + metadata.myDesc;
						OutputDebugStringA(text.c_str());
						text = "\nHandled events: " + std::to_string(metadata.myHandledEvents);
						OutputDebugStringA(text.c_str());
						text = "\nClock cycles: " + std::to_string(metadata.myClockCycles);
						OutputDebugStringA(text.c_str());
						text = "\nTotal time: " + std::to_string(metadata.myTotalTime);
						OutputDebugStringA(text.c_str());
					}
					myDoOnce = false;
				}
#endif
				return;
			}

			KE::Timer timer;
			int handledEventsCount = 0;

			while (!myEventQueue.empty())
			{
				if (CHECK_CONDITION())
				{
#ifdef ENABLE_EVENT_PRINTING
					myEventMetadatas.push_back(EventMetadata());
					myEventMetadatas.back().myDesc = "Exceeded max time/events/cycles";
					myEventMetadatas.back().myHandledEvents = handledEventsCount;
					myEventMetadatas.back().myClockCycles = timer.GetElapsedCycles();
					myEventMetadatas.back().myTotalTime = timer.GetTotalTime();
#endif
					return;
				}

				++handledEventsCount;
				SendEvent(*myEventQueue.front());
				myEventQueue.pop();

				timer.UpdateDeltaTime();
			}
		}

		// Creates new event and adds it to the EventSystem
		// Returns a pointer to the created event
		template <class EventClass, typename... Args>
		EventClass* CreateNewEvent(Args&&... args)
		{
			myEvents.emplace_back(new EventClass(std::forward<Args>(args)...));
			return dynamic_cast<EventClass*>(myEvents.back());
		}

		// Deletes all pointers allocated in the EventSystem
		void HandleCleanup()
		{
			for (Event* event : myEvents)
			{
				delete event;
			}
			myEvents.clear();
		}

	private:
		// Private constructor to prevent instantiation
		EventSystem()
		{
			myEvents.reserve(MAX_ADDED_EVENTS);
		}

		~EventSystem() = default;
		EventSystem(const EventSystem&) = delete;
		EventSystem& operator=(const EventSystem&) = delete;

	private:
#ifdef ENABLE_EVENT_PRINTING
		struct EventMetadata
		{
			std::string myDesc;
			float myTotalTime;
			size_t myClockCycles;
			unsigned int myHandledEvents;
		};

		inline std::vector<EventMetadata> myEventMetadatas;
#endif

		std::unordered_map<std::type_index, std::vector<IObserver*>> myObservers = {};
		std::queue<Event*> myEventQueue = {};
		std::vector<Event*> myEvents = {};
		bool myDoOnce = true;
	};
}
