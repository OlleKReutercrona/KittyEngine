#pragma once
#include "Engine/Source/Graphics/Particle/ParticleEmitter.h"

#include <Editor/Source/EditorInterface.h>
#include "Engine/Source/Math/KittyMath.h"
#include "Engine/Source/Graphics/PostProcessing.h"
#include "Engine/Source/Graphics/Renderers/BasicRenderer.h"

namespace KE
{
	constexpr int VFX_SEQUENCE_FRAME_RATE = 120;
	constexpr const char* VFX_SEQUENCE_FILE_LOCATION = "Data/InternalAssets/VFXSequences/";

	class ParticleEmitter;
	class SpriteManager;

	struct MeshList;
	class VFXMeshInstance;
	class VFXManager;

	struct fxBuffer
	{
		Vector4f colour;
		Vector2f uvOffset;

		Vector2f uvScale;
		Vector4f bloomAttributes;
	};

	enum class VFXType : int
	{
		ParticleEmitter,
		VFXMeshInstance,

		Count
	};

	enum class VFXAttributeTypes
	{
		UV_X_SCROLL,
		UV_Y_SCROLL,
		
		TRANSLATION_X,
		TRANSLATION_Y,
		TRANSLATION_Z,
		
		ROTATION_X,
		ROTATION_Y,
		ROTATION_Z,
		
		SCALE_X,
		SCALE_Y,
		SCALE_Z,

		COLOUR_R,
		COLOUR_G,
		COLOUR_B,
		COLOUR_A,

		UV_X_SCALE,
		UV_Y_SCALE,
		//

		Count
	};



	static const char* VFXAttributeTypeNames[(int)VFXAttributeTypes::Count] =
	{
		"UV Offset: X",
		"UV Offset: Y",
		
		"Translation: X",
		"Translation: Y",
		"Translation: Z",

		"Rotation: X",
		"Rotation: Y",
		"Rotation: Z",

		"Scale: X",
		"Scale: Y",
		"Scale: Z",

		"Colour: R",
		"Colour: G",
		"Colour: B",
		"Colour: A",

		"UV Scale: X",
		"UV Scale: Y"
	};

	static const Vector3f VFXCurveDefaults[(int)VFXAttributeTypes::Count] =
	{
		{0.0f, 1.0f, 0.5f  }, //UV_X_SCROLL
		{0.0f, 1.0f, 0.5f  }, //UV_Y_SCROLL

		{-1.0f, 1.0f, 0.5f }, //TRANSLATION_X
		{-1.0f, 1.0f, 0.5f }, //TRANSLATION_Y
		{-1.0f, 1.0f, 0.5f }, //TRANSLATION_Z

		{0.0f, 360.0f, 0.5f }, //ROTATION_X
		{0.0f, 360.0f, 0.5f }, //ROTATION_Y
		{0.0f, 360.0f, 0.5f }, //ROTATION_Z

		{-1.0f, 1.0f, 0.5f }, //SCALE_X
		{-1.0f, 1.0f, 0.5f }, //SCALE_Y
		{-1.0f, 1.0f, 0.5f }, //SCALE_Z

		{0.0f, 1.0f, 0.5f  }, //COLOUR_R
		{0.0f, 1.0f, 0.5f  }, //COLOUR_G
		{0.0f, 1.0f, 0.5f  }, //COLOUR_B
		{0.0f, 1.0f, 0.5f  }, //COLOUR_A

		{0.0f, 1.0f, 0.5f  }, //UV_X_SCALE
		{0.0f, 1.0f, 0.5f  }, //UV_Y_SCALE

	};

	enum class VFXCurveProfiles
	{
		None,
		Discrete,
		Linear,
		Smooth,
		//Bezier,

		Count
	};

	static const char* VFXCurveProfileNames[(int)VFXCurveProfiles::Count] =
	{
		"None",
		"Discrete",
		"Linear",
		"Smooth",
		//"Bezier (Broken!)",
	};

	struct VFXCustomBufferInput
	{
		CBuffer* constantBuffer = nullptr;
		void* bufferData = nullptr;

		int bufferSlot = 0;
		int bufferSize = 0;
	};

	struct VFXCurveDataSet
	{
		VFXAttributeTypes myType = VFXAttributeTypes::Count;
		VFXCurveProfiles myCurveProfile = VFXCurveProfiles::Smooth;
		
