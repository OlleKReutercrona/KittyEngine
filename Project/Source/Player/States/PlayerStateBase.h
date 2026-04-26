#pragma once
#include "Engine/Source/Graphics/FX/VFX.h"

namespace KE
{
	enum class eInteractionType;
}

namespace KE
{
	struct PlayerEvent;
	struct Interaction;
}

namespace P8
{
	enum class eInputAction;
	class Player;


	typedef unsigned int StateBehaviourFlags;
	enum StateBehaviourFlag : unsigned int
	{
		eBlockRotation = 1 << 0,
		eBlockMovement = 1 << 1,
		eStopVelocity = 1 << 2,
		eRunWhilePaused = 1 << 3,

		eCount
	};

	struct StateInput
	{
		const eInputAction& actionType;
		const KE::eInteractionType& interactionType;
	};

	class PlayerStateBase
	{
	protected:
		Player* myPlayer = nullptr;
	public:
		virtual ~PlayerStateBase() = default;
		virtual void Init(Player* aPlayer) { myPlayer = aPlayer; }		

		virtual StateBehaviourFlags GetBehaviourFlags() const { return 0; }

		virtual void Update() = 0;
		virtual void OnEnter() = 0;
		virtual void OnExit() = 0;

		//virtual void ReceiveEvent(const KE::PlayerEvent& aEvent) = 0;
		virtual void ActionInput(const StateInput& aInput) = 0;


		virtual void OnPlayerIndexChange() {__noop;}
	};

}


namespace P8
{
	class PlayerIdleState : public PlayerStateBase
	{
	public:
		void Update() override;
		void OnEnter() override;
		void OnExit() override;

		virtual void ActionInput(const StateInput& aInput) override;
	};

}

namespace P8
{
	class PlayerMoveState : public PlayerStateBase
	{
	public:
		void Update() override;
		void OnEnter() override;
		void OnExit() override;

		virtual void ActionInput(const StateInput& aInput) override;
	};

}

namespace P8
{
	class PlayerDashState : public PlayerStateBase
	{
	public:
		StateBehaviourFlags GetBehaviourFlags() const override
		{
			return StateBehaviourFlag::eBlockRotation | StateBehaviourFlag::eBlockMovement;
		}

		void Update() override;
		void OnEnter() override;
		void OnExit() override;

		virtual void ActionInput(const StateInput& aInput) override;
	};

}

namespace P8
{
	class PlayerWindupState : public PlayerStateBase
	{
	private:
		Transform vfxTransform;

	public:
		StateBehaviourFlags GetBehaviourFlags() const override
		{
			return StateBehaviourFlag::eBlockMovement | StateBehaviourFlag::eStopVelocity;
		}

		void Update() override;
		void OnEnter() override;
		void OnExit() override;

		virtual void ActionInput(const StateInput& aInput) override;
	};

}

namespace P8
{
	class PlayerThrowState : public PlayerStateBase
	{
	private:
		float lockTime = 0.25f;
		float lockTimer = 0.0f;
	public:
		StateBehaviourFlags GetBehaviourFlags() const override
		{
			return 0;//StateBehaviourFlag::eBlockRotation;
		}

		void Update() override;
		void OnEnter() override;
		void OnExit() override;

		virtual void ActionInput(const StateInput& aInput) override;
	};

}

namespace P8
{
	class PlayerRecallingState : public PlayerStateBase
	{
	public:
		StateBehaviourFlags GetBehaviourFlags() const override
		{
			return StateBehaviourFlag::eBlockMovement | StateBehaviourFlag::eStopVelocity;
		}

		void Update() override;
		void OnEnter() override;
		void OnExit() override;

		virtual void ActionInput(const StateInput& aInput) override;
	};

}

namespace P8
{
	class PlayerAttackState : public PlayerStateBase
	{
	private:
		float attackTimer = 0.0f;
		float attackTime = 1.0f;
	public:
		StateBehaviourFlags GetBehaviourFlags() const override
		{
			return StateBehaviourFlag::eBlockRotation | StateBehaviourFlag::eBlockMovement;
		}

