#pragma once
namespace KE_EDITOR
{
	enum class WindowStatus
	{
		Open,
		Hidden,
		Closed
	};

	typedef const std::any& EditorWindowInput;

	class EditorWindowBase
	{
		KE_EDITOR_FRIEND
	protected:
		unsigned int myID = 0;
		WindowStatus myStatus = WindowStatus::Open;
		const char* myName = nullptr;

		unsigned int myDockNodeID = 0;
		int myWindowFlags = 0;

	public:
		EditorWindowBase(EditorWindowInput aStartupData = {}) {};
		virtual ~EditorWindowBase() {};
		//
		void SetID(unsigned int anIndex) { myID = anIndex; }
		unsigned int GetID() const { return myID; }
		WindowStatus GetStatus() const { return myStatus; }

		const int GetWindowFlags() const { return myWindowFlags; }
		void SetWindowFlags(const int aFlags) { myWindowFlags = aFlags; }

		bool Begin();
		void End();

		virtual void StyleBegin() {}
		virtual void StyleEnd() {}

		//
		virtual const char* GetWindowName() const { return ""; }
		virtual void Init() = 0;
		virtual void Update() = 0;
		virtual void Render() = 0;
		//

		virtual void Serialize(void* aWorkingData)	 { __noop; }
		virtual void Deserialize(void* aWorkingData) { __noop; }

		const char* GetName() const { return myName; }
		void SetName(const char* aName) { myName = aName; }

		//Set the dock node ID for this window
		void Dock(unsigned int aDockNodeID) {myDockNodeID = aDockNodeID; }
		unsigned int GetDockNodeID();

	};
}