		float myMinValue = 0.0f;
		float myMaxValue = 2.0f;

		std::vector<Vector2f> myData;
		bool visible = true;

		const char* GetName() { return VFXAttributeTypeNames[(int)myType]; }
		bool IsValid() { return myType != VFXAttributeTypes::Count; }
		float GetEvaluatedValue(int aFrameIndex, int aFirstFrameIndex, int aLastFrameIndex)
		{
			float leastTime = myData.front().x;
			float mostTime = myData.back().x;
			
			//scale leastTime and mostTime to be 0->1
			float frameProgressFraction = (float)(aFrameIndex - aFirstFrameIndex) / (float)(aLastFrameIndex - aFirstFrameIndex);
			float time = leastTime + (mostTime - leastTime) * frameProgressFraction;
			
			//find the two closest points
			int lowerClosestPoint = -1;
			int upperClosestPoint = -1;

			for (int i = 0; i < myData.size(); ++i)
			{
				if (myData[i].x <= time)
				{
					lowerClosestPoint = i;
				}
				else
				{
					upperClosestPoint = i;
					break;
				}
			}
			
			if (lowerClosestPoint == -1)
			{
				return myData[upperClosestPoint].y;
			}
			else if (upperClosestPoint == -1)
			{
				return myData[lowerClosestPoint].y;
			}
			else
			{
				float lowerTime = myData[lowerClosestPoint].x;
				float upperTime = myData[upperClosestPoint].x;
				float lowerValue = myData[lowerClosestPoint].y;
				float upperValue = myData[upperClosestPoint].y;
				float timeFraction = (time - lowerTime) / (upperTime - lowerTime);
				float value = 0.0f;

				switch (myCurveProfile)
				{
				case VFXCurveProfiles::Linear:
				{
					value = lowerValue + (upperValue - lowerValue) * timeFraction;
					break;
				}
				case VFXCurveProfiles::Smooth:
				{
					float smoothStep = Smoothstep(timeFraction);
					value = lowerValue + (upperValue - lowerValue) * smoothStep;
					break;
				}
				//case VFXCurveProfiles::Bezier:
				//{
				//	float bezierStep = CubicBezierStep(timeFraction);
				//	value = lowerValue + (upperValue - lowerValue) * bezierStep;
				//	break;
				//}
				case VFXCurveProfiles::Discrete:
				{
					value = lowerValue;
					break;
				}
				default:
					break;
				}

				return myMinValue + (myMaxValue - myMinValue) * value;
			}
		}
	};

	struct VFXTimeStamp
	{
		int myStartpoint = 0;
		int myEndpoint = 0;
		
		int myEffectIndex = 0;
		VFXType myType = VFXType::Count;

		bool myIsOpened = false;
		std::array<KE::VFXCurveDataSet, (size_t)KE::VFXAttributeTypes::Count> myCurveDataSets;
	};

	struct VFXRenderInput
	{
		union
		{
			Transform* myTransform;
			Transform myStationaryTransform;
		};

		eRenderLayers myLayer = eRenderLayers::Main;

		bool looping = false;
		bool isStationary = false;
		bool bloom = true;
		Vector3f scaleOverride = { -1.0f, -1.0f, -1.0f };

		VFXCustomBufferInput customBufferInput;

		Transform& GetTransform() { if (isStationary) { return myStationaryTransform; } return *myTransform; }

		VFXRenderInput(Transform& aTransform, bool aIsLooping = false, bool aIsStationary = false)
		{
			looping = aIsLooping; isStationary = aIsStationary;
			if (aIsStationary)
			{
				myStationaryTransform = aTransform;
			}
			else
			{
				myTransform = &aTransform;
			}
		}
	};

	struct VFXEmitter
	{
		ParticleEmitter myEmitter;
		int myStartFrame = 0;
		int myEndFrame = 0;
	};

	struct VFXSequence
	{
		std::string myName = "New Sequence";
		int myDuration = 1 * VFX_SEQUENCE_FRAME_RATE;
		int myIndex = -1;

		std::vector<VFXMeshInstance> myVFXMeshes;
		std::vector<VFXEmitter> myParticleEmitters;
		std::vector<VFXTimeStamp> myTimestamps;

		VFXManager* myManager = nullptr;

		void AddVFXMeshInstance();
		void AddParticleEmitter();
	};

