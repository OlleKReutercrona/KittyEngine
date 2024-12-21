#pragma once
#include <wrl/client.h>

#include "Engine\Source\Math\Matrix4x4.h"
#include "Engine\Source\Math\Vector4.h"
#include "Engine\Source\Graphics\Primitives\CubePrimitive.h"
#include "Engine\Source\Graphics\Primitives\SpherePrimitive.h"
#include "Engine\Source\Graphics\Primitives\CapsulePrimitive.h"



using Microsoft::WRL::ComPtr;
struct ID3D11Device;
struct ID3D11DeviceContext;
struct ID3D11Buffer;
struct ID3D11VertexShader;
struct ID3D11PixelShader;
struct ID3D11InputLayout;
struct D3D11_INPUT_ELEMENT_DESC;


namespace KE
{
	class PixelShader;
	class VertexShader;

#define DL_DEFAULT_COLOUR { 1.0f, 0.0f, 0.0f, 1.0f }

	struct LineVertex
	{
		LineVertex() = default;
		LineVertex(Vector4f aPos) { position = aPos; }
		LineVertex(Vector4f aPos, Vector4f aColour) { position = aPos;  colour = aColour;  }
		LineVertex(Vector3f aPos) { position = { aPos.x,aPos.y,aPos.z, 1.0f }; } // Added by Daniel
		LineVertex(Vector3f aPos, Vector4f aColour) { position = { aPos.x,aPos.y,aPos.z, 1.0f };  colour = aColour;  } // Added by Daniel
		LineVertex(const float aX, const float aY, const float aZ, const float aW) { position = {aX, aY, aZ, aW};}
		Vector4f position = { 1.0f,1.0f,1.0f,1.0f };
		Vector4f colour = DL_DEFAULT_COLOUR;

		LineVertex operator*(const Vector3f aSizeModifier)
		{
			LineVertex copy = *this;

			copy.position.x *= aSizeModifier.x;
			copy.position.y *= aSizeModifier.y;
			copy.position.z *= aSizeModifier.z;

			return copy;
		}

		LineVertex operator*=(const Vector3f aSizeModifier)
		{
			LineVertex copy = *this;

			copy.position.x *= aSizeModifier.x;
			copy.position.y *= aSizeModifier.y;
			copy.position.z *= aSizeModifier.z;

			return copy;
		}

		LineVertex operator*= (const float aSizeModifier)
		{
			LineVertex copy = *this;

			copy.position.x *= aSizeModifier;
			copy.position.y *= aSizeModifier;
			copy.position.z *= aSizeModifier;

			return copy;
		}

		LineVertex operator+= (const float aSizeModifier)
		{
			LineVertex copy = *this;

			copy.position.x += aSizeModifier;
			copy.position.y += aSizeModifier;
			copy.position.z += aSizeModifier;

			return copy;
		}

		LineVertex operator+(const Vector3f aPosition)
		{
			LineVertex copy = *this;

			copy.position.x += aPosition.x;
			copy.position.y += aPosition.y;
			copy.position.z += aPosition.z;

			return copy;
		}

		LineVertex operator*(const float aRadius)
		{
			LineVertex copy = *this;

			copy.position.x *= aRadius;
			copy.position.y *= aRadius;
			copy.position.z *= aRadius;

			return copy;
		}
	};

	struct LineRenderData
	{
		std::vector<int> myIndices;
		std::vector<LineVertex> myVertices;
		PixelShader* myPS  = nullptr;
		VertexShader* myVS = nullptr;
		std::vector<D3D11_INPUT_ELEMENT_DESC> myLayout;
	};



	class DebugLine
	{
		friend class DebugRenderer;

	public:
		DebugLine();
		~DebugLine();

		bool Initialize(LineRenderData aRenderData, ID3D11Device* aDevice, bool isStatic = false);
		void Render(ID3D11DeviceContext* context, VertexShader* aVSOverride = nullptr, PixelShader* aPSOverride = nullptr);

		void SetVertexes();
		void SetIndices();
		LineVertex* GetVertex(const int anID);
		inline std::vector<LineVertex>* GetVertexes() { return &myVertices; };

		void AddLine(const LineVertex aStartPoint, const LineVertex anEndPoint);

		void DrawCube(const Vector3f& aPosition, const Vector3f& aDimensions, const Vector4f& aColour = DL_DEFAULT_COLOUR);
		void DrawCube(const Transform& aTransform, const Vector3f& aDimensions,  const Vector4f& aColour = DL_DEFAULT_COLOUR);
		void DrawSphere(const Vector3f& aPosition, const float aRadius, const Vector4f& aColour = DL_DEFAULT_COLOUR);
		void DrawSphere(const Transform& aTransform, float aRadius, const Vector4f& aColour = DL_DEFAULT_COLOUR);
		void DrawCone(const Vector3f& aPosition, const Vector3f& aDirection, const float aLength, const float anOuterRadius, const float anInnerRadius, const Vector4f& aColour = DL_DEFAULT_COLOUR);

		void DrawCapsule(const Vector3f& aPosition, const float aRadius, const float aLength, const Vector4f& aColour = DL_DEFAULT_COLOUR);
		void DrawCapsule(const Transform& aTransform, const float aRadius, const float aLength, const Vector4f& aColour = DL_DEFAULT_COLOUR);

		void EndFrame();
	private:
		void CreateBuffers();

		ID3D11Device* myDevice = nullptr;

		ComPtr<ID3D11Buffer> myVertexBuffer;
		ComPtr<ID3D11Buffer> myIndexBuffer;
		VertexShader* myVertexShader;
		PixelShader* myPixelShader;
		ComPtr<ID3D11InputLayout> myInputLayout;

		std::vector<LineVertex> myVertices;
		std::vector<int> myIndicesVector;

		Matrix4x4f myTransform;
		int myIndices = 0;

		bool isReady = false;
		bool myIsStatic = false;

		// Primitives
		inline static bool myPrimitivesHasBeenCreated = false;
		inline static std::array<LineVertex, 8> myCubeVertices;
		CubePrimitive myCube;
		SpherePrimitive mySphere;
		CapsulePrimitive myCapsule;

		inline static int mySphereSlices = 10;
		inline static int mySphereStacks = 5;
		inline static float mySphereRadius = 1.0f;

		inline static std::vector<LineVertex> mySphereVertices;
		inline static std::vector<int> mySphereIndices;
	};


}