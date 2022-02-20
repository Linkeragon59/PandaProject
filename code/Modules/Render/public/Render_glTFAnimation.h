#pragma once

namespace Render
{
	class glTFModel;
	struct glTFNode;

	struct glTFAnimationChannel
	{
		enum class Path { TRANSLATION, ROTATION, SCALE };
		Path myPath;
		uint mySamplerIndex = 0;
		glTFNode* myNode;
	};

	struct glTFAnimationSampler
	{
		std::vector<float> myInputTimes;
		std::vector<glm::vec4> myOutputValues;
	};

	struct glTFAnimation
	{
		void Load(glTFModel* aContainer, const tinygltf::Model& aModel, uint anAnimationIndex);

		void Update(float aDeltaTime);

		std::string myName;

		std::vector<glTFAnimationSampler> mySamplers;
		std::vector<glTFAnimationChannel> myChannels;

		float myStartTime = std::numeric_limits<float>::max();
		float myEndTime = std::numeric_limits<float>::min();
		float myCurrentTime = 0.0f;
	};
}
