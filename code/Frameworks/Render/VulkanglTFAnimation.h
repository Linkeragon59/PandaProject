#pragma once

namespace Render::Vulkan::glTF
{
	class Model;
	struct Node;

	struct AnimationChannel
	{
		enum class Path { TRANSLATION, ROTATION, SCALE };
		Path myPath;
		uint mySamplerIndex = 0;
		Node* myNode;
	};

	struct AnimationSampler
	{
		std::vector<float> myInputTimes;
		std::vector<glm::vec4> myOutputValues;
	};

	struct Animation
	{
		void Load(Model* aContainer, const tinygltf::Model& aModel, uint anAnimationIndex);

		void Update(float aDeltaTime);

		std::string myName;

		std::vector<AnimationSampler> mySamplers;
		std::vector<AnimationChannel> myChannels;

		float myStartTime = std::numeric_limits<float>::max();
		float myEndTime = std::numeric_limits<float>::min();
		float myCurrentTime = 0.0f;
	};
}