		void Update() override;
		void OnEnter() override;
		void OnExit() override;

		virtual void ActionInput(const StateInput& aInput) override;
	};

}

namespace P8
{
	class PlayerDeathState : public PlayerStateBase
	{
	private:
		float deathTimer = 0.0f;
		float deathTime = 5.0f;
	public:
		StateBehaviourFlags GetBehaviourFlags() const override
		{
			return StateBehaviourFlag::eBlockRotation  | 
				   StateBehaviourFlag::eBlockMovement  | 
				   StateBehaviourFlag::eRunWhilePaused | 
				   StateBehaviourFlag::eStopVelocity   ;
		}

		void Update() override;
		void OnEnter() override;
		void OnExit() override;

		virtual void ActionInput(const StateInput& aInput) override;
	};

}

namespace P8
{
	class PlayerFallingState : public PlayerStateBase
	{
	private:
		float fallTimer = 0.0f;
		float fallTime = 0.5f;
	public:
		StateBehaviourFlags GetBehaviourFlags() const override
		{
			return StateBehaviourFlag::eBlockRotation | StateBehaviourFlag::eBlockMovement;
		}

		void Update() override;
		void OnEnter() override;
		void OnExit() override;

		virtual void ActionInput(const StateInput& aInput) override;

		bool CanDash();
	};

}

namespace P8
{
	class PlayerTauntState : public PlayerStateBase
	{
	private:
		float tauntTimer = 0.0f;
		float tauntTime = 1.25f;
	public:
		StateBehaviourFlags GetBehaviourFlags() const override
		{
			return StateBehaviourFlag::eBlockRotation | StateBehaviourFlag::eBlockMovement | StateBehaviourFlag::eStopVelocity;
		}

		void Update() override;
		void OnEnter() override;
		void OnExit() override;

		virtual void ActionInput(const StateInput& aInput) override;
	};

}

namespace P8
{
	class PlayerLobbyState : public PlayerStateBase
	{
	private:
	public:
		StateBehaviourFlags GetBehaviourFlags() const override
		{
			return StateBehaviourFlag::eBlockRotation | StateBehaviourFlag::eBlockMovement | StateBehaviourFlag::eRunWhilePaused;
		}

		void Update() override;
		void OnEnter() override;
		void OnExit() override;

		void OnReady();
		void OnUnready();

		virtual void ActionInput(const StateInput& aInput) override;

		void SetVFXActive(bool aActive, int idx);

		void OnPlayerIndexChange() override;
	};

}

namespace P8
{
	class PlayerWinScreenState : public PlayerStateBase
	{
	private:
		int scoreboardPosition = 0;
		bool taunting = false;

		float winTimer = 0.0f;
		float winTime = 0.05f;

		KE::VFXPlayerInterface myWinscreenVFX;
		Transform myWinscreenVFXTransform;

	public:
		StateBehaviourFlags GetBehaviourFlags() const override
		{
			return StateBehaviourFlag::eBlockRotation | StateBehaviourFlag::eBlockMovement | StateBehaviourFlag::eRunWhilePaused;
		}

		void Init(Player* aPlayer) override;	

		void Update() override;
		void OnEnter() override;
		void OnExit() override;

		virtual void ActionInput(const StateInput& aInput) override;

		void SetVFXActive(bool aActive, int idx);

		void OnPlayerIndexChange() override;
	};

}

namespace P8
{
	struct PlayerStateList
	{
		std::unordered_map<std::type_index, PlayerStateBase*> states;

		template <typename T>
		inline PlayerStateBase* CreateState()
		{
			states[typeid(T)] = new T();
			return states[typeid(T)];
		}

		template <typename T>
		inline PlayerStateBase* GetState()
		{
			return states[typeid(T)];
		}

		~PlayerStateList()
		{
			for (auto& state : states)
			{
				delete state.second;
			}
		}
	};
}