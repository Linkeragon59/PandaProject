#pragma once

namespace Render
{
	struct glTFAnimationChannel
	{
		enum class Type { TRANSLATION, ROTATION, SCALE };
		Type myType;
		uint32_t mySamplerIndex = 0;
		uint32_t myNodeIndex = 0;
	};

	struct glTFAnimationSampler
	{
		std::vector<float> myTimes;
		std::vector<glm::vec4> myValues;
	};

	struct glTFAnimation
	{
		std::string myName;

		std::vector<glTFAnimationSampler> mySamplers;
		std::vector<glTFAnimationChannel> myChannels;

		float myStartTime = std::numeric_limits<float>::max();
		float myEndTime = std::numeric_limits<float>::min();

		void Load(const tinygltf::Model& aModel, uint32_t anAnimationIndex);
	};
}
