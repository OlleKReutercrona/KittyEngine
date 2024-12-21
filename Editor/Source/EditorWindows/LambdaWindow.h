#pragma once
#include <functional>

#ifndef KITTYENGINE_NO_EDITOR


namespace KE_EDITOR
{

	typedef std::function<void()> LambdaWindowFunc;

	struct LambdaWindowInput
	{
		std::string name;
		LambdaWindowFunc func;
	};

	class LambdaWindow : public EditorWindowBase
	{
		KE_EDITOR_FRIEND
	private:
		LambdaWindowInput myFunctionData;

	public:
		LambdaWindow(EditorWindowInput aStartupData);

		const char* GetWindowName() const override { return myFunctionData.name.c_str(); }
		void Init() override;
		void Update() override;
		void Render() override;

		void Serialize(void* aWorkingData) override;
		void Deserialize(void* aWorkingData) override;
	};
}

#endif