	struct VFXSequencePlayerData
	{
		VFXRenderInput myRenderInput;
		eRenderLayers myLayer;

		std::vector<bool> myTimestampTriggerInfo;
		std::vector<VFXEmitter> myEmitters;

		int mySequenceIndex;
		float myTimer;
		int myFrame;
		bool myIsLooping;
		bool myIsWaitingOnParticles = false;
	};

	struct VFXSequenceRenderPackage
	{
		ModelData* modelData;
		Transform instanceTransform;
		eRenderLayers layer;
		VFXCustomBufferInput customBuffer;

		struct
		{
			Vector2f uvOffset		= { 0.0f,0.0f			};
			Vector3f translation	= { 0.0f,0.0f,0.0f		};
			Vector3f rotation		= { 0.0f,0.0f,0.0f		};
			Vector3f scale			= { 1.0f,1.0f,1.0f		};
			Vector4f colour			= { 1.0f,1.0f,1.0f,1.0f };
			Vector2f uvScale		= { 1.0f,1.0f };
		} attributes;

		bool bloom = true;
	};

	class VFXMeshInstance
	{
	friend class VFXManager;
	private:
		Transform myTransform;

		ModelData myModelData;
	public:
		VFXMeshInstance() = default;
		~VFXMeshInstance() = default;

		inline void SetModelData(const ModelData& aModelData) { myModelData = aModelData; }
		inline ModelData* GetModelData() { return &myModelData; }
		inline Transform* GetTransform() { return &myTransform; }
	};

	class VFXManager
	{
		KE_EDITOR_FRIEND
	private:
		Graphics* myGraphics;
		SpriteManager* mySpriteManager;
		std::vector<VFXSequence> myVFXSequences;
		PostProcessing myVFXPostProcessing;
		BasicRenderer myVFXRenderer;

		//render data
		std::vector<VFXSequencePlayerData> myRenderQueue;
		std::vector<VFXSequenceRenderPackage> myRenderPackages;
		std::vector<SpriteBatch*> mySpriteBatches;
		//
		void RenderInspectedVFX(int aVFXIndex, int aCurrentFrame, Transform* aTransform, eRenderLayers aLayer);
	public:
		void Init(Graphics* aGraphics);
		void RenderVFX(fxBuffer& fxb, ModelData* modelData);
		void Render(eRenderLayers aLayer, ID3D11DepthStencilView* aDSV);
		void Update(float aDeltaTime);
		void Resize(int aWidth, int aHeight);

		void EndFrame();

		void PrepareRenderData(VFXSequencePlayerData& aPlayerData);


		void InitializeVFXMesh(VFXMeshInstance& aMeshToInitialize) const;
		void InitializeParticleEmitter(ParticleEmitter& anEmitterToInitialize) const;
		
		inline PostProcessing& GetPostProcessing() { return myVFXPostProcessing; }

		void TriggerVFXSequence(int aVFXSequenceIndex, const VFXRenderInput& aRenderInput);
		void StopVFXSequence(int aVFXSequenceIndex, const VFXRenderInput& aRenderInput);

		void SaveVFXSequence(VFXSequence* aSequence);
		void LoadVFXSequence(int aVFXSequenceIndex, const std::string& aFilePath);

		int CreateVFXSequence(const std::string& aName);

		//try to avoid using this function, it's slow. Get the index once and store it.
		int GetVFXSequenceFromName(const std::string& aName);
		VFXSequence* GetVFXSequence(int aVFXSequenceIndex);
		void ClearVFX();
	};

	struct VFXPlayerInterface
	{
		VFXManager* manager;

		std::vector<int> myVFXSequenceIndices;

		void TriggerVFXSequence(int aVFXSequenceIndex, const VFXRenderInput& aRenderInput)
		{
			manager->TriggerVFXSequence(myVFXSequenceIndices[aVFXSequenceIndex], aRenderInput);
		}

		void StopVFXSequence(int aVFXSequenceIndex, const VFXRenderInput& aRenderInput)
		{
			manager->StopVFXSequence(myVFXSequenceIndices[aVFXSequenceIndex], aRenderInput);
		}

		void AddVFX(const std::string& aVFXName)
		{
			myVFXSequenceIndices.push_back(manager->GetVFXSequenceFromName(aVFXName));
		}
	};




}